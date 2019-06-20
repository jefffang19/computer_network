#include "client.h"

vector<unsigned char> out; 
int num_of_files;
int request_file[5];
int numbth_client;

void wfile(int num);

int main(int argv, char* argc[]){
	srand(time(NULL));
	
	Client myclient;
	
	
	if(argv<3){
		fprintf(stderr,"please run my script with  source ./run200client.sh\n");
		return -1;
	}
	
	istringstream ss(argc[1]);
	
	ss >> numbth_client;
	cout << "input # client : " << numbth_client << endl;
	
	num_of_files = argc[2][0]-'0';
	cout << "request how many files : " << num_of_files << endl;
	
	cout << "request video file number : ";
	for(int i = 0;i < num_of_files;++i){
		request_file[i] = argc[3+i][0]-'0';
		cout << request_file[i] << endl;
	}
	
	
	
	//set src ip and port
	myclient.child.myCreateSocket(client_ip, client_port[numbth_client-1]);
	myclient.initInfo();
	
	//init connection
	myclient.child.myConnect(server_ip,server_port);
	
	myclient.child.inithandshake(numbth_client, num_of_files, request_file);
	
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
