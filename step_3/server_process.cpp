#include "server.h"

void Server_process::inithandshake(){
	bool endhs = false, first = true;
	bool timeout = false;
	req_files.clear();
	
	cout << "Listening to client\n";
	while(!endhs){
		Packet recv_packet;
		
		//wait for new client, don't need to timeout
		if(first){
			recv_packet = myRecv();
			first = false;
		}
		
		else{
			timeout = false;
			//use timeoutable recv() to wait for ack after sent synack
			//timeout 5 sec
			try{ recv_packet = timeout_recv(5,*this); }
			catch(runtime_error &e){
				cout << e.what() << endl;
				timeout = true;
			}
		}
		
		
		if(timeout) mySend(Packet(packetType::packet_synack, *this, (char*)NULL)); //resend SYNACK
		else{
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
						for(int i=0;i<recv_packet.data[0];++i) req_files.push_back(recv_packet.data[i+1] - '0');

						endhs = true; //complete 3way handshake
					}
					break;
			}
		}
	}
	cout << "=====Complete the three-way handshake=====" << endl;

	//parse the ip:port format
	string unparse_str = addr(this->destSocket);
	int iplen = 0;
	for(int i=0;i<unparse_str.size();++i){
		if(unparse_str[i]!=':') ++iplen;
		else break;
	}
	char ip[iplen+1];
	int port = 0;	
	for(int i=0;i<iplen;++i) ip[i] = unparse_str[i];
		ip[iplen] = '\0';
	for(int i=iplen+1;i<unparse_str.size();++i)
		port = port * 10 + (unparse_str[i] - '0');
	
	masterchild = false;//this will become important to determine who is parent later in server.cpp
	if(childcnt>4){ cout << "already has 4 clients!"; return; } 
	//now fork a child
	pid_t pid;
	int status;
	pid = fork();
	switch(pid){
		//child
		case 0:
			cout << "As a child, I am connected	to: " << addr(destSocket) << endl;
			//remember to differ your child's port
			myCreateSocket(server_ip,server_port+childcnt+1);
			break;
		//parent
		default:
			childpid[childcnt] = pid;
			++childcnt;
			//waitpid(pid,&status,WNOHANG);
			masterchild = true;
			cout << "As a parent, I don't send file!\n";
			break;
	}
}

