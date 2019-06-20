#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>

using namespace std;

const int default_RTT = 150;  //150ms
const int default_THRESHOLD = 65535;  //64K
const int default_MSS = 1024; //1K
const int default_header_size = 20; //20 bytes
const int default_BUFFER_SIZE = 32768; //32K
const int num_of_clients = 200;

const char server_ip[] = "127.0.0.1";
const int server_port = 5001; 
const char client_ip[] = "127.0.0.1";
const int client_port[] = {1264, 3245, 9546, 7456}; 
#endif

