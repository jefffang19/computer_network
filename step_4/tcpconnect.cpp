#include "tcpconnect.h"

//packet part
packetType Packet::packet_type(){
	if(header.ACK && header.SYN) return packet_synack;
	else if(header.ACK) return packet_ack;
	else if(header.SYN) return packet_syn;
	else if(header.FIN) return packet_fin;
	else packet_data;
}

Packet::Packet(){
	this->header.srcPort = 0;
	this->header.destPort = 0;
	this->header.seqNum = 0;
	this->header.ackNum = 0;
	this->header.ACK = false;
	this->header.SYN = false;
	this->header.FIN = false;
	this->header.recv_wnd = 0;
	this->header.checksum = 0;
	/*debug*/
	strcpy(this->data,"hello world!");
}

Packet::Packet(packetType type, Tcpconnect tcp, const char* indata, int indataSize){
	this->header.srcPort = tcp.srcSocket.sin_port;
	this->header.destPort = tcp.destSocket.sin_port;
	this->header.seqNum = tcp.seqNum;
	this->header.ackNum = tcp.ackNum;
	this->header.recv_wnd = tcp.recv_wnd;
	this->header.checksum = 1;
	switch(type){
		case packet_data:
			if(indata==NULL){
				fprintf(stderr,"data str is null\n");
				break;
			}
			/*int dataSize;
			if(strlen(data) <= tcp.MSS) dataSize = strlen(indata);
			else dataSize = tcp.MSS;*/
			for(int i=0;i<indataSize;++i) this->data[i] = indata[i];
			this->header.ACK = false;
			this->header.SYN = false;
			this->header.FIN = false;
			break;
		case packet_ack:
			this->header.ACK = true;
			this->header.SYN = false;
			this->header.FIN = false;
			strcpy(this->data,"ACK");
			break;
		case packet_syn:
			this->header.ACK = false;
			this->header.SYN = true;
			this->header.FIN = false;
			strcpy(this->data,"SYN");
			break;
		case packet_synack:
			this->header.ACK = true;
			this->header.SYN = true;
			this->header.FIN = false;
			strcpy(this->data,"SYNACK");
			break;
		case packet_fin:
			this->header.ACK = false;
			this->header.SYN = false;
			this->header.FIN = true;
			strcpy(this->data,"FIN");
			break;
	} 
}

//tcp network part


void Tcpconnect::myCreateSocket(const char* srcip, int srcport, int silence){
	// create socket failed
	if((hostfd = socket(AF_INET,SOCK_DGRAM,0)) == -1){
		if(!silence) fprintf(stderr,"socket create failed\n");
		return;
	}
	memset((char*)&srcSocket,0,sizeof(srcSocket));
	this->srcSocket.sin_family = AF_INET;
    this->srcSocket.sin_addr.s_addr = inet_addr(srcip);
    this->srcSocket.sin_port = srcport;
    if( (bind(hostfd, (struct sockaddr *)&srcSocket, sizeof(srcSocket) ) )== -1){
    	if(!silence) fprintf(stderr,"bind socket failed\n");
    	return;
    }
    if(!silence) printf("socket create successful\nSrcIP: %s\nSrcPort: %d\n",srcip,srcport);
}

void Tcpconnect::myConnect(const char* destip, int destport){
	memset((char*) &destSocket, 0, sizeof(destSocket) );
    this->destSocket.sin_family = AF_INET;
    this->destSocket.sin_addr.s_addr = inet_addr(destip);
    this->destSocket.sin_port = destport;
}

void Tcpconnect::printslowstart(){
	switch(this->printstatslowstart){
		case 1:
			cout << "*****Slow start*****\n";
		break;
		case 2:
			cout << "*****Congestion avoidance*****\n";
		break;
		case 3:
			cout << "*****Fast recover*****\n";
		break;
	}
	
	this->printstatslowstart = 0;
	return;
}

void Tcpconnect::slowstart(int delayack){
	if(delayack) isNewACK = true;
	//slowstart
	switch(this->status){
		case tcp_begin:
			this->status = tcp_slowstart;
			printstatslowstart=0;
			break;
		case tcp_slowstart:
			if(cwnd>=ssthresh){
				this->status = tcp_congestionavoid;
				printstatslowstart=2;
			}
			else if(dupACK==3){
				this->status = tcp_fastrecover;
				printstatslowstart=3;
				ssthresh = cwnd / 2;
				cwnd = ssthresh + 3 * MSS;
			}
			else if(isNewACK){
				cwnd += MSS;
				dupACK = 0;
			}
			else if(isDupACK) ++dupACK;
			else if(isTimeout){
				ssthresh = cwnd / 2;
				cwnd = MSS;
				dupACK = 0;
			}
		break;
		case tcp_congestionavoid:
			if(isTimeout){
				this->status = tcp_slowstart;
				printstatslowstart=1;
				ssthresh = cwnd / 2;
				cwnd = MSS;
				dupACK = 0;
			}
			else if(isDupACK) ++dupACK;
			else if(dupACK==3){
				this->status = tcp_fastrecover;
				printstatslowstart=3;
				ssthresh = cwnd / 2;
				cwnd = ssthresh + 3*MSS;
			}
			else if(isNewACK){
				cwnd = cwnd + MSS * (MSS * 1.0/ cwnd);
				dupACK = 0;
			}
		break;		
		case tcp_fastrecover:
			if(isTimeout){
				this->status = tcp_slowstart;
				printstatslowstart=1;
				ssthresh = cwnd / 2;
				cwnd = MSS;
				dupACK = 0;
			}
			else if(isNewACK){
				this->status = tcp_congestionavoid;
				printstatslowstart=2;
				cwnd = ssthresh;
				dupACK = 0;
			}
			else if(isDupACK) cwnd += MSS;
		break;
	}
	
	if(delayack) isNewACK = false;
	return;
}

void Tcpconnect::mySend(Packet packet, bool safemode){
	if(safemode) cout << "NO loss mode : on\n";
	
	map<packetType,string> pt;
	pt[packet_syn] = "SYN";
	pt[packet_ack] = "ACK";
	pt[packet_synack] = "SYNACK";
	pt[packet_fin] = "FIN";
	pt[packet_data] = "DATA";
	if(pt[packet.packet_type()] != "DATA") cout << "Send a packet(" << pt[packet.packet_type()] << ") to " << addr(destSocket) << endl;
	
	//if no loss
	if(true||safemode) sendto(hostfd, &packet, sizeof(Packet), 0, (struct sockaddr *) &destSocket, sizeof(destSocket) );
	//if loss
	else{
		cout << "unfortunately packet(" << pt[packet.packet_type()] << ") was lost during transmit." << endl <<
			"( seq num = " << packet.header.seqNum << ", " << "ack num = " << packet.header.ackNum << ")\n";
	}
}

void Tcpconnect::updateNum(const Packet recv_packet){
	this->seqNum = recv_packet.header.ackNum;
	this->ackNum = recv_packet.header.seqNum + 1;
}

bool Tcpconnect::isNewAck(const Packet recv_packet){
	if(recv_packet.header.ackNum > 0 && recv_packet.header.ackNum <= this->seqNum){
		isNewACK = false;
		isDupACK = true;
		return false;
	}
	else{
		isNewACK = true;
		isDupACK = false;
		return true;
	}
}



Packet Tcpconnect::myRecv(){
	Packet recv_packet;
	socklen_t packetSize = sizeof(destSocket);
	
	usleep( (this->RTT >> 1) * 1000);
	
	//UDP recv
	if(recvfrom(hostfd, &recv_packet, sizeof(Packet), 0, (struct sockaddr *) &destSocket, &packetSize)<0){
		perror("error recvfrom");
		exit(1);
	}
	map<packetType,string> pt;
	pt[packet_syn] = "SYN";
	pt[packet_ack] = "ACK";
	pt[packet_synack] = "SYNACK";
	pt[packet_fin] = "FIN";
	pt[packet_data] = "DATA";
	packetType recvpt = recv_packet.packet_type();
	if( recvpt == packet_syn) cout << "=====start three-way handshake=====" << endl;
	if(doprintrcv){
		cout << "Receive a packet(" << pt[recvpt] << ") from " << addr(destSocket) << endl;
		cout << "\treceive a packet ( seq num = " << recv_packet.header.seqNum << ", " << "ack num = " << recv_packet.header.ackNum << ")\n";
	}	
	return recv_packet;
}

Tcpconnect temptcp;
Packet timeout_recv_thread();
Packet timeout_recv_wrap(int timeout_sec, Tcpconnect tcp);

Packet Tcpconnect::timeout_recv(int timeout_sec, Tcpconnect tcp){
	return timeout_recv_wrap(timeout_sec, tcp);
}

Packet timeout_recv_wrap(int timeout_sec, Tcpconnect tcp){
	mutex m;
	condition_variable cv;
	Packet recv_packet;
	temptcp = tcp;
	thread t([&cv, &recv_packet](){
		recv_packet = timeout_recv_thread();
		cv.notify_one();
	});
	
	t.detach();
	
	{
		unique_lock<mutex> l(m);
		if(cv.wait_for(l, chrono::seconds(timeout_sec)) == cv_status::timeout)
			throw runtime_error("Timeout");
	}
	return recv_packet;
}

Packet timeout_recv_thread(){
	Packet recv_packet;
	recv_packet = temptcp.myRecv();
	return recv_packet;
}

bool Tcpconnect::packetLost(){
	//0.05 loss
	if(rand()%10000 < (loss*100)) return true;
	else return false;
}

int Tcpconnect::disconnet(){
	close(hostfd);
}

string Tcpconnect::addr(const struct sockaddr_in socket){
	char temp[100] = {'\0'};
	string ip = inet_ntop(AF_INET, &socket.sin_addr, temp, INET_ADDRSTRLEN);
	int port = socket.sin_port;
	return ip + ":" + to_string(port);
}
	
