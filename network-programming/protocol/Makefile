.PHONEY:all
all:TcpServer TcpClient
TcpServer:tcp_server.cc
	g++ -o $@ $^ -std=c++11 -g -lpthread
TcpClient:tcp_client.cc
	g++ -o $@ $^ -std=c++11 -g
.PHONEY:clean
clean:
	rm -f TcpClient TcpServer