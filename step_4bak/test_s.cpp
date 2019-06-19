#include "tcpconnect.h"

int main(){
	Packet a;
	Tcpconnect tcp;
	tcp.myCreateSocket("127.0.0.1",5002);
	tcp.myConnect("127.0.0.1",5001);
	tcp.mySend(a);
	return 0;
}
