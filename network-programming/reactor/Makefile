.PHONETY:all
all:tcp_client tcp_server
tcp_client:tcp_client.cc
	g++ -o $@ $^ -std=c++11
tcp_server:tcp_server.cc
	g++ -o $@ $^ -std=c++11
.PHONETY:clean
clean:
	rm -f tcp_client tcp_server
