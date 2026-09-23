const slides = [...document.querySelectorAll('.slide')];
const tabs = [...document.querySelectorAll('[role="tab"]')];
const layoutName = document.querySelector('#layout-name');
const layoutNote = document.querySelector('#layout-note');
const pageCounter = document.querySelector('#page-counter');
const exportButton = document.querySelector('#export-image');
const exportStatus = document.querySelector('#export-status');
const preview = document.querySelector('.preview');
const fullscreenButton = document.querySelector('#toggle-fullscreen');
const exitFullscreenButton = document.querySelector('#exit-fullscreen');
const fullscreenMessage = document.querySelector('#fullscreen-message');
const fullscreenStatus = document.querySelector('#fullscreen-status');
const fullscreenCounter = document.querySelector('#fullscreen-counter');
const exportButtons = [exportButton, document.querySelector('#fullscreen-export')];
let current = 0;
let exporting = false;
let fullscreenChanging = false;
let wasFullscreen = false;

function syncFullscreen() {
  const active = document.fullscreenElement === preview;
  fullscreenButton.setAttribute('aria-pressed', String(active));
  fullscreenButton.textContent = active ? '退出全屏' : '全屏演示';
  if (active) slides[current].focus({ preventScroll: true });
  else if (wasFullscreen) fullscreenButton.focus({ preventScroll: true });
  wasFullscreen = active;
}

async function toggleFullscreen() {
  const active = document.fullscreenElement === preview;
  if (fullscreenChanging || (exporting && !active) || !document.fullscreenEnabled) return;
  fullscreenChanging = true;
  fullscreenButton.disabled = true;
  exitFullscreenButton.disabled = true;
  fullscreenMessage.textContent = '';
  try {
    if (active) await document.exitFullscreen();
    else {
      await preview.requestFullscreen();
      fullscreenStatus.textContent = '← → 翻页 · Home / End 首尾页 · F 或 Esc 退出全屏';
      fullscreenStatus.dataset.error = 'false';
    }
  } catch {
    const message = '全屏失败：请重试，或使用新版 Edge / Chrome 打开此 HTML；无需联网或启动服务。';
    fullscreenMessage.textContent = message;
    fullscreenStatus.textContent = message;
  } finally {
    fullscreenChanging = false;
    fullscreenButton.disabled = false;
    exitFullscreenButton.disabled = false;
    syncFullscreen();
  }
}

fullscreenButton.addEventListener('click', toggleFullscreen);
exitFullscreenButton.addEventListener('click', toggleFullscreen);
document.addEventListener('fullscreenchange', syncFullscreen);
if (!document.fullscreenEnabled || !preview.requestFullscreen) {
  fullscreenButton.disabled = true;
  fullscreenMessage.textContent = '当前浏览器不支持全屏，请使用新版 Edge 或 Chrome；翻页与 PNG 导出仍可使用。';
}

function showSlide(index, updateUrl = true) {
  if (exporting) return;
  current = (index + slides.length) % slides.length;
  slides.forEach((slide, i) => { slide.hidden = i !== current; });
  tabs.forEach((tab, i) => {
    tab.setAttribute('aria-selected', String(i === current));
    tab.tabIndex = i === current ? 0 : -1;
  });
  const slide = slides[current];
  layoutName.textContent = slide.dataset.name;
  layoutNote.textContent = slide.dataset.note;
  pageCounter.textContent = `${String(current + 1).padStart(2, '0')} / ${String(slides.length).padStart(2, '0')}`;
  fullscreenCounter.textContent = pageCounter.textContent;
  if (updateUrl) {
    const url = new URL(location.href);
    url.searchParams.set('slide', slide.id);
    history.replaceState(null, '', url);
  }
}

function showUrlSlide() {
  const id = new URLSearchParams(location.search).get('slide');
  const index = slides.findIndex(slide => slide.id === id);
  showSlide(index < 0 ? 0 : index, false);
}

document.querySelectorAll('[data-slide]').forEach(control => {
  control.addEventListener('click', event => {
    event.preventDefault();
    showSlide(slides.findIndex(slide => slide.id === control.dataset.slide));
    if (control.tagName === 'A') slides[current].focus({ preventScroll: true });
  });
});

document.querySelectorAll('#previous, #fullscreen-previous').forEach(button => {
  button.addEventListener('click', () => showSlide(current - 1));
});
document.querySelectorAll('#next, #fullscreen-next').forEach(button => {
  button.addEventListener('click', () => showSlide(current + 1));
});
document.addEventListener('keydown', event => {
  if (event.altKey || event.ctrlKey || event.metaKey || event.shiftKey) return;
  if (event.key === 'Escape' && document.fullscreenElement === preview) {
    event.preventDefault();
    if (!event.repeat) toggleFullscreen();
    return;
  }
  if (event.target.closest('input, textarea, select, [contenteditable]')) return;
  if (event.key.toLowerCase() === 'f') {
    event.preventDefault();
    if (!event.repeat) toggleFullscreen();
    return;
  }
  const isTab = event.target.getAttribute('role') === 'tab';
  const isFullscreen = document.fullscreenElement === preview;
  let index;
  if (event.key === 'ArrowLeft') index = current - 1;
  else if (event.key === 'ArrowRight') index = current + 1;
  else if ((isTab || isFullscreen) && event.key === 'Home') index = 0;
  else if ((isTab || isFullscreen) && event.key === 'End') index = slides.length - 1;
  else return;
  event.preventDefault();
  const isInsideSlide = Boolean(event.target.closest('.slide'));
  showSlide(index);
  if (isTab) tabs[current].focus({ preventScroll: true });
  else if (isInsideSlide) slides[current].focus({ preventScroll: true });
});
window.addEventListener('popstate', showUrlSlide);
showUrlSlide();

async function renderSlidePng(slide) {
  await document.fonts.ready;
  await Promise.all([...slide.querySelectorAll('img')].map(image => image.decode()));
  const { width, height } = slide.getBoundingClientRect();
  const clone = slide.cloneNode(true);
  const originals = [slide, ...slide.querySelectorAll('*')];
  const copies = [clone, ...clone.querySelectorAll('*')];
  originals.forEach((element, index) => {
    const copy = copies[index];
    const style = getComputedStyle(element);
    for (const property of style) copy.style.setProperty(property, style.getPropertyValue(property));
    copy.removeAttribute('id');
    copy.removeAttribute('tabindex');
    copy.style.outline = 'none';
    if (element instanceof HTMLImageElement) {
      copy.src = element.currentSrc;
      copy.removeAttribute('srcset');
      copy.removeAttribute('loading');
    }
  });
  Object.assign(clone.style, { width: `${width}px`, height: `${height}px`, margin: '0', position: 'relative', left: '0', top: '0', transform: 'none' });
  const markup = new XMLSerializer().serializeToString(clone);
  const svg = `<svg xmlns="http://www.w3.org/2000/svg" width="1920" height="1080" viewBox="0 0 ${width} ${height}"><foreignObject width="${width}" height="${height}">${markup}</foreignObject></svg>`;
  const image = new Image();
  image.src = `data:image/svg+xml;charset=utf-8,${encodeURIComponent(svg)}`;
  await image.decode();
  const canvas = document.createElement('canvas');
  canvas.width = 1920;
  canvas.height = 1080;
  const context = canvas.getContext('2d');
  context.fillStyle = '#ffffff';
  context.fillRect(0, 0, canvas.width, canvas.height);
  context.drawImage(image, 0, 0, canvas.width, canvas.height);
  return new Promise((resolve, reject) => {
    canvas.toBlob(blob => blob ? resolve(blob) : reject(new Error('PNG 编码失败')), 'image/png');
  });
}

function setExportStatus(message, error = false) {
  for (const status of [exportStatus, fullscreenStatus]) {
    status.dataset.error = String(error);
    status.textContent = message;
  }
}

async function exportCurrentSlide() {
  if (exporting || fullscreenChanging) return;
  exporting = true;
  exportButtons.forEach(button => {
    button.disabled = true;
    button.textContent = '正在导出…';
  });
  setExportStatus('正在生成 1920 × 1080 PNG，请稍候…');
  const navigation = document.querySelector('.slide-navigation');
  const stage = document.querySelector('.stage');
  navigation.inert = true;
  stage.inert = true;
  const slide = slides[current];
  const prefix = document.title.replace(/[<>:"/\\|?*\x00-\x1f]/g, '_').trim().slice(0, 80) || 'maccura';
  const filename = `${prefix}-${String(current + 1).padStart(2, '0')}-${slide.id}-1920x1080.png`;
  try {
    const blob = await renderSlidePng(slide);
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = filename;
    document.body.append(link);
    try { link.click(); }
    finally {
      link.remove();
      setTimeout(() => URL.revokeObjectURL(url), 60000);
    }
    setExportStatus(`已生成 ${filename}，已发起下载，请查看浏览器下载列表。`);
  } catch {
    setExportStatus('导出失败：请确认内嵌底图正常显示，使用新版 Chrome 或 Edge 重新打开此文件后重试；无需联网或启动服务。', true);
  } finally {
    exporting = false;
    exportButtons.forEach(button => {
      button.disabled = false;
      button.textContent = '导出当前页 PNG';
    });
    navigation.inert = false;
    stage.inert = false;
  }
}
exportButtons.forEach(button => button.addEventListener('click', exportCurrentSlide));
