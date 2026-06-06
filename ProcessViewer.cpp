#include "ProcessViewer.h"

void procMonitor::collectInfo(){
	DIR *dirH;
	struct direct *entries;
	vector<string> proclist;
	dirH = opendir("/proc");
	bool isProc = true;
	if(dirH == NULL){
		return;
	}
	entries = readdir(dirH);
	while(entries != NULL){
		string name = entries-> d_name;
		for(char c :name){//filters out any dir with letters
			if(!isdigit(c)){
				isProc = false;
			}
		}
		if(isProc){//enters in the procs
			proclist.push_back(name);
		}
	}
	closedir(dirH);
}

int main(){
	procMonitor mon1;
	mon1.collectInfo();
	return 0;
}
