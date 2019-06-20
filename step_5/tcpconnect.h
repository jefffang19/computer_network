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
		int MSS, RTT, ssthresh;
		int hostfd;
		struct sockaddr_in srcSocket, destSocket;
		int seqNum, ackNum, recv_wnd, dupACK, cwnd;
		double loss; //in percentage
		int isloss;
		tcpstate status;
		//slow start status
		bool isTimeout, isNewACK, isDupACK;
		int printstatslowstart;
		bool doprintrcv;
		
		void myCreateSocket(const char* srcip, int srcport);
		void myConnect(const char* destip, int destport);
		void mySend(Packet packet, bool safemode = false);
		void slowstart();
		Packet myRecv(int *timeout, const bool isclient = false);
		int disconnet();
		string addr(const struct sockaddr_in socket);
		bool isNewAck(const Packet recv_packet);
		bool updateNum(const Packet recv_packet);
		bool packetLost();
		void printslowstart();
		Tcpconnect(){
			MSS = default_MSS;
			cwnd = MSS;
			recv_wnd = default_BUFFER_SIZE;
			RTT = default_RTT; 
			ssthresh = 8192; //smaller to let congestion control happen
			loss = 0.01;
			status = tcp_begin;
			dupACK = 0;
			isTimeout = false;
			isNewACK = false;
			isDupACK = false;
			doprintrcv = true;
			printstatslowstart = 0;
			isloss=0;
		}
};

#endif
