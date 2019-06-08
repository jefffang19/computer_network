#include "client.h"

vector<unsigned char> out;
int numbth_client;  //you can be 1~4

void wfile();

int main(){
	srand(time(NULL));
	
	cout << "what number is this client? ";
	cin >> numbth_client;
	Client myclient;
	//set src ip and port
	myclient.child.myCreateSocket(client_ip,client_port[numbth_client-1]);
	myclient.initInfo();
	
	//init connection
	myclient.child.myConnect(server_ip,server_port);
	
	myclient.child.inithandshake(numbth_client);
	
	
	myclient.recvfile();
	
	wfile();
	
	return 0;
}

void wfile(){
	stringstream ss;
	ss << numbth_client;
	string n = ss.str();
	ofstream outf( n + "-out.mp4", ios::binary);
	char temp[out.size()];
	for(int i=0;i<out.size();++i){
		temp[i] = out[i];
	}
	copy(temp,temp+out.size(),ostreambuf_iterator<char>(outf));
	outf.close();
	cout << n << "-out.mp4 created , size :" << out.size() << " bytes\n"; 
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
				}
				child.mySend(Packet(packet_ack,this->child,(char*)NULL));
		}
		else if(recv_packet.packet_type() == packet_fin){
			cout << "Receive file complete\n";
			isEnd = true;
		}
	}
	cout << "got file len = " << out.size() << "bytes" << endl;
}