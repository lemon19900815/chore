'use strict';

// Copy-only isolation is not a sandbox; run only on trusted local HTML.
const fs = require('node:fs/promises');
const path = require('node:path');
const os = require('node:os');
const { spawn } = require('node:child_process');
const { pathToFileURL } = require('node:url');
const { EventEmitter } = require('node:events');
const assert = require('node:assert/strict');

const short = value => String(value?.message || value).replace(/\s+/g, ' ').slice(0, 320);
const safeUrl = url => url?.startsWith('data:') ? `${url.slice(0, 70)}… (${url.length} chars)` : url;
function bounded(promise, timeout, label) {
  let timer;
  return Promise.race([promise, new Promise((_, reject) => {
    timer = setTimeout(() => reject(new Error(`${label} timed out`)), timeout);
  })]).finally(() => clearTimeout(timer));
}

class CDP {
  constructor(endpoint) {
    this.socket = new WebSocket(endpoint);
    this.events = new EventEmitter();
    this.pending = new Map();
    this.waiters = new Set();
    this.sequence = 0;
    this.closed = false;
    this.opened = new Promise((resolve, reject) => {
      this.socket.addEventListener('open', resolve, { once: true });
      this.socket.addEventListener('error', () => reject(new Error('DevTools connection failed')), { once: true });
      this.socket.addEventListener('close', () => reject(new Error('DevTools connection closed')), { once: true });
    });
    this.opened.catch(() => {});
    this.socket.addEventListener('message', event => {
      let message;
      try { message = JSON.parse(event.data); } catch { return; }
      if (message.id) {
        const entry = this.pending.get(message.id);
        if (!entry) return;
        this.pending.delete(message.id);
        clearTimeout(entry.timer);
        if (message.error) entry.reject(new Error(`${entry.method}: ${message.error.message}`));
        else entry.resolve(message.result);
      } else {
        this.events.emit(message.method, message.params, message.sessionId);
      }
    });
    this.socket.addEventListener('close', () => this.cancel(new Error('DevTools connection closed')));
    this.socket.addEventListener('error', () => this.cancel(new Error('DevTools connection failed')));
  }
  send(method, params = {}, sessionId, timeout = 30000) {
    if (this.closed || this.socket.readyState !== WebSocket.OPEN) return Promise.reject(new Error('DevTools is not connected'));
    return new Promise((resolve, reject) => {
      const id = ++this.sequence;
      const timer = setTimeout(() => {
        this.pending.delete(id);
        reject(new Error(`${method} timed out`));
      }, timeout);
      this.pending.set(id, { resolve, reject, timer, method });
      try { this.socket.send(JSON.stringify({ id, method, params, sessionId })); }
      catch (error) { this.pending.delete(id); clearTimeout(timer); reject(error); }
    });
  }
  once(name, predicate = () => true, timeout = 30000) {
    let cancel;
    const promise = new Promise((resolve, reject) => {
      const finish = (error, value) => {
        clearTimeout(timer);
        this.events.off(name, handler);
        this.waiters.delete(cancel);
        error ? reject(error) : resolve(value);
      };
      const handler = (value, session) => { if (predicate(value, session)) finish(null, value); };
      const timer = setTimeout(() => finish(new Error(`${name} timed out`)), timeout);
      cancel = error => finish(error);
      this.waiters.add(cancel);
      this.events.on(name, handler);
    });
    // An earlier command may fail before the caller awaits this event.
    promise.catch(() => {});
    return promise;
  }
  cancel(error) {
    this.closed = true;
    for (const entry of this.pending.values()) { clearTimeout(entry.timer); entry.reject(error); }
    this.pending.clear();
    for (const cancel of [...this.waiters]) cancel(error);
  }
  close() {
    this.cancel(new Error('Verifier finished'));
    this.events.removeAllListeners();
    this.socket.close();
  }
}

async function executable(explicit) {
  if (explicit) {
    const candidate = path.resolve(explicit);
    assert((await fs.stat(candidate)).isFile(), 'Browser executable is not a file');
    return candidate;
  }
  assert(process.platform === 'win32', 'Pass an explicit Edge/Chrome executable on non-Windows systems');
  const roots = [...new Set([process.env.ProgramFiles, process.env['ProgramFiles(x86)'], process.env.ProgramW6432,
    process.env.LOCALAPPDATA, 'C:/Program Files', 'C:/Program Files (x86)'].filter(Boolean))];
  for (const product of ['Microsoft/Edge/Application/msedge.exe', 'Google/Chrome/Application/chrome.exe']) {
    for (const root of roots) {
      const candidate = path.join(root, product);
      if (await fs.stat(candidate).then(stat => stat.isFile(), () => false)) return candidate;
    }
  }
  throw new Error('Edge/Chrome not found; pass its executable as the second argument');
}

function startup(browser) {
  return new Promise((resolve, reject) => {
    let output = '';
    const finish = (error, value) => {
      clearTimeout(timer);
      browser.stderr.off('data', data);
      browser.off('error', failed);
      browser.off('exit', exited);
      error ? reject(error) : resolve(value);
    };
    const data = chunk => {
      output = (output + chunk.toString()).slice(-8192);
      const match = output.match(/DevTools listening on (ws:\/\/[^\s]+)/);
      if (match) finish(null, match[1]);
    };
    const failed = error => finish(new Error(`Browser startup: ${short(error)}`));
    const exited = code => finish(new Error(`Browser exited during startup (${code})`));
    const timer = setTimeout(() => finish(new Error('Browser startup timed out')), 20000);
    browser.stderr.on('data', data);
    browser.once('error', failed);
    browser.once('exit', exited);
  });
}

async function stopBrowser(browser, cdp, exited, isRunning) {
  if (cdp && !cdp.closed) await cdp.send('Browser.close', {}, undefined, 3000).catch(() => {});
  if (cdp) cdp.close(); // Also rejects outstanding waits and clears their timers.
  if (!browser || !isRunning()) return;
  try { await bounded(exited, 5000, 'Browser shutdown'); } catch { /* Fall back to our own PID only. */ }
  if (!isRunning()) return;
  if (process.platform === 'win32') {
    // Never /IM, never kill by executable name: this PID uses a unique profile.
    const killer = spawn('taskkill.exe', ['/PID', String(browser.pid), '/T', '/F'], { stdio: 'ignore', windowsHide: true });
    await bounded(new Promise((resolve, reject) => {
      killer.once('error', reject);
      killer.once('exit', code => code === 0 ? resolve() : reject(new Error(`taskkill exited ${code}`)));
    }), 5000, 'Browser process cleanup').catch(error => { if (isRunning()) throw error; });
  } else {
    browser.kill('SIGKILL');
  }
  await bounded(exited, 5000, 'Browser exit');
}

async function main() {
  assert(Number(process.versions.node.split('.')[0]) >= 22 && typeof WebSocket === 'function', 'Node 22+ with built-in WebSocket is required');
  const args = process.argv.slice(2);
  assert(args.length >= 1 && args.length <= 2 && !args.some(arg => arg.startsWith('--')),
    'Usage: node verify-offline.cjs <trusted-local.html> [Edge/Chrome executable]');
  const source = path.resolve(args[0]);
  assert(/\.html?$/i.test(source) && (await fs.stat(source)).isFile(), 'Input must be a trusted local HTML file');
  const browserPath = await executable(args[1]);
  const testSource = await fs.readFile(path.join(__dirname, 'browser-tests.js'), 'utf8');
  const evidence = await fs.mkdtemp(path.join(os.tmpdir(), 'maccura-ppt-offline-'));
  const profile = path.join(evidence, 'profile');
  const downloads = path.join(evidence, 'downloads');
  const isolated = path.join(evidence, 'html');
  const report = { source, browser: browserPath, evidence, started: new Date().toISOString(),
    httpRequests: [], externalFiles: [], errors: [], pngs: [], failures: [] };
  let browser;
  let cdp;
  let running = false;
  let exited = Promise.resolve();
  let brokenPhase = null;
  const interrupted = signal => {
    report.failures.push(`Interrupted: ${signal}`);
    if (cdp) cdp.close();
    else if (browser && running) browser.kill();
  };
  const sigint = () => interrupted('SIGINT');
  const sigterm = () => interrupted('SIGTERM');
  process.once('SIGINT', sigint);
  process.once('SIGTERM', sigterm);
  try {
    await Promise.all([fs.mkdir(profile), fs.mkdir(downloads), fs.mkdir(isolated)]);
    const copy = path.join(isolated, path.basename(source));
    await fs.copyFile(source, copy); // Deliberately copy exactly ONE HTML file, no assets.
    report.copy = copy;
    browser = spawn(browserPath, [
      '--headless=new', '--no-first-run', '--no-default-browser-check', '--disable-background-networking',
      '--remote-debugging-address=127.0.0.1', '--remote-debugging-port=0', `--user-data-dir=${profile}`, 'about:blank'
    ], { stdio: ['ignore', 'ignore', 'pipe'], windowsHide: true });
    running = Boolean(browser.pid);
    exited = new Promise(resolve => {
      browser.once('exit', () => { running = false; resolve(); });
      browser.once('error', () => { running = false; resolve(); });
    });
    const endpoint = await startup(browser);
    cdp = new CDP(endpoint);
    await bounded(cdp.opened, 10000, 'DevTools connection');
    const { targetId } = await cdp.send('Target.createTarget', { url: 'about:blank' });
    const { sessionId } = await cdp.send('Target.attachToTarget', { targetId, flatten: true });
    const page = (method, params = {}, timeout) => cdp.send(method, params, sessionId, timeout);
    const evaluate = async (expression, timeout = 30000) => {
      const result = await page('Runtime.evaluate', { expression, awaitPromise: true, returnByValue: true }, timeout);
      if (result.exceptionDetails) throw new Error(short(result.exceptionDetails.exception?.description || result.exceptionDetails.text));
      return result.result.value;
    };
    const recordError = (kind, message, url, allowImageError = false) => {
      const imageFailure = brokenPhase && allowImageError && (url === brokenPhase.url ||
        /source image cannot be decoded|image decoding failed|图片解码失败/i.test(message));
      report.errors.push({ kind, message: short(message), url: safeUrl(url), ignoredIntentionalImageFailure: Boolean(imageFailure) });
    };
    cdp.events.on('Runtime.bindingCalled', (data, session) => {
      if (session !== sessionId || data.name !== '__pptOfflineTestPhase') return;
      try {
        const marker = JSON.parse(data.payload);
        if (marker.phase === 'broken-image') brokenPhase = marker.active ? marker : null;
      } catch { report.failures.push('Invalid browser-test phase marker'); }
    });
    cdp.events.on('Runtime.exceptionThrown', (data, session) => {
      if (session === sessionId) recordError('exception', data.exceptionDetails.exception?.description || data.exceptionDetails.text, data.exceptionDetails.url);
    });
    cdp.events.on('Runtime.consoleAPICalled', (data, session) => {
      if (session === sessionId && data.type === 'error') {
        recordError('console.error', data.args.map(arg => arg.value ?? arg.description ?? arg.type).join(' '), undefined, true);
      }
    });
    cdp.events.on('Log.entryAdded', (data, session) => {
      if (session === sessionId && data.entry.level === 'error') {
        recordError(`log:${data.entry.source}`, data.entry.text, data.entry.url, true);
      }
    });
    const fileUrl = pathToFileURL(copy).href;
    cdp.events.on('Network.requestWillBeSent', (data, session) => {
      if (session !== sessionId) return;
      const url = data.request.url;
      if (/^https?:/i.test(url)) report.httpRequests.push({ url, type: data.type, method: data.request.method });
      if (/^file:/i.test(url) && url.split(/[?#]/)[0] !== fileUrl) report.externalFiles.push(url);
    });
    await page('Page.enable');
    await page('Runtime.enable');
    await page('Log.enable');
    await page('Runtime.addBinding', { name: '__pptOfflineTestPhase' });
    await page('Network.enable');
    await page('Network.setCacheDisabled', { cacheDisabled: true });
    await page('Network.setBlockedURLs', { urls: ['http://*', 'https://*', 'ws://*', 'wss://*'] });
    await page('Network.emulateNetworkConditions', { offline: true, latency: 0, downloadThroughput: 0, uploadThroughput: 0 });
    await page('Emulation.setDeviceMetricsOverride', { width: 1280, height: 960, deviceScaleFactor: 1, mobile: false });
    await cdp.send('Browser.setDownloadBehavior', { behavior: 'allow', downloadPath: downloads, eventsEnabled: true });
    const loaded = cdp.once('Page.loadEventFired', (_, session) => session === sessionId);
    const navigation = await page('Page.navigate', { url: fileUrl });
    assert(!navigation.errorText, `Navigation failed: ${navigation.errorText}`);
    await loaded;
    report.environment = await evaluate('({protocol:location.protocol, online:navigator.onLine})');
    await evaluate(testSource);
    report.document = await evaluate('pptOfflineTests.inspect()', 60000);
    await evaluate('pptOfflineTests.selectSlide(0); document.fonts.ready.then(() => new Promise(requestAnimationFrame))');
    const screenshot = await page('Page.captureScreenshot', { format: 'png' });
    await fs.writeFile(path.join(evidence, 'cover-browser.png'), Buffer.from(screenshot.data, 'base64'));
    const click = async selector => {
      const point = await evaluate(`(() => {
        const element = document.querySelector(${JSON.stringify(selector)});
        if (!element) throw new Error('Missing control: ' + ${JSON.stringify(selector)});
        element.scrollIntoView({ block: 'center' });
        const rect = element.getBoundingClientRect();
        return { x: rect.x + rect.width / 2, y: rect.y + rect.height / 2 };
      })()`);
      await page('Input.dispatchMouseEvent', { type: 'mousePressed', button: 'left', clickCount: 1, ...point });
      await page('Input.dispatchMouseEvent', { type: 'mouseReleased', button: 'left', clickCount: 1, ...point });
    };
    const key = async (key, code, windowsVirtualKeyCode) => {
      await page('Input.dispatchKeyEvent', { type: 'keyDown', key, code, windowsVirtualKeyCode });
      await page('Input.dispatchKeyEvent', { type: 'keyUp', key, code, windowsVirtualKeyCode });
    };
    report.fullscreen = [];
    report.fullscreen.push(await evaluate('pptOfflineTests.rejectFullscreen()'));
    await click('#toggle-fullscreen');
    await evaluate('pptOfflineTests.waitForFullscreen(true)');
    report.fullscreen.push(await evaluate('pptOfflineTests.inspectFullscreen(true)'));
    for (const [name, code, keyCode, index] of [
      ['ArrowRight', 'ArrowRight', 39, 1 % report.document.slides.length],
      ['End', 'End', 35, report.document.slides.length - 1],
      ['Home', 'Home', 36, 0]
    ]) {
      await key(name, code, keyCode);
      assert.equal(await evaluate('document.querySelector(".slide:not([hidden])").id'), report.document.slides[index].id,
        `${name}: fullscreen keyboard navigation failed`);
    }
    await click('#fullscreen-next');
    await click('#fullscreen-previous');
    assert.equal(await evaluate('document.querySelector(".slide:not([hidden])").id'), report.document.slides[0].id);
    for (const [width, height] of [[1920, 1080], [390, 844]]) {
      await page('Emulation.setDeviceMetricsOverride', { width, height, deviceScaleFactor: 1, mobile: false });
      await evaluate('new Promise(requestAnimationFrame)');
      report.fullscreen.push(await evaluate('pptOfflineTests.inspectFullscreen(true)'));
      const image = await page('Page.captureScreenshot', { format: 'png' });
      await fs.writeFile(path.join(evidence, `fullscreen-${width}x${height}.png`), Buffer.from(image.data, 'base64'));
    }
    await click('#exit-fullscreen');
    await evaluate('pptOfflineTests.waitForFullscreen(false)');
    report.fullscreen.push(await evaluate('pptOfflineTests.inspectFullscreen(false)'));
    await page('Emulation.setDeviceMetricsOverride', { width: 1280, height: 960, deviceScaleFactor: 1, mobile: false });
    await evaluate('document.querySelector("#toggle-fullscreen").focus()');
    await key('f', 'KeyF', 70);
    await evaluate('pptOfflineTests.waitForFullscreen(true)');
    await key('f', 'KeyF', 70);
    await evaluate('pptOfflineTests.waitForFullscreen(false)');
    assert.equal(await evaluate('document.activeElement.id'), 'toggle-fullscreen', 'Exit must restore keyboard focus');
    await click('#toggle-fullscreen');
    await evaluate('pptOfflineTests.waitForFullscreen(true)');
    await key('Escape', 'Escape', 27);
    await evaluate('pptOfflineTests.waitForFullscreen(false)');
    report.fullscreen.push('Esc 退出');
    await click('#toggle-fullscreen');
    await evaluate('pptOfflineTests.waitForFullscreen(true)');
    report.fullscreen.push('按钮/F 进入退出、首尾及左右翻页、窄窗口、焦点恢复');
    try {
      report.browserTests = await evaluate('runSlideExportTests()', 60000 + report.document.slides.length * 20000);
      report.failures.push(...report.browserTests.failures);
    } catch (error) { report.failures.push(`Browser tests: ${short(error)}`); }

    const transfers = new Map();
    cdp.events.on('Browser.downloadWillBegin', data => {
      transfers.set(data.guid, { ...data, state: 'inProgress' });
    });
    cdp.events.on('Browser.downloadProgress', data => {
      const entry = transfers.get(data.guid);
      if (entry) Object.assign(entry, data);
    });
    for (let i = 0; i < report.document.slides.length; i++) {
      const slide = report.document.slides[i];
      await evaluate(`pptOfflineTests.selectSlide(${i}) && true`);
      const started = cdp.once('Browser.downloadWillBegin');
      const [, transfer] = await Promise.all([
        click('#fullscreen-export').then(() => evaluate('pptOfflineTests.waitForIdle()')), started
      ]);
      let progress = transfers.get(transfer.guid);
      if (progress.state !== 'completed' && progress.state !== 'canceled') {
        progress = await cdp.once('Browser.downloadProgress', data => data.guid === transfer.guid && data.state !== 'inProgress');
      }
      assert.equal(progress.state, 'completed', `${slide.id}: download canceled`);
      report.pngs.push({ id: slide.id, filename: transfer.suggestedFilename, guid: transfer.guid });
    }
    const files = await fs.readdir(downloads);
    assert.equal(files.length, report.document.slides.length, 'Real PNG download file count differs from slide count');
    assert.equal(transfers.size, report.document.slides.length, 'Duplicate or missing real download events');
    assert.equal(new Set(report.pngs.map(png => png.filename)).size, files.length, 'Duplicate real download filenames');
    for (const png of report.pngs) {
      assert(files.includes(png.filename) && path.basename(png.filename) === png.filename, 'Missing or invalid download filename');
      const handle = await fs.open(path.join(downloads, png.filename), 'r');
      try {
        const header = Buffer.alloc(33);
        const { bytesRead } = await handle.read(header, 0, header.length, 0);
        assert.equal(bytesRead, 33, `${png.id}: truncated PNG`);
        assert(header.subarray(0, 8).equals(Buffer.from([137, 80, 78, 71, 13, 10, 26, 10])), `${png.id}: invalid PNG signature`);
        assert.equal(header.readUInt32BE(8), 13, `${png.id}: invalid IHDR length`);
        assert.equal(header.toString('ascii', 12, 16), 'IHDR', `${png.id}: missing IHDR`);
        png.width = header.readUInt32BE(16);
        png.height = header.readUInt32BE(20);
        png.bytes = (await handle.stat()).size;
        assert.equal(png.width, 1920, `${png.id}: wrong PNG width`);
        assert.equal(png.height, 1080, `${png.id}: wrong PNG height`);
      } finally { await handle.close(); }
    }
    report.downloads = [...transfers.values()];
    await evaluate('document.exitFullscreen()');
    await evaluate('pptOfflineTests.waitForFullscreen(false)');
    report.fullscreen.push(await evaluate('pptOfflineTests.inspectFullscreen(false)'));
    report.normalBrowserTests = await evaluate('runSlideExportTests()', 60000 + report.document.slides.length * 20000);
    report.failures.push(...report.normalBrowserTests.failures);
    await page('Emulation.setDeviceMetricsOverride', { width: 390, height: 844, deviceScaleFactor: 1, mobile: false });
    await evaluate('pptOfflineTests.selectSlide(0); new Promise(requestAnimationFrame)');
    assert(await evaluate('document.documentElement.scrollWidth <= innerWidth'), 'Normal preview overflows a narrow window');
    const narrow = await page('Page.captureScreenshot', { format: 'png' });
    await fs.writeFile(path.join(evidence, 'normal-390x844.png'), Buffer.from(narrow.data, 'base64'));
    // A final round trip also drains preceding console/network events.
    await evaluate('pptOfflineTests.selectSlide(0); true');
  } catch (error) {
    report.failures.push(short(error));
  } finally {
    try { await stopBrowser(browser, cdp, exited, () => running); }
    catch (error) { report.failures.push(`Browser cleanup: ${short(error)}`); }
    process.off('SIGINT', sigint);
    process.off('SIGTERM', sigterm);
    if (report.httpRequests.length) report.failures.push(`HTTP requests must be zero (got ${report.httpRequests.length})`);
    if (report.externalFiles.length) report.failures.push(`External file dependencies: ${report.externalFiles.length}`);
    const errors = report.errors.filter(error => !error.ignoredIntentionalImageFailure);
    if (errors.length) report.failures.push(`${errors.length} JavaScript/console errors: ${errors[0].message}`);
    report.pass = report.failures.length === 0;
    report.finished = new Date().toISOString();
    await fs.writeFile(path.join(evidence, 'verification.json'), JSON.stringify(report, null, 2));
  }
  if (report.pass) console.log(`PASS: ${report.pngs.length} real PNGs (1920x1080), offline file://, 0 HTTP requests.\nEvidence: ${evidence}`);
  else {
    console.error(`FAIL: ${short(report.failures[0])}\nEvidence: ${evidence}`);
    process.exitCode = 1;
  }
}

main().catch(error => { console.error(`FAIL: ${short(error)}`); process.exitCode = 1; });
