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
	while(myServer.child.masterchild){
	 myServer.child.inithandshake();
	//if not master child, send video file
	}
	
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
	this->child.masterchild = true; //the child of server class is master child
	strcpy(fileBuffer,"hello world");
}

void Server::initInfo(){
	cout << "====================" << endl <<
			"RTT delay = "	<< child.RTT << "ms" << endl <<
			"MSS = " << child.MSS << " bytes" << endl <<
			"threshold = " << child.ssthresh << " bytes" << endl <<
			"buffer size = "<< default_BUFFER_SIZE << " bytes" << endl <<
			"loss = "<< child.loss << " %" << endl <<
			"Server IP & port= " << child.addr(child.srcSocket) << endl <<
			"====================" << endl;
}

void Server::printStatus(){
	cout << "====================" << endl << 
			"cwnd = " << child.cwnd << ", " << endl <<
			"recv_wnd = " << child.recv_wnd << ", " << endl <<
			"threshold = " << child.ssthresh << endl <<
			"====================" << endl;
}

int Server::sendfile(const char *data, const int dataSize){
	printStatus();
	int datastart = 0,
		datalen = dataSize,
		segmentcnt = 0, //count how much segement divided
		MSScnt = 1;
	child.doprintrcv=true;
	
	while(datalen > 0){
	
		char segmentdata[child.cwnd+5];  //divide data into segements of length MSS
		if(datalen < child.cwnd){
			for(int i=0;i<datalen;++i) segmentdata[i] = data[i+datastart];
		}	
		else{
			for(int i=0;i<child.cwnd;++i) segmentdata[i] = data[i+datastart];
		}
		
		MSScnt = child.cwnd/child.MSS;
		//cout << "debug: cwnd, MSS: " << child.cwnd << " " << child.MSS << endl;
		
		//transmit
		if(child.status==tcp_begin){
			child.printstatslowstart=1;
			child.status=tcp_slowstart;
		}
		child.printslowstart();
		
		printf("cwnd = %d MSS, rwnd = %d, threshold = %d\n",MSScnt,child.recv_wnd,child.ssthresh);
		for(int i=0;i<MSScnt;++i){
			char tmparr[child.cwnd];
			for(int j=0;j<child.cwnd;++j) tmparr[j]=segmentdata[j + child.MSS*i];
			Packet t(packetType::packet_data, this->child, tmparr, child.MSS);
			t.header.seqNum+=i;
			t.header.ackNum+=i;
			
			child.mySend(t,1);
		}
		cout << "\tsend: " << MSScnt << " MSS\n";
		
		//acks
		for(int i=0 ;i<MSScnt;++i){
			
			//you don't need to recv ack on odd times, but recv when last
			if(i%2 || MSScnt==1){
				Packet recv_packet;
				
				//use timeoutable recv()
				//timeout 5 sec
				bool timeout = false;
				child.isTimeout = false;
				try{ recv_packet = child.timeout_recv(5, this->child); }
				catch(runtime_error &e){
					cout << e.what() << endl;
					timeout = true;
					child.isTimeout = true;
				}
				
				if(child.isNewAck(recv_packet)) child.updateNum();
				
			}
			child.slowstart(i%2 == 0);
			
			
			datalen = datalen - child.MSS;
			datastart += child.MSS;
			++segmentcnt;
			
			child.seqNum++;
			child.ackNum++;

		}
		
	}
	child.mySend(Packet(packetType::packet_fin, this->child, NULL));
	exit(1);
	return segmentcnt;
}

