#include <vector>
#include <cstdint>
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;


//process stats
typedef struct procStats{
	int pid;
	char* name;
	uint64_t cpuTime;
	uint64_t memUsage;
}procStats;

class procMonitor{
	private:
		vector<procStats> processes;
	
	public:
	//collecting the proc info
		void collectInfo();
};


