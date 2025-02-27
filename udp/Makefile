.PHONY: all
all: UdpServer UdpClient

UdpServer: udp_server.cc
	g++ -o $@ $^ -std=c++11

UdpClient: udp_client.cc
	g++ -o $@ $^ -std=c++11

.PHONY: clean
clean:
	rm -f UdpClient UdpServer