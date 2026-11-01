const { test } = require('node:test');
const assert = require('node:assert/strict');
const { parseSnapshot, compareProcesses } = require('../src/process-data');

test('accepts missing metrics and treats process names as data', () => {
  const row = { pid: 1, name: '<img onerror=alert(1)>', state: 'S', cpuPercent: null, memoryMiB: 0 };
  assert.deepEqual(parseSnapshot(JSON.stringify({ processes: [row] })).processes, [row]);
  for (const bad of [null, {}, { processes: [null] }, { processes: [{ ...row, cpuPercent: -1 }] },
    { processes: [{ ...row, pid: '1' }] }, { processes: [{ ...row, memoryMiB: '0' }] }]) {
    assert.throws(() => parseSnapshot(JSON.stringify(bad)));
  }
  assert.throws(() => parseSnapshot('broken json'));
});

test('sorts numbers numerically, keeps missing values last and ties stable', () => {
  const rows = [{ pid: 1, cpuPercent: null }, { pid: 2, cpuPercent: 9 }, { pid: 3, cpuPercent: 100 }, { pid: 4, cpuPercent: 9 }];
  assert.deepEqual([...rows].sort((a, b) => compareProcesses(a, b, 'cpuPercent', -1)).map(p => p.pid), [3, 2, 4, 1]);
  assert.deepEqual([...rows].sort((a, b) => compareProcesses(a, b, 'cpuPercent', 1)).map(p => p.pid), [2, 4, 3, 1]);
});
