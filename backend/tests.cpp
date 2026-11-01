#include "ProcessViewer.h"
#include <cassert>
#include <iostream>
#include <sstream>

std::string statText(const std::string& user = "3000000000", const std::string& system = "25") {
    std::ostringstream text;
    text << "42 (my ) worker\n) R";
    for (int field = 4; field <= 22; ++field)
        text << ' ' << (field == 14 ? user : field == 15 ? system : field == 22 ? "123" : "0");
    return text.str();
}

int main() {
    procStats proc;
    assert(parseStat(statText(), proc));
    assert(proc.pid == 42 && proc.name == "my ) worker\n" && proc.curState == "R");
    assert(proc.cpuTime == 3000000025ULL && proc.startTime == 123);
    assert(!parseStat("42 (incomplete) R 0", proc));
    assert(!parseStat(statText("-1"), proc));
    assert(!parseStat(statText("12oops"), proc));
    assert(!parseStat(statText("18446744073709551616"), proc));
    assert(!parseStat(statText("18446744073709551615", "1"), proc));
    std::istringstream memory("Name:\tmy worker\nVmRSS:\t3000000000 kB\n");
    parseMemory(memory, proc);
    assert(proc.hasMemory && proc.memUsage == 3000000000ULL);
    std::istringstream missing("Name:\tkernel worker\n");
    parseMemory(missing, proc);
    assert(!proc.hasMemory);
    std::istringstream invalid("VmRSS: -1 kB\n");
    parseMemory(invalid, proc);
    assert(!proc.hasMemory);
    procStats before = proc;
    proc.sampledAt = before.sampledAt + std::chrono::seconds(2);
    proc.cpuTime += 100;
    calculateCpu(proc, before, 100);
    assert(proc.hasCpu && proc.cpuPercent == 50);
    proc.cpuTime = before.cpuTime;
    calculateCpu(proc, before, 100);
    assert(proc.hasCpu && proc.cpuPercent == 0);
    proc.startTime++;
    calculateCpu(proc, before, 100);
    assert(!proc.hasCpu);
    proc = before;
    calculateCpu(proc, before, 100);
    assert(!proc.hasCpu);
    proc.sampledAt += std::chrono::seconds(1);
    proc.cpuTime--;
    calculateCpu(proc, before, 100);
    assert(!proc.hasCpu);
    calculateCpu(proc, before, 0);
    assert(!proc.hasCpu);
    std::cout << "Parser and CPU tests passed\n";
}
