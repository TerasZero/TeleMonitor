const { app, BrowserWindow } = require('electron');
const path = require('node:path');
const { spawn } = require('node:child_process');
const { createInterface } = require('node:readline');
const { parseSnapshot } = require('./process-data');

if (require('electron-squirrel-startup')) app.quit();

let collector;
let latest = null;

function createWindow() {
  const window = new BrowserWindow({
    width: 1000, height: 720, minWidth: 640, minHeight: 400,
    backgroundColor: '#f5f3ed', title: 'TeleMonitor',
    webPreferences: {
      preload: MAIN_WINDOW_PRELOAD_WEBPACK_ENTRY,
      contextIsolation: true, nodeIntegration: false, sandbox: true,
    },
  });
  window.setMenuBarVisibility(false);
  window.webContents.setWindowOpenHandler(() => ({ action: 'deny' }));
  window.webContents.on('will-navigate', (event) => event.preventDefault());
  const send = (update) => {
    latest = update;
    if (!window.isDestroyed()) window.webContents.send('processes', update);
  };
  window.webContents.on('did-finish-load', () => {
    if (latest) send(latest);
    if (collector) return;
    if (process.platform !== 'linux' && process.platform !== 'win32') {
      send({ error: 'TeleMonitor needs Linux or Windows with WSL.' });
      return;
    }
    const binary = app.isPackaged
      ? path.join(process.resourcesPath, 'telemonitor')
      : path.resolve(app.getAppPath(), '../../backend/telemonitor');
    // Windows development uses the default WSL distribution's Linux processes.
    const linuxPath = binary.replace(/^([A-Za-z]):/, (_, drive) => `/mnt/${drive.toLowerCase()}`).replace(/\\/g, '/');
    collector = process.platform === 'win32'
      ? spawn('wsl.exe', ['--exec', linuxPath, '--json'], { windowsHide: true })
      : spawn(binary, ['--json']);
    const child = collector;
    const lines = createInterface({ input: child.stdout });
    let failed = false;
    const fail = (error) => { failed = true; send({ error }); };
    lines.on('line', (line) => {
      if (failed) return;
      try { send(parseSnapshot(line)); }
      catch {
        fail('Collector sent invalid data. Restart TeleMonitor after rebuilding the backend.');
        child.kill();
      }
    });
    child.stderr.on('data', (chunk) => console.error(chunk.toString()));
    child.on('error', () => fail('Could not start collector. Build backend/telemonitor first; Windows also needs WSL.'));
    child.on('close', () => {
      lines.close();
      if (!failed) fail('Collector stopped. Restart TeleMonitor to reconnect.');
    });
  });
  window.loadURL(MAIN_WINDOW_WEBPACK_ENTRY);
}

app.whenReady().then(createWindow);
app.on('before-quit', () => {
  if (collector) {
    collector.stdout.destroy();
    collector.kill();
  }
});
app.on('window-all-closed', () => app.quit());
