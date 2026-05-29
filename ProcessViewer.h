#include <vector>
#include <cstdint>



//process stats
typedef struct procStats{
	int pid;
	char* name;
	uint64_t cpuTime;
	uint64_t memUsage;
}procStat;

class procMonitor{
	private:
		vector<procStat> processes;
	
	public:
	//collecting the proc info
	void collectInfo();
};


