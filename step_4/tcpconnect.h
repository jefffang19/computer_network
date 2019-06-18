#ifndef TCPCONNECT_H
#define TCPCONNECT_H
#include "settings.h"

//tcp state
enum tcpstate{ tcp_begin, tcp_slowstart, tcp_congestionavoid, tcp_fastrecover };
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
		int MSS, RTT, sstresh;
		int hostfd;
		struct sockaddr_in srcSocket, destSocket;
		int seqNum, ackNum, recv_wnd, dupACK, cwnd;
		double loss; //in percentage
		tcpstate status;
		
		void myCreateSocket(const char* srcip, int srcport);
		void myConnect(const char* destip, int destport);
		void mySend(Packet packet, bool safemode = false);
		Packet myRecv();
		int disconnet();
		string addr(const struct sockaddr_in socket);
		bool isNewAck(const Packet recv_packet);
		void updateNum(const Packet recv_packet);
		bool packetLost();
		void slowstart(bool timeout, bool newACK);
		Packet timeout_recv(int timeout_sec, Tcpconnect tcp); //the version of recv with timeout
		Tcpconnect(){
			MSS = 1;
			cwnd = MSS;
			RTT = default_RTT;
			sstresh = default_THRESHOLD;
			loss = 0.01;
			status = tcp_begin;
			dupACK = 0;
		}
};

#endif
