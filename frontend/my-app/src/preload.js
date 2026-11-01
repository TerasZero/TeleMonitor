const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('telemonitor', {
  onUpdate(callback) {
    const listener = (_event, update) => callback(update);
    ipcRenderer.on('processes', listener);
    return () => ipcRenderer.removeListener('processes', listener);
  },
});
