#include "server.h"
#define MAXVIDEOSIZE 1000000

char vfile[MAXVIDEOSIZE];

Server myServer;

//return buffer size
int rfile(int num){ 
	stringstream ss;
	ss << num;
	string n = ss.str();
	ifstream in( n + ".mp4", ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(in), {});
	cout << n << ".mp4 " << "size: " << buffer.size() << " bytes" <<  endl;
	for(int i=0;i<buffer.size();++i){
		vfile[i] = buffer[i];
	}
	in.close();
	return buffer.size();
}

int main(){
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
	
	if(!myServer.child.masterchild){
		//read video 1.mp4
		int vsize;
		for(int i=0;i<myServer.child.req_files.size();++i){
			vsize = rfile(myServer.child.req_files[i]);
			myServer.sendfile(vfile,vsize);
		}
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
			"loss = "<< child.loss << " %" << endl <<
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
		}
		child.mySend(Packet(packetType::packet_data, this->child, segmentdata, child.MSS));
		
		//now wait for ack from client
		Packet recv_packet;
		//use timeoutable recv()
		//timeout 5 sec
		bool timeout = false;
		try{ recv_packet = child.timeout_recv(5, this->child); }
		catch(runtime_error &e){
			cout << e.what() << endl;
			timeout = true;
		}
		if(timeout) continue; //resend
		
		// if new ack then recv, else ignore, and don't - datalen
		else if(child.isNewAck(recv_packet)){
			child.updateNum(recv_packet);
			datalen = datalen - child.MSS;
			datastart += child.MSS;
			++segmentcnt; 
		}
	}
	child.mySend(Packet(packetType::packet_fin, this->child, NULL));
	return segmentcnt;
}

