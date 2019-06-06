#include "packet.h"

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

Packet::Packet(packetType type, Tcpconnect tcp, const char* data){
	this->header.srcPort = tcp.srcSocket.sin_port;
	this->header.destPort = tcp.destSocket.sin_port;
	this->header.seqNum = tcp.seqNum;
	this->header.ackNum = tcp.ackNum;
	this->header.recv_wnd = tcp.recv_wnd;
	switch(type){
		case packet_data:
			if(data==NULL){
				fprintf(stderr,"data str is null\n");
				break;
			}
			int dataSize;
			if(strlen(data) <= tcp.MSS) dataSize = strlen(data);
			else dataSize = tcp.MSS;
			strncpy(this->data, data, dataSize);
			this->header.checksum = dataSize;
			break;
		case packet_ack:
			this->header.ACK = true;
			this->header.SYN = false;
			this->header.FIN = false;
			this->header.checksum = 1;
			break;
		case packet_syn:
			this->header.ACK = false;
			this->header.SYN = true;
			this->header.FIN = false;
			this->header.checksum = 1;
			break;
		case packet_synack:
			this->header.ACK = true;
			this->header.SYN = true;
			this->header.FIN = false;
			this->header.checksum = 1;
			break;
		case packet_fin:
			this->header.ACK = false;
			this->header.SYN = false;
			this->header.FIN = true;
			this->header.checksum = 1;
			break;
	} 
}
