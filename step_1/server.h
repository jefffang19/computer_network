#include "tcpconnect.h"

class Server_process : public Tcpconnect {
	public:
		pid_t childpid[num_of_clients];
		int childcnt;
		vector<int> req_files;
		bool masterchild;
		void inithandshake();
		void endhandshake();
		Server_process(){
			childcnt = 0;
			masterchild = false;
		}
};

class Server {
	public:
		Server_process child;
		char fileBuffer[default_BUFFER_SIZE];
		int cwnd;
		int recv_wnd;
		int threshold;
		int dupACKcnt;
		tcpstate state;
		
		Server();
		void initInfo();
		void printStatus();
		int sendfile(const char *data, const int dataSize = 0);  //return how many segments
};

