#include "ProcessViewer.h"
#include <iostream>
#include <stdexcept>
#include <thread>

int main(int argc, char* argv[]) {
    const bool json = argc == 2 && std::string(argv[1]) == "--json";
    if (argc > 1 && !json) {
        std::cerr << "Usage: telemonitor [--json]\n";
        return 1;
    }
    try {
        procMonitor monitor;
        monitor.collectInfo();
        if (json) {
            do {
                monitor.printJson(std::cout);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                monitor.collectInfo();
            } while (std::cout);
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            monitor.collectInfo();
            monitor.sortCpuUsage();
            monitor.printInfo(std::cout);
            std::string choice;
            while (std::cout << "Refresh/sort: 1 PID | 2 memory | 3 CPU | 4 quit\n> "
                   && std::getline(std::cin, choice)) {
                if (choice == "4") break;
                if (choice != "1" && choice != "2" && choice != "3") {
                    std::cout << "Enter 1, 2, 3, or 4.\n";
                    continue;
                }
                monitor.collectInfo();
                if (choice == "1") monitor.sortPid();
                if (choice == "2") monitor.sortMemUsage();
                if (choice == "3") monitor.sortCpuUsage();
                monitor.printInfo(std::cout);
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "TeleMonitor: " << error.what() << '\n';
        return 1;
    }
}
