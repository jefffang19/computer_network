#include "client.h"


void Client_process::inithandshake(int numth_client, int num_of_files, int request_file[]){
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
		int timeout = 0;
		recv_packet = myRecv(&timeout);
		if(timeout==-1){
			mySend(Packet(packetType::packet_syn, *this, (char*)NULL)); //resend SYN
		}
		
		//got syn-ack from server and is not dup ack
		else if(recv_packet.packet_type() == packetType::packet_synack  && isNewAck(recv_packet)){
			updateNum(recv_packet);
			Packet a(packetType::packet_ack, *this, (char*)NULL);
			//in data part, tell 'how much file i want' '#video file i want' ...
			a.data[0] = '0' + num_of_files;
			for(int i = 1;i <= num_of_files;++i) a.data[i] = '0' + request_file[i-1];
			mySend(a);
			break;
		}
	}
	cout << "=====Complete the three-way handshake=====" << endl;
	//here you need to switch to new port
	myConnect(server_ip,server_port+numth_client);
}

