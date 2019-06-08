#include "client.h"


void Client_process::inithandshake(int numbth_client){
	cout << "=====start the three-way handshake=====" << endl;
	int START_SEQNUM = (rand() % 10000 + 1);
	
	this->seqNum = START_SEQNUM;
	//send syn packet to server
	mySend(Packet(packetType::packet_syn, *this, (char*)NULL));
	//now wait from syn-ack packet from server
	while(1){
	
		Packet recv_packet;
		//use timeoutable recv()
		//timeout 5 sec
		bool timeout = false;
		try{ recv_packet = timeout_recv(5, *this); }
		catch(runtime_error &e){
			cout << e.what() << endl;
			timeout = true;
		}
		
		if(timeout) mySend(Packet(packetType::packet_syn, *this, (char*)NULL)); //resend SYN
		
		//got syn-ack from server and is not dup ack
		else if(recv_packet.packet_type() == packetType::packet_synack  && isNewAck(recv_packet)){
			updateNum(recv_packet);
			mySend(Packet(packetType::packet_ack, *this, (char*)NULL));
			break;
		}
	}
	cout << "=====Complete the three-way handshake=====" << endl;
	//here you need to switch to new port
	myConnect(server_ip,server_port+numbth_client);
}

