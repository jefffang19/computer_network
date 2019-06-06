#include "server.h"
#define START_SEQNUM_S 393

void Server_process::inithandshake(){
	bool endhs = false;
	this->seqNum = START_SEQNUM_S;
	
	while(!endhs){
		Packet recv_packet = myRecv();
		
		switch(recv_packet.packet_type()){
			//get syn from client
			case packetType::packet_syn:
				if(isNewAck(recv_packet)){
					updateNum(recv_packet);
					//send synack to client
					mySend(Packet(packetType::packet_synack, *this, NULL));
				}
				break;
			//get ack from client
			case packetType::packet_ack:
				if(isNewAck(recv_packet)){
					updateNum(recv_packet);
					endhs = true; //complete 3way handshake
				}
				break;
		}
	}
	cout << "=====Complete the three-way handshake=====" << endl;
}
