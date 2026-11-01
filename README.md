# TeleMonitor

A light-weight linux process monitor. A C++ collector and electron table. Displays pid, process name, current cpu%, resident memory in MiB and status. Click on column headers to sort. Desktop monitor runs ~1x per second.

# To build the collector
You would need linux (or WSL) and g++, it uses C++11

cd backend
make
make test
./telemonitor

The menu in terminal will update when you select sorting order. '4' to exit. Rejecting invalid input. Exit gracefully on eoi.
If you want to send the JSON per line to electron then:
./telemonitor --json

stderr to report error. so it won't break the json stream.

# For the desktop to run
Use a recent node LTS release and npm. Have the collector built then:

cd frontend/my-app
npm ci
npm test
npm start

If linux, then on your desktop graphical session.
If windows, then have the collector built under WSL. Then do the npm commands from powershell. Uses default WSL not windows process info in electron. Put the project on a drive mounted under /mnt/ in WSL

So in this checkout it's:

cd /mnt/c/Users/mxz/TeleMonitor/backend
make test all

npm run package includes the collector as an extra resource from backend/telemonitor. Make sure to build that for your intended linux architecture first. Still need to build windows under wsl. Can't monitor macOS.

backend/ProcessViewer.cpp reads /proc and makes readings. Computes cpu usage. Outputs terminal or json.
backend/main.cpp selects terminal menu or json sampling loop.
frontend/my-app/src/main.js starts and stops the collector. Validates input snapshots. Send snapshots to window.
src/preload.js only the snapshot subscribe is exposed to interface.
src/renderer.js src/index.html src/index.css for rendering/sorting of the table.
backend/tests.cpp and frontend/my-app/tests/ Some tiny regress tests.

cpu % = (cpu ticks change) / (ticks per second) / (sec elapsed) * 100
100% is if it has one 100% core. Multithreaded can go higher than 100%. No cpu % for 1st sample. PIDs and start time of process used to match samples.
So if a PID is reused it gets a fresh start. Don't store previous samples.
Replace each time you collect. Not growing history.

Memory is the Linux VmRSS converted from KiB->MiB. Some rough measure of resident memory. Missing values are represented as — in window, - in terminal, null in json. 0 means actual zero value. Skip processes that have disappeared by collection time. Get the name from parenthesized stat.
Keeps spaces. Linux restriction on process name length.

Table becomes muted. It will say error/lagged instead of showing outdated readings as current. Text inserted for process names, not html. Window has no direct access to node. preload bridge is in electron isolation model. https://www.electronjs.org/docs/latest/tutorial/security

No server. no database. no history charts. no control process stuff. no ui framework.