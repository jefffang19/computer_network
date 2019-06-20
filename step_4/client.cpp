#include "client.h"

vector<unsigned char> out;
int numbth_client;  //you can be 1~4
int num_of_files;
int request_file[5];


void wfile(int num);

int main(){
	srand(time(NULL));
	
	Client myclient;
	
	cout << "input # client ";
	numbth_client = 1;
	cout << "request how many files ";
	num_of_files = 1;
	cout << "request video file number ";
	for(int i = 0;i < num_of_files;++i) request_file[i] = 3;
	
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
	int isOddNum = 1;

	while(!isEnd){
		cout << "Listening\n";
		Packet recv_packet = child.myRecv();
		//cout << "Receive file from " << child.addr(child.destSocket) << endl;
		if(recv_packet.packet_type() == packet_data) {
				// if new ack then recv, else ignore
				if(child.isNewAck(recv_packet)){
					for(int i = 0;i<child.MSS;++i) fileBuffer[i] = recv_packet.data[i];
					for(int i = 0;i<child.MSS;++i) out.push_back(fileBuffer[i]);
					child.updateNum(recv_packet);
					++isOddNum;
				}
				if(isOddNum%2 == 0) child.mySend(Packet(packet_ack,this->child,(char*)NULL));
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
