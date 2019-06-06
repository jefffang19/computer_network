#include "server.h"


int main(){
	char data[] = "hello myfriend";
	Server myServer;
	myServer.child.myCreateSocket("127.0.0.1", 5002);
	myServer.child.myConnect("127.0.0.1", 5001);
	myServer.initInfo();
	myServer.sendfile(data);
	return 0;
}


Server::Server(){
	this->cwnd = 1;
	this->recv_wnd = default_BUFFER_SIZE;
	this->threshold = default_THRESHOLD;
	this->dupACKcnt = 0;
	state = tcpstate::tcp_none;
	strcpy(fileBuffer,"hello world");
}

void Server::initInfo(){
	cout << "====================" << endl <<
			"RTT delay = "	<< child.RTT << "ms" << endl <<
			"MSS = " << child.MSS << " bytes" << endl <<
			"threshold = " << child.THRESHOLD << " bytes" << endl <<
			"buffer size = "<< default_BUFFER_SIZE << " bytes" << endl <<
			"Server IP & port= " << child.addr(child.srcSocket) << endl <<
			"====================" << endl;
}

void Server::printStatus(){
	cout << "====================" << endl << 
			"cwnd = " << cwnd << ", " << endl <<
			"recv_wnd = " << recv_wnd << ", " << endl <<
			"threshold = " << threshold << endl <<
			"====================" << endl;
}

int Server::sendfile(const char *data){
	printStatus();
	int datastart = 0,
		datalen = strlen(data)-datastart,
		segmentcnt = 0; //count how much segement divided
	
	while(datalen > 0){
		char segmentdata[child.MSS];  //divide data into segements of length MSS
		if(datalen < child.MSS) strcpy(segmentdata,data);
		else{
			strncpy(segmentdata, data+datastart, datalen);
			datastart = datastart + child.MSS;
			++segmentcnt;
		}
		datalen = datalen - child.MSS;
		child.mySend(Packet(packetType::packet_data, this->child, segmentdata));	
	}
	return segmentcnt;
}
