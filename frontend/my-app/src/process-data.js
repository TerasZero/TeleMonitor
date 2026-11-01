function parseSnapshot(line) {
  const data = JSON.parse(line);
  const metric = (value) => value === null || (Number.isFinite(value) && value >= 0);
  if (!data || !Array.isArray(data.processes) || !data.processes.every((proc) =>
    proc && Number.isSafeInteger(proc.pid) && proc.pid > 0
    && typeof proc.name === 'string' && typeof proc.state === 'string'
    && proc.state.length === 1 && metric(proc.cpuPercent) && metric(proc.memoryMiB))) {
    throw new Error('Invalid process snapshot');
  }
  return data;
}

function compareProcesses(a, b, column, direction) {
  const left = a[column];
  const right = b[column];
  if (left === null || right === null) {
    if (left !== right) return left === null ? 1 : -1;
    return a.pid - b.pid;
  }
  const order = typeof left === 'string' ? left.localeCompare(right) : left - right;
  return order * direction || a.pid - b.pid;
}

module.exports = { parseSnapshot, compareProcesses };
