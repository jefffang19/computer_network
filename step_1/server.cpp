#include "server.h"


char vfile[1000000];
//return buffer size
int rfile(){ 
	ifstream in("1.mp4", ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(in), {});
	cout << "1.mp4 " << "size: " << buffer.size() << " bytes" <<  endl;
	for(int i=0;i<buffer.size();++i){
		vfile[i] = buffer[i];
	}
	in.close();
	return buffer.size();
}

int main(){
	Server myServer;
	myServer.child.myCreateSocket(server_ip, server_port);
	myServer.initInfo();
	
	myServer.child.inithandshake();
	
	/*simulate 123.456kB file*/
	/*char data[123457];
	for(int i=0;i<123456;++i) data[i]='a';
	*/
	
	//if is master child, keep listen for new client's SYN
	while(myServer.child.masterchild) myServer.child.inithandshake();
	//if not master child, send video file
	
	if(!myServer.child.masterchild){cout << "debug\n";
		//read video 1.mp4
		int vsize;
		vsize = rfile();
	
		myServer.sendfile(vfile,vsize);
	}
	
	return 0;
}


Server::Server(){
	this->cwnd = 1;
	this->recv_wnd = default_BUFFER_SIZE;
	this->threshold = default_THRESHOLD;
	this->dupACKcnt = 0;
	this->child.masterchild = true; //the child of server class is master child
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

int Server::sendfile(const char *data, const int dataSize){
	printStatus();
	int datastart = 0,
		datalen = dataSize,
		segmentcnt = 0; //count how much segement divided
	
	while(datalen > 0){
		char segmentdata[child.MSS+10];  //divide data into segements of length MSS
		if(datalen < child.MSS){
			for(int i=0;i<datalen;++i) segmentdata[i] = data[i+datastart];
		}	
		else{
			for(int i=0;i<child.MSS;++i) segmentdata[i] = data[i+datastart];
			datastart += child.MSS;
		}
		++segmentcnt; 
		child.mySend(Packet(packetType::packet_data, this->child, segmentdata, child.MSS));
		//now wait for ack from client
		Packet recv_packet = child.myRecv();
		// if new ack then recv, else ignore, and don't - datalen
		if(child.isNewAck(recv_packet)){
			child.updateNum(recv_packet);
			datalen = datalen - child.MSS;
		}
	}
	child.mySend(Packet(packetType::packet_fin, this->child, NULL));
	return segmentcnt;
}
