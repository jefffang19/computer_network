#include "client.h"

vector<unsigned char> out;
int numbth_client;  //you can be 1~4
int num_of_files;
int request_file[5];


void wfile(int num);

int main(){
	srand(time(NULL));
	
	Client myclient;
	
	cout << "input # client : 1\n";
	numbth_client = 1;
	cout << "request how many files : 1\n";
	num_of_files = 1;
	cout << "request video file number : 2\n";
	for(int i = 0;i < num_of_files;++i) request_file[i] = 2;
	
	//set src ip and port
	myclient.child.myCreateSocket(client_ip,client_port[numbth_client-1]);
	myclient.initInfo();
	
	//init connection
	myclient.child.myConnect(server_ip,server_port);
	
	myclient.child.inithandshake(numbth_client, num_of_files, request_file);
	
	
	//get all the files request
	for(int i=0;i<num_of_files;++i){
		myclient.recvfile();
		wfile(i+1);
		out.clear();
	}
	
	return 0;
}

void wfile(int num){
	stringstream ss;
	ss << numbth_client;
	string n = ss.str();
	stringstream ss2;
	ss2 << num;
	string numf = ss2.str();
	ofstream outf( "client" + n + "-out" + numf + ".mp4", ios::binary);
	
	char temp[out.size()];
	for(int i=0;i<out.size();++i){
		temp[i] = out[i];
	}
	copy(temp,temp+out.size(),ostreambuf_iterator<char>(outf));
	outf.close();
	
	cout << "client" << n << "-out" << numf << ".mp4 created , size :" << out.size() << " bytes\n";
}

void Client::initInfo(){
	cout << "====================" << endl <<
			"RTT delay = "	<< child.RTT << "ms" << endl <<
			"MSS = " << child.MSS << " bytes" << endl <<
			"buffer size = "<< default_BUFFER_SIZE << " bytes" << endl <<
			"loss = "<< child.loss << " %" << endl <<
			"Client IP & port= " << child.addr(child.srcSocket) << endl <<
			"====================" << endl;
}
void Client::recvfile(){
	bool isEnd = false;
	int timeout;
	unsigned char tmparr[10000];
	int index=0;
	Packet recv_packet;
	struct Sack{
		int losscnt;
		int rang[4][2];
		Sack(){
			losscnt = 0;
		}
	}sack;
	
	tmparr.clear();
	
	while(!isEnd){
		cout << "Listening\n";
		
		recv_packet = child.myRecv(&timeout,false);
		
		//if timeout happened, send dup ack
		if(timeout==-1){
			//create sack info
			sack.rang[sack.losscnt][0] = child.ackNum;
			sack.rang[sack.losscnt][1] = child.ackNum;
			sack.losscnt++;
			Packet t(packet_ack,this->child,(char*)NULL);
			t.header.option[0] = 5;
			for(int i=0;i<sack.losscnt;++i) t.data[i] = sack.rang[i][0];
			
			for(int i = 0;i<child.MSS;++i) tmparr[index++]=0;
			
			child.mySend(t);
			continue;
		}
		
		//sack
		i(losscnt==3){
			for(int i = 0;i<child.MSS;++i) tmparr[
		
		//cout << "Receive file from " << child.addr(child.destSocket) << endl;
		if(recv_packet.packet_type() == packet_data) {
				// if new ack and inorder then recv
				if(child.isNewAck(recv_packet) && child.updateNum(recv_packet)){
					for(int i = 0;i<child.MSS;++i) fileBuffer[i] = recv_packet.data[i];
					for(int i = 0;i<child.MSS;++i) out.push_back(fileBuffer[i]);
				}
				//if this is server try to resend the lost ones
				else if(recv_packet.header.option[0]==5){
					for(int i=0;i<child.MSS;++i) tmparr[i] = recv_packet.data[i];
					recv_packet = child.myRecv(&timeout,true);
					for(int i=2048;i<2048+child.MSS;++i) tmparr[i] = recv_packet.data[i];
					recv_packet = child.myRecv(&timeout,true);
					for(int i=4096;i<4096+child.MSS;++i) tmparr[i] = recv_packet.data[i];
					//finally to output
					for(int i=0;i<index;++i) out.push_back(tmparr[i]);
					//ack and seq Num to rightest
					child.seqNum = child.bestseqNum;
					child.ackNum = child.bestseqNum;
				}
					
				// if out of order
				else{
					for(int i = 0;i<child.MSS;++i) tmparr[index++]=recv_packet.data[i];
				}
				child.mySend(Packet(packet_ack,this->child,(char*)NULL));
		}
		//when the ack packet of threeway handshake loss
		else if(recv_packet.packet_type() == packet_synack){
			cout << "Server did not complete threeway handshake\ntry to fix\n";
			child.mySend(Packet(packet_ack,this->child,(char*)NULL));
		}
		else if(recv_packet.packet_type() == packet_fin){
			cout << "Receive file complete\n";
			isEnd = true;
		}
	}
	cout << "got file len = " << out.size() << "bytes" << endl;
}
