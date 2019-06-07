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

Packet::Packet(packetType type, Tcpconnect tcp, const char* indata){
	this->header.srcPort = tcp.srcSocket.sin_port;
	this->header.destPort = tcp.destSocket.sin_port;
	this->header.seqNum = tcp.seqNum;
	this->header.ackNum = tcp.ackNum;
	this->header.recv_wnd = tcp.recv_wnd;
	switch(type){
		case packet_data:
			if(indata==NULL){
				fprintf(stderr,"data str is null\n");
				break;
			}
			int dataSize;
			if(strlen(data) <= tcp.MSS) dataSize = strlen(data);
			else dataSize = tcp.MSS;
			strncpy(this->data, indata, dataSize);
			this->header.checksum = dataSize;
			this->header.ACK = false;
			this->header.SYN = false;
			this->header.FIN = false;
			break;
		case packet_ack:
			this->header.ACK = true;
			this->header.SYN = false;
			this->header.FIN = false;
			this->header.checksum = 1;
			strcpy(this->data,"ACK");
			break;
		case packet_syn:
			this->header.ACK = false;
			this->header.SYN = true;
			this->header.FIN = false;
			this->header.checksum = 1;
			strcpy(this->data,"SYN");
			break;
		case packet_synack:
			this->header.ACK = true;
			this->header.SYN = true;
			this->header.FIN = false;
			this->header.checksum = 1;
			strcpy(this->data,"SYNACK");
			break;
		case packet_fin:
			this->header.ACK = false;
			this->header.SYN = false;
			this->header.FIN = true;
			this->header.checksum = 1;
			strcpy(this->data,"FIN");
			break;
	} 
}

//tcp network part


void Tcpconnect::myCreateSocket(const char* srcip, int srcport){
	// create socket failed
	if((hostfd = socket(AF_INET,SOCK_DGRAM,0)) == -1){
		fprintf(stderr,"socket create failed\n");
		return;
	}
	memset((char*)&srcSocket,0,sizeof(srcSocket));
	this->srcSocket.sin_family = AF_INET;
    this->srcSocket.sin_addr.s_addr = inet_addr(srcip);
    this->srcSocket.sin_port = srcport;
    if( (bind(hostfd, (struct sockaddr *)&srcSocket, sizeof(srcSocket) ) )== -1){
    	fprintf(stderr,"bind socket failed\n");
    	return;
    }
    printf("socket create successful\nSrcIP: %s\nSrcPort: %d\n",srcip,srcport);
}

void Tcpconnect::myConnect(const char* destip, int destport){
	memset((char*) &destSocket, 0, sizeof(destSocket) );
    this->destSocket.sin_family = AF_INET;
    this->destSocket.sin_addr.s_addr = inet_addr(destip);
    this->destSocket.sin_port = destport;
    printf("Set destination socket\nDestIP: %s\nDestPort: %d\n",destip,destport);
}

void Tcpconnect::mySend(Packet packet){
	sendto(hostfd, &packet, sizeof(Packet), 0, (struct sockaddr *) &destSocket, sizeof(destSocket) );
	
	map<packetType,string> pt;
	pt[packet_syn] = "SYN";
	pt[packet_ack] = "ACK";
	pt[packet_synack] = "SYNACK";
	pt[packet_fin] = "FIN";
	pt[packet_data] = "DATA";
	cout << "Send a packet(" << pt[packet.packet_type()] << ") to " << addr(destSocket) << endl;
	
	//cout << "Packet sent\nData: " << packet.data << endl ;
}

void Tcpconnect::updateNum(const Packet recv_packet){
	this->seqNum = recv_packet.header.ackNum;
	this->ackNum = recv_packet.header.seqNum + recv_packet.header.checksum;
}

bool Tcpconnect::isNewAck(const Packet recv_packet){
	if(recv_packet.header.ackNum > 0 && recv_packet.header.ackNum <= this->seqNum) return false;
	else return true;
}



Packet Tcpconnect::myRecv(){
	Packet recv_packet;
	socklen_t packetSize = sizeof(destSocket);
	// wait half RTT time before recieve
	usleep( (this->RTT >> 1) * 1000);
	//UDP recv
	recvfrom(hostfd, &recv_packet, sizeof(Packet), 0, (struct sockaddr *) &destSocket, &packetSize);
	
	map<packetType,string> pt;
	pt[packet_syn] = "SYN";
	pt[packet_ack] = "ACK";
	pt[packet_synack] = "SYNACK";
	pt[packet_fin] = "FIN";
	pt[packet_data] = "DATA";
	packetType recvpt = recv_packet.packet_type();
	if( recvpt == packet_syn) cout << "=====start three-way handshake=====" << endl;
	cout << "Receive a packet(" << pt[recvpt] << ") from " << addr(destSocket) << endl;
	cout << "\treceive a packet ( seq num = " << recv_packet.header.seqNum << ", " << "ack num = " << recv_packet.header.ackNum << ")\n";
			
	return recv_packet;
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
	
