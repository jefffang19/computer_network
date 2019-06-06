#ifndef PACKET_H
#define PACKET_H
#include "settings.h"

//foward declartion
class Tcpconnect;

//packet type
enum packetType{ packet_data, packet_ack, packet_syn, packet_synack, packet_fin };

class Tcpheader{
	public:
		short int srcPort, destPort;
		int seqNum, ackNum;
		bool ACK, SYN, FIN;
		short int recv_wnd, checksum;
};

class Packet {
	public:
		Tcpheader header;
		char data[default_MSS];
		Packet();
		Packet(packetType type, Tcpconnect tcp, const char* data);
		packetType packet_type();
};



#endif
	
