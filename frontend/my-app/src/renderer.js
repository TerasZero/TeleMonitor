import './index.css';
const { compareProcesses } = require('./process-data');

const body = document.querySelector('tbody');
const status = document.querySelector('#status');
const count = document.querySelector('#count');
let processes = [];
let column = 'cpuPercent';
let direction = -1;
let lastUpdate = Date.now();
let failed = false;
const states = { R: 'Running', S: 'Sleeping', D: 'Waiting', Z: 'Zombie', T: 'Stopped', t: 'Tracing', I: 'Idle', X: 'Dead' };

function render() {
  const rows = document.createDocumentFragment();
  for (const proc of [...processes].sort((a, b) => compareProcesses(a, b, column, direction))) {
    const row = document.createElement('tr');
    const values = [proc.pid, proc.name, proc.cpuPercent === null ? '—' : proc.cpuPercent.toFixed(1),
      proc.memoryMiB === null ? '—' : proc.memoryMiB.toFixed(1), states[proc.state] || proc.state];
    for (const value of values) {
      const cell = document.createElement('td');
      cell.textContent = value;
      row.append(cell);
    }
    rows.append(row);
  }
  if (!processes.length) {
    const row = document.createElement('tr');
    const cell = document.createElement('td');
    cell.colSpan = 5;
    cell.textContent = 'No readable processes in this snapshot.';
    row.append(cell);
    rows.append(row);
  }
  body.replaceChildren(rows);
  count.textContent = `${processes.length} processes`;
  document.querySelectorAll('th').forEach((header) => {
    const active = header.querySelector('button').dataset.column === column;
    header.setAttribute('aria-sort', active ? direction === 1 ? 'ascending' : 'descending' : 'none');
  });
}

document.querySelectorAll('th button').forEach((button) => {
  button.addEventListener('click', () => {
    const next = button.dataset.column;
    direction = next === column ? -direction : ['cpuPercent', 'memoryMiB'].includes(next) ? -1 : 1;
    column = next;
    render();
  });
});

window.telemonitor.onUpdate((update) => {
  lastUpdate = Date.now();
  failed = Boolean(update.error);
  status.classList.toggle('error', failed);
  if (failed) {
    status.textContent = update.error;
    body.classList.add('stale');
    return;
  }
  processes = update.processes;
  status.textContent = 'Live · sampling every second';
  body.classList.remove('stale');
  render();
});
setInterval(() => {
  if (!failed && Date.now() - lastUpdate > 5000) {
    status.textContent = 'Updates delayed · showing the last snapshot';
    status.classList.add('error');
    body.classList.add('stale');
  }
}, 1000);
