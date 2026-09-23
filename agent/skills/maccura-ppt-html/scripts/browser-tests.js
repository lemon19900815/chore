'use strict';

(() => {
  const WIDTH = 1920;
  const HEIGHT = 1080;
  const BROKEN_IMAGE = 'data:image/png;base64,ppt-offline-intentional-invalid';
  const assert = (condition, message) => { if (!condition) throw new Error(message); };
  const sections = () => [...document.querySelectorAll('.slide')];
  const tabs = () => [...document.querySelectorAll('[role="tab"]')];
  const active = () => {
    const visible = sections().filter(slide => !slide.hidden);
    assert(visible.length === 1, '必须且只能显示一页');
    return visible[0];
  };
  const titlePrefix = () => document.title.replace(/[<>:"/\\|?*\u0000-\u001f]/g, '_').trim().slice(0, 80) || 'maccura';
  const tick = () => new Promise(resolve => setTimeout(resolve, 0));
  const phase = enabled => {
    if (typeof globalThis.__pptOfflineTestPhase === 'function') {
      globalThis.__pptOfflineTestPhase(JSON.stringify({ phase: 'broken-image', active: enabled, url: BROKEN_IMAGE }));
    }
  };

  function selectSlide(index) {
    const slide = sections()[index];
    assert(slide && tabs()[index], `不存在第 ${index + 1} 页`);
    tabs()[index].click();
    assert(active() === slide, `${slide.id}: tab 没有切换到对应页`);
    tabs().forEach((tab, i) => {
      assert(tab.getAttribute('aria-selected') === String(i === index), `${slide.id}: aria-selected 错误`);
      assert(tab.tabIndex === (i === index ? 0 : -1), `${slide.id}: tabIndex 错误`);
    });
    return slide;
  }

  function waitForIdle() {
    const button = document.querySelector('#export-image');
    assert(button, '缺少导出按钮 #export-image');
    return new Promise((resolve, reject) => {
      if (!button.disabled) return resolve();
      const observer = new MutationObserver(() => {
        if (!button.disabled) { clearTimeout(timer); observer.disconnect(); resolve(); }
      });
      const timer = setTimeout(() => {
        observer.disconnect();
        reject(new Error('图片导出超时（15 秒）'));
      }, 15000);
      observer.observe(button, { attributes: true, attributeFilter: ['disabled'] });
    });
  }

  async function inspect() {
    assert(location.protocol === 'file:' && !navigator.onLine, '必须在 file:// 且浏览器离线环境验证');
    const slides = sections();
    const controls = tabs();
    assert(slides.length > 0 && slides.length === controls.length, 'slides/tabs 必须非空且数量相同');
    const allIds = [...document.querySelectorAll('[id]')].map(element => element.id);
    assert(allIds.every(id => id.trim() && !/\s/.test(id)), 'DOM ID 不能为空或含空白');
    assert(new Set(allIds).size === allIds.length, 'DOM ID 不唯一');
    slides.forEach((slide, i) => {
      const tab = controls[i];
      assert(slide.id && tab.id, '每页和 tab 必须有非空唯一 ID');
      assert(slide.getAttribute('role') === 'tabpanel', `${slide.id}: 缺少 tabpanel role`);
      assert(tab.dataset.slide === slide.id && tab.getAttribute('aria-controls') === slide.id,
        `${slide.id}: tabs 必须与 DOM .slide 同序并正确 aria-controls`);
      assert(slide.getAttribute('aria-labelledby') === tab.id, `${slide.id}: aria-labelledby 错误`);
      assert(tab.closest('[role="tablist"]'), `${slide.id}: tab 不在 tablist 内`);
    });
    const ids = new Set(slides.map(slide => slide.id));
    document.querySelectorAll('[data-slide]').forEach(control => {
      assert(ids.has(control.dataset.slide), `无效 data-slide 目标: ${control.dataset.slide}`);
    });
    await document.fonts.ready;
    const original = slides.indexOf(active());
    const result = [];
    try {
      for (let i = 0; i < slides.length; i++) {
        const slide = selectSlide(i);
        const background = slide.querySelector('img.slide-background');
        assert(background, `${slide.id}: 缺少 img.slide-background`);
        assert(/^data:image\//i.test(background.currentSrc || background.src), `${slide.id}: 底图不是 data:image`);
        await background.decode();
        assert(background.naturalWidth > 0 && background.naturalHeight > 0, `${slide.id}: 底图未 decode`);
        assert(Math.abs(background.naturalWidth / background.naturalHeight - 16 / 9) < 0.01,
          `${slide.id}: 底图不是 16:9`);
        const rect = slide.getBoundingClientRect();
        assert(rect.width > 0 && rect.height > 0 && Math.abs(rect.width / rect.height - 16 / 9) < 0.01,
          `${slide.id}: 显示比例不是 16:9`);
        result.push({ id: slide.id, tabId: controls[i].id, background: `${background.naturalWidth}x${background.naturalHeight}` });
      }
      // PNG, JPEG, WebP, SVG, etc. are all allowed if the browser can decode them.
      for (const image of document.images) {
        assert(/^data:image\//i.test(image.currentSrc || image.src), '所有 HTML 图片必须内嵌为 data:image');
        await image.decode();
      }
    } finally {
      selectSlide(original);
    }
    return { title: document.title, prefix: titlePrefix(), slides: result };
  }

  async function run() {
    const slides = sections();
    const button = document.querySelector(document.fullscreenElement ? '#fullscreen-export' : '#export-image');
    const status = document.querySelector(document.fullscreenElement ? '#fullscreen-status' : '#export-status');
    const next = document.querySelector('#next');
    const stage = document.querySelector('.stage');
    const navigation = document.querySelector('.slide-navigation');
    assert(button && status && next && stage && navigation, '缺少导出状态、next、stage 或 navigation');
    const firstIndex = slides.indexOf(active());
    const originalTitle = document.title;
    const originalClick = HTMLAnchorElement.prototype.click;
    const downloads = [];
    const results = [];
    const failures = [];
    const names = new Set();
    let injectedFailure = false;
    HTMLAnchorElement.prototype.click = function () {
      if (!this.download) return originalClick.call(this);
      // Attach the rejection handler immediately; blob URLs can be revoked by the exporter.
      const blob = fetch(this.href).then(response => response.blob());
      blob.catch(() => {});
      downloads.push({ name: this.download, blob });
    };
    const unlocked = label => {
      assert(!button.disabled && !stage.inert && !navigation.inert, `${label}: 导出结束必须解除按钮/stage/navigation 锁定`);
      assert(!document.querySelector('[data-export-clone]'), `${label}: 遗留临时导出元素`);
    };
    const filename = (download, slide, label) => {
      assert(download.name.endsWith('.png') && download.name.includes(slide.id), `${label}: 文件名应含页面标识并以 .png 结尾`);
      if (!download.name.startsWith(titlePrefix())) failures.push(`${label}: PNG 前缀必须来自 document.title，并将 Windows 非法字符替换为下划线`);
    };
    try {
      for (let i = 0; i < slides.length; i++) {
        const slide = selectSlide(i);
        const id = slide.id;
        const before = downloads.length;
        button.click();
        assert(button.disabled, `${id}: 导出期间必须禁用重复点击`);
        assert(stage.inert && navigation.inert, `${id}: 导出期间必须锁定 stage/navigation`);
        button.click();
        // .click() bypasses inert hit testing, so this also checks the JS exporting guard.
        next.click();
        assert(active() === slide, `${id}: 导出期间 click next 不得换页`);
        await waitForIdle();
        unlocked(id);
        assert(active() === slide, `${id}: 导出改变了当前页`);
        assert(downloads.length === before + 1, `${id}: 必须且只能导出一张 PNG；${status.textContent}`);
        const download = downloads.at(-1);
        filename(download, slide, id);
        assert(!names.has(download.name), `${id}: 不同页面的文件名不能重复`);
        names.add(download.name);
        const blob = await download.blob;
        assert(blob.type === 'image/png', `${id}: 下载不是 PNG`);
        const bitmap = await createImageBitmap(blob);
        try {
          assert(bitmap.width === WIDTH && bitmap.height === HEIGHT, `${id}: bitmap 不是 1920×1080`);
          const canvas = document.createElement('canvas');
          canvas.width = WIDTH;
          canvas.height = HEIGHT;
          const context = canvas.getContext('2d', { willReadFrequently: true });
          context.drawImage(bitmap, 0, 0);
          const reference = document.createElement('canvas');
          reference.width = WIDTH;
          reference.height = HEIGHT;
          const referenceContext = reference.getContext('2d', { willReadFrequently: true });
          // Composite the original background over white, just like the PNG exporter.
          referenceContext.fillStyle = '#ffffff';
          referenceContext.fillRect(0, 0, WIDTH, HEIGHT);
          referenceContext.drawImage(slide.querySelector('img.slide-background'), 0, 0, WIDTH, HEIGHT);
          // Only protected background/brand areas, not arbitrary content positions.
          for (const [x, y] of [[0, 0], [1800, 50]]) {
            const pixel = context.getImageData(x, y, 1, 1).data;
            const expected = referenceContext.getImageData(x, y, 1, 1).data;
            assert(pixel.every((value, channel) => Math.abs(value - expected[channel]) < 12), `${id}: 原底图左上角/右上品牌区或白色底色发生偏移`);
          }
          const actual = context.getImageData(0, 0, WIDTH, HEIGHT).data;
          const background = referenceContext.getImageData(0, 0, WIDTH, HEIGHT).data;
          let changedPixels = 0;
          for (let p = 0; p < actual.length; p += 4) {
            assert(actual[p + 3] === 255, `${id}: PNG 必须完全不透明`);
            if (Math.max(Math.abs(actual[p] - background[p]), Math.abs(actual[p + 1] - background[p + 1]),
              Math.abs(actual[p + 2] - background[p + 2])) > 20) changedPixels++;
          }
          const hasContent = Boolean(slide.innerText.trim()) || [...slide.querySelectorAll('img:not(.slide-background), svg, canvas, video')]
            .some(element => element.getClientRects().length && getComputedStyle(element).visibility !== 'hidden');
          assert(!hasContent || changedPixels > 8, `${id}: 有 HTML 内容的页不能仅导出底图`);
          results.push({ id, size: `${bitmap.width}x${bitmap.height}`, bytes: blob.size, changedPixels, hasContent, name: download.name });
        } finally {
          bitmap.close();
        }
        // Verify that ordinary navigation resumes, not just the inert properties.
        next.click();
        assert(active() === slides[(i + 1) % slides.length], `${id}: 导出后 next 无法换页`);
      }
      selectSlide(slides.length - 1);
      const image = active().querySelector('img.slide-background');
      const source = image.getAttribute('src');
      const srcset = image.getAttribute('srcset');
      try {
        phase(true);
        injectedFailure = true;
        image.removeAttribute('srcset');
        image.src = BROKEN_IMAGE;
        await image.decode().catch(() => {});
        const before = downloads.length;
        button.click();
        await waitForIdle();
        assert(downloads.length === before, '底图损坏时不能导出不完整图片');
        assert(status.textContent.includes('失败'), '导出失败必须显示可读提示');
        unlocked('失败恢复');
      } finally {
        if (source === null) image.removeAttribute('src');
        else image.setAttribute('src', source);
        if (srcset !== null) image.setAttribute('srcset', srcset);
        await image.decode();
        await tick();
        phase(false);
        injectedFailure = false;
      }
      // Change title deliberately: a hard-coded or cached prefix must not pass.
      document.title = '离线标题验收<>:"/\\|?*';
      const before = downloads.length;
      button.click();
      await waitForIdle();
      assert(downloads.length === before + 1, '图片恢复后必须可以重新导出');
      unlocked('重试');
      filename(downloads.at(-1), active(), 'title-derived filename');
      const recovered = await downloads.at(-1).blob;
      assert(recovered.type === 'image/png' && recovered.size > 0, '恢复后必须生成有效 PNG');
      results.push({ check: '失败提示、解除锁定、恢复重试、title-derived filename', name: downloads.at(-1).name });
      return { results, failures };
    } finally {
      // Keep monkey patches and test-only mutations confined to this test run.
      await waitForIdle().catch(() => {});
      HTMLAnchorElement.prototype.click = originalClick;
      document.title = originalTitle;
      if (injectedFailure) phase(false);
      selectSlide(firstIndex);
    }
  }

  async function waitForFullscreen(enabled) {
    const start = performance.now();
    while (Boolean(document.fullscreenElement) !== enabled || document.querySelector('#toggle-fullscreen')?.disabled) {
      assert(performance.now() - start < 5000, '全屏切换超时或按钮未恢复');
      await new Promise(requestAnimationFrame);
    }
    await new Promise(requestAnimationFrame);
  }

  function inspectFullscreen(enabled) {
    const preview = document.querySelector('.preview');
    const enter = document.querySelector('#toggle-fullscreen');
    const toolbar = document.querySelector('.fullscreen-toolbar');
    assert(enter && toolbar, '缺少全屏按钮或全屏操作栏');
    assert(enter.getAttribute('aria-pressed') === String(enabled), '全屏按钮状态不同步');
    assert((document.fullscreenElement === preview) === enabled, '必须使包含所有幻灯片的 preview 全屏');
    assert(Boolean(toolbar.getClientRects().length) === enabled, '全屏操作栏可见状态错误');
    if (!enabled) return { enabled };
    assert(!document.querySelector('#fullscreen-status').textContent.includes('全屏失败'), '成功进入全屏后必须清除之前的全屏失败提示');
    const slide = active();
    const rect = slide.getBoundingClientRect();
    const bar = toolbar.getBoundingClientRect();
    assert(Math.abs(rect.width / rect.height - 16 / 9) < 0.01, '全屏幻灯片必须保持 16:9');
    assert(rect.left >= -1 && rect.top >= -1 && rect.right <= innerWidth + 1 && rect.bottom <= bar.top + 1,
      '全屏幻灯片裁切或操作栏遮挡品牌/内容');
    assert(bar.bottom <= innerHeight + 1 && bar.left >= -1 && bar.right <= innerWidth + 1, '全屏操作栏溢出窗口');
    for (const button of toolbar.querySelectorAll('button')) {
      const bounds = button.getBoundingClientRect();
      assert(bounds.width >= 44 && bounds.height >= 44 && bounds.right <= innerWidth + 1, '全屏按钮触控尺寸或位置不正确');
      assert(!button.closest('.slide'), '全屏按钮不得进入 PNG');
    }
    assert(document.querySelector('#fullscreen-counter').textContent === document.querySelector('#page-counter').textContent,
      '全屏页码未同步');
    return { enabled, id: slide.id, width: rect.width, height: rect.height, viewport: [innerWidth, innerHeight] };
  }

  async function rejectFullscreen() {
    const preview = document.querySelector('.preview');
    const button = document.querySelector('#toggle-fullscreen');
    assert(button, '缺少全屏按钮 #toggle-fullscreen');
    const descriptor = Object.getOwnPropertyDescriptor(preview, 'requestFullscreen');
    try {
      Object.defineProperty(preview, 'requestFullscreen', {
        configurable: true, value: () => Promise.reject(new Error('intentional fullscreen rejection'))
      });
      button.click();
      await tick();
      assert(!button.disabled && !document.fullscreenElement, '全屏失败后按钮必须解锁且不伪装成全屏');
      assert(document.querySelector('#fullscreen-message').textContent.includes('全屏失败'), '缺少全屏失败提示');
      selectSlide(sections().length - 1);
      selectSlide(0);
      return '全屏拒绝后提示、解锁及导航正常';
    } finally {
      if (descriptor) Object.defineProperty(preview, 'requestFullscreen', descriptor);
      else delete preview.requestFullscreen;
    }
  }

  globalThis.pptOfflineTests = Object.freeze({ inspect, run, selectSlide, waitForIdle, waitForFullscreen, inspectFullscreen, rejectFullscreen });
  globalThis.runSlideExportTests = run;
})();
