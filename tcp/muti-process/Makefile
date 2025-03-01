.PHONY:all
all:TcpClient TcpServer
TcpClient:tcp_client.cc
	g++ -o $@ $^ -std=c++11
TcpServer:tcp_server.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f TcpClient TcpServer