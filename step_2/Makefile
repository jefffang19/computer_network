all:
	g++ -std=c++11 -pthread -o c client.cpp tcpconnect.cpp client_process.cpp
	g++ -std=c++11 -pthread -o s tcpconnect.cpp server.cpp server_process.cpp
clean:
	rm -f test_c test_s s c

