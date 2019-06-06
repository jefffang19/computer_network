#ifndef CLIENT_H
#define CLIENT_H
#include "tcpconnect.h"

class Client_process : public Tcpconnect {
	public:
		Client_process() : Tcpconnect() { }
		void inithandshake();
		void endhandshake();
};

class Client {
	public:
		Client_process child;
		char fileBuffer[default_BUFFER_SIZE];
		void initInfo();
		void status();
		void recvfile();
};

#endif
		
