#include "ProcessViewer.h"
#include <dirent.h>

//Enumerates all processes through /proc and stores the pid's in processes vector
void procMonitor::collectInfo(){
	DIR *dirH;
	DIR *dirP;
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
		for(char c :pidStr){//filters out any dir with letters (proc dir are named by pid)
		//so filter out entries with letters "cpuinfo..."=non processes
			if(!isdigit(c)){
				isProc = false;
			}
		}
		if(isProc){//enters in the procs pid as a int
			procStats proc;
			int currPid = stoi(pidStr);
			cout << pidStr << endl;
			cout <<"testtt" <<endl;
			proc.pid = currPid;
			
			string statusPath = "/proc/" + pidStr + "/status";
			ifstream statusFile(statusPath); //file that contains proc info
			if(statusFile.is_open()){
				string statLine;
				while(getline(statusFile,statLine)){
					stringstream stringS(statLine); //temp stream
					string token;
					
					stringS >>token; //first word, usually its name
					if(token == "Name:"){
						stringS >>proc.name;
					}
					else if(token == "VmRSS:"){
						int mem;
						string sizeUnit;
						stringS >>mem>>sizeUnit;
						proc.memUsage = to_string(mem) + " " + sizeUnit;
            				} 
        			}
        			statusFile.close();
        		}
        		processes.push_back(proc);
        	}
        	entries = readdir(dirH);
        }
        cout << "Total amount :" << processes.size() << endl;
	closedir(dirH);     
        	
}

int main(){
	procMonitor mon1;
	mon1.collectInfo();
	return 0;
}
