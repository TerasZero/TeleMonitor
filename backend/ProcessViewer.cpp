#include "ProcessViewer.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unistd.h>

namespace {
bool readUnsigned(const std::string& text, uint64_t& value) {
    if (text.empty() || text.find_first_not_of("0123456789") != std::string::npos)
        return false;
    std::istringstream input(text);
    return static_cast<bool>(input >> value);
}

void writeString(std::ostream& out, const std::string& text) {
    const char* hex = "0123456789abcdef";
    out << '"';
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 32 || c == 127)
            out << "\\u00" << hex[c >> 4] << hex[c & 15];
        else out << c;
    }
    out << '"';
}
}

bool parseStat(const std::string& text, procStats& proc) {
    const auto open = text.find('(');
    const auto close = text.rfind(')'); // Linux process names can themselves contain ')'.
    if (open == std::string::npos || close == std::string::npos || close <= open)
        return false;
    procStats parsed;
    std::istringstream pidInput(text.substr(0, open));
    if (!(pidInput >> parsed.pid) || parsed.pid <= 0) return false;
    pidInput >> std::ws;
    if (!pidInput.eof()) return false;
    parsed.name = text.substr(open + 1, close - open - 1);
    std::istringstream fields(text.substr(close + 1));
    if (!(fields >> parsed.curState) || parsed.curState.size() != 1) return false;
    uint64_t user = 0, system = 0;
    std::string token;
    for (int field = 4; field <= 22; ++field) {
        if (!(fields >> token)) return false;
        if (field == 14 && !readUnsigned(token, user)) return false;
        if (field == 15 && !readUnsigned(token, system)) return false;
        if (field == 22 && !readUnsigned(token, parsed.startTime)) return false;
    }
    if (user > std::numeric_limits<uint64_t>::max() - system) return false;
    parsed.cpuTime = user + system;
    proc = parsed;
    return true;
}

void parseMemory(std::istream& status, procStats& proc) {
    proc.hasMemory = false;
    proc.memUsage = 0;
    std::string line;
    while (std::getline(status, line)) {
        std::istringstream fields(line);
        std::string label, amount, unit;
        if (fields >> label && label == "VmRSS:") {
            proc.hasMemory = (fields >> amount >> unit) && unit == "kB"
                && readUnsigned(amount, proc.memUsage);
            return;
        }
    }
}

void calculateCpu(procStats& current, const procStats& previous, long ticksPerSecond) {
    current.hasCpu = false;
    current.cpuPercent = 0;
    const double seconds = std::chrono::duration<double>(current.sampledAt - previous.sampledAt).count();
    if (current.pid != previous.pid || current.startTime != previous.startTime
        || current.cpuTime < previous.cpuTime || seconds <= 0 || ticksPerSecond <= 0) return;
    current.cpuPercent = 100.0 * (current.cpuTime - previous.cpuTime) / ticksPerSecond / seconds;
    current.hasCpu = true;
}

void procMonitor::collectInfo() {
    const long ticksPerSecond = sysconf(_SC_CLK_TCK);
    if (ticksPerSecond <= 0) throw std::runtime_error("Cannot read the system clock tick rate");
    DIR* directory = opendir("/proc");
    if (!directory) throw std::runtime_error("Cannot open /proc; run TeleMonitor on Linux");
    std::vector<std::string> pids;
    while (auto* entry = readdir(directory)) {
        const std::string pid(entry->d_name);
        if (!pid.empty() && pid.find_first_not_of("0123456789") == std::string::npos)
            pids.push_back(pid);
    }
    closedir(directory);
    std::unordered_map<int, procStats> previous;
    for (const auto& proc : processes) previous.emplace(proc.pid, proc);
    processes.clear();
    for (const auto& pid : pids) {
        const std::string base = "/proc/" + pid;
        std::ifstream stat(base + "/stat");
        std::ostringstream text;
        text << stat.rdbuf();
        procStats proc;
        if (!stat || !parseStat(text.str(), proc) || std::to_string(proc.pid) != pid) continue;
        proc.sampledAt = std::chrono::steady_clock::now();
        std::ifstream status(base + "/status");
        parseMemory(status, proc);
        // Verify identity again because the PID may be reused between file reads.
        std::ifstream check(base + "/stat");
        std::ostringstream checkText;
        checkText << check.rdbuf();
        procStats identity;
        if (!check || !parseStat(checkText.str(), identity)
            || identity.pid != proc.pid || identity.startTime != proc.startTime) continue;
        const auto old = previous.find(proc.pid);
        if (old != previous.end()) calculateCpu(proc, old->second, ticksPerSecond);
        processes.push_back(proc);
    }
}

void procMonitor::sortPid() {
    std::sort(processes.begin(), processes.end(), [](const procStats& a, const procStats& b) {
        return a.pid < b.pid;
    });
}
void procMonitor::sortMemUsage() {
    std::sort(processes.begin(), processes.end(), [](const procStats& a, const procStats& b) {
        if (a.hasMemory != b.hasMemory) return a.hasMemory;
        return a.memUsage != b.memUsage ? a.memUsage > b.memUsage : a.pid < b.pid;
    });
}
void procMonitor::sortCpuUsage() {
    std::sort(processes.begin(), processes.end(), [](const procStats& a, const procStats& b) {
        if (a.hasCpu != b.hasCpu) return a.hasCpu;
        return a.cpuPercent != b.cpuPercent ? a.cpuPercent > b.cpuPercent : a.pid < b.pid;
    });
}

void procMonitor::printInfo(std::ostream& out) const {
    out << "PID\tCPU %\tMiB\tSTATE\tNAME\n" << std::fixed << std::setprecision(1);
    for (const auto& proc : processes) {
        out << proc.pid << '\t';
        if (proc.hasCpu) out << proc.cpuPercent; else out << '-';
        out << '\t';
        if (proc.hasMemory) out << proc.memUsage / 1024.0; else out << '-';
        out << '\t' << proc.curState << '\t';
        writeString(out, proc.name);
        out << '\n';
    }
    out << processes.size() << " processes. CPU: 100% = one core. '-' = unavailable.\n";
}

void procMonitor::printJson(std::ostream& out) const {
    out << "{\"processes\":[" << std::fixed << std::setprecision(2);
    bool first = true;
    for (const auto& proc : processes) {
        if (!first) out << ',';
        first = false;
        out << "{\"pid\":" << proc.pid << ",\"name\":";
        writeString(out, proc.name);
        out << ",\"state\":";
        writeString(out, proc.curState);
        out << ",\"cpuPercent\":";
        if (proc.hasCpu) out << proc.cpuPercent; else out << "null";
        out << ",\"memoryMiB\":";
        if (proc.hasMemory) out << proc.memUsage / 1024.0; else out << "null";
        out << '}';
    }
    out << "]}\n" << std::flush;
}
