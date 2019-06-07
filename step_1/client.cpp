#include "client.h"


int main(){
	Client myclient;
	//set src ip and port
	myclient.child.myCreateSocket("127.0.0.1",5001);
	myclient.initInfo();
	
	//init connection
	myclient.child.myConnect("127.0.0.1",5002);
	
	myclient.child.inithandshake();
	
	myclient.recvfile();
	return 0;
}


void Client::initInfo(){
	cout << "====================" << endl <<
			"RTT delay = "	<< child.RTT << "ms" << endl <<
			"MSS = " << child.MSS << " bytes" << endl <<
			"buffer size = "<< default_BUFFER_SIZE << " bytes" << endl <<
			"Client IP & port= " << child.addr(child.srcSocket) << endl <<
			"====================" << endl;
}
void Client::recvfile(){
	bool isEnd = false;
	cout << "Listening\n";
	Packet recv_packet = child.myRecv();
	cout << "Receive file from " << child.addr(child.destSocket) << endl;
	if(recv_packet.packet_type() == packet_data) {
			// if new ack then recv, else ignore
			if(child.isNewAck(recv_packet)){
				strcpy(fileBuffer, recv_packet.data);
				child.updateNum(recv_packet);
			}
			child.mySend(Packet(packet_ack,this->child,(char*)NULL));
	}
	cout << "got file : " << endl << fileBuffer << endl;
}
