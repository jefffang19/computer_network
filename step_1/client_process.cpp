#include "client.h"


void Client_process::inithandshake(int numbth_client){
	cout << "=====start the three-way handshake=====" << endl;
	int START_SEQNUM = (rand() % 8888 + 1);
	
	this->seqNum = START_SEQNUM;
	//send syn packet to server
	mySend(Packet(packetType::packet_syn, *this, (char*)NULL));
	//now wait from syn-ack packet from server
	while(1){
		Packet recv_packet = myRecv();
		//got syn-ack from server and is not dup ack
		if(recv_packet.packet_type() == packetType::packet_synack  && isNewAck(recv_packet)){
			updateNum(recv_packet);
			mySend(Packet(packetType::packet_ack, *this, (char*)NULL));
			break;
		}
	}
	cout << "=====Complete the three-way handshake=====" << endl;
	//here you need to switch to new port
	myConnect(server_ip,server_port+numbth_client);
}

