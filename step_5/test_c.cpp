#include "packet.hpp"
#include "tcpconnect.hpp"

int main(){
	Packet a;
	cout << a.data << endl;
	cout << sizeof(a.header) << endl;
	usleep(5000);
	Tcpconnect tcp;
	tcp.myCreateSocket("127.0.0.1",5001);
	tcp.myConnect("127.0.0.1",5002);
	tcp.mySend(a);
	return 0;
}
