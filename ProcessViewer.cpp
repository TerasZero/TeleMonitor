#include "ProcessViewer.h"
#include <dirent.h>

//Enumerates all processes through /proc and stores the pid's in processes vector
void procMonitor::collectInfo(){
	DIR *dirH;
	struct dirent *entries;
	//look throuh /proc to find running processes
	dirH = opendir("/proc");
	bool isProc = true;
	
	if(dirH == NULL){
		return;
	}
	entries = readdir(dirH);
	while(entries != NULL){
		isProc = true;
		string pidStr = entries-> d_name;
		//auto iterate through each char in pid
		for(char c :pidStr){//filters out any directory with letters (proc dir are named by pid)
		//so filter out entries with letters "cpuinfo..."=non processes
			if(!isdigit(c)){
				isProc = false;
			}
		}
		if(isProc){
			procStats proc;
			int currPid = stoi(pidStr);
			proc.pid = currPid;
			//info from status file (human readable info: name, state, memory usage)
			string statusPath = "/proc/" + pidStr + "/status";
			ifstream statusFile(statusPath); //file that contains proc info
			if(statusFile.is_open()){
				string statusLine;
				while(getline(statusFile,statusLine)){
					stringstream stringS(statusLine); //temp stream
					string token;
					
					stringS >>token; //first word, usually its name
					if(token == "Name:"){
						stringS >>proc.name;
					}
					else if(token == "VmRSS:"){
						int mem;
						string sizeUnit;
						stringS >>mem>>sizeUnit;
						proc.memUsage = mem;
            		}
					else if(token == "State:"){
						stringS >>proc.curState;
					}	
        		}
        		statusFile.close();
			}

			//statfile contains cpu time info, and other 
			string statPath = "/proc/" + pidStr + "/stat";
			ifstream statFile(statPath);
			if(statFile.is_open()){
				proc.cpuTime = 0;
				string statLine;
				getline(statFile,statLine);
				stringstream stringS(statLine);
				string token;
				int count = 0;
				while(stringS >> token){
					//cout << count << ": " << token << endl;
					count++;
					if(count == 14 || count == 15){
						proc.cpuTime += stoi(token); //line 14: utime(user time) +line 15: stime(system time)
					}
				}
				statFile.close();
        	}
        	processes.push_back(proc);
        }
        entries = readdir(dirH);
    }
	closedir(dirH);     
        	
}

void procMonitor::printInfo(){
	for(procStats proc: processes){
		cout << "PID: " << proc.pid << ", Name: " << proc.name << ", Memory Usage: " << proc.memUsage << ", CPU Time: " << proc.cpuTime << ", State: " << proc.curState << endl;
	}
	cout << "Total amount :" << processes.size() << endl;
}

bool comparePid(procStats& a, procStats& b){
	return a.pid < b.pid;
}

void procMonitor::sortPid(){
	sort(processes.begin(),processes.end(),comparePid); 
}

int main(){
	procMonitor mon1;
	mon1.collectInfo();
	mon1.printInfo();
	return 0;
}
