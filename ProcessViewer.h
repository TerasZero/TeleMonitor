#include <vector>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;


//process stats
typedef struct procStats{
	int pid;
	string name;
	uint64_t cpuTime;
	uint64_t memUsage;
	string curState; //running, sleeping, zombie, etc
}procStats;

class procMonitor{
	private:
		vector<procStats> processes;
	
	public:
	//collecting the proc info and printing info
		void collectInfo();
		void printInfo();
	
	//sort types
		void sortPid();
		void sortName();
		void sortMemUsage();
		void sortCpuTime();
};


