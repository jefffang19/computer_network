#ifndef TCPCONNECT_H
#define TCPCONNECT_H
#include "settings.h"

//tcp state
enum tcpstate{ tcp_none, tcp_slowstart, tcp_congestionavoid, tcp_fastrecover };
//packet type
enum packetType{ packet_data, packet_ack, packet_syn, packet_synack, packet_fin };

//declaration
class Packet;
class Tcpheader;
class Tcpconnect;

//definition
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
		Packet(packetType type, Tcpconnect tcp, const char* indata, int indataSize = 0);
		packetType packet_type();
};

class Tcpconnect {
	public:
		int MSS, RTT, THRESHOLD;
		int hostfd;
		struct sockaddr_in srcSocket, destSocket;
		int seqNum, ackNum, recv_wnd;
		
		void myCreateSocket(const char* srcip, int srcport);
		void myConnect(const char* destip, int destport);
		void mySend(Packet packet);
		Packet myRecv();
		int disconnet();
		string addr(const struct sockaddr_in socket);
		bool isNewAck(const Packet recv_packet);
		void updateNum(const Packet recv_packet);
		Tcpconnect(){
			MSS = default_MSS;
			RTT = default_RTT;
			THRESHOLD = default_THRESHOLD;
		}
};

#endif
