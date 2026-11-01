#pragma once
#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

struct procStats {
    int pid = 0;
    std::string name;
    std::string curState;
    uint64_t cpuTime = 0;
    uint64_t startTime = 0;
    uint64_t memUsage = 0; // KiB from VmRSS
    bool hasMemory = false;
    double cpuPercent = 0;
    bool hasCpu = false;
    std::chrono::steady_clock::time_point sampledAt;
};

bool parseStat(const std::string& text, procStats& proc);
void parseMemory(std::istream& status, procStats& proc);
void calculateCpu(procStats& current, const procStats& previous, long ticksPerSecond);

class procMonitor {
    std::vector<procStats> processes;
public:
    void collectInfo();
    void printInfo(std::ostream& out) const;
    void printJson(std::ostream& out) const;
    void sortPid();
    void sortMemUsage();
    void sortCpuUsage();
};
