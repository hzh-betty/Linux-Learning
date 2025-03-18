#include"tcp_server.hpp"
#include<memory>
int main(int argc,char*argv[])
{
    if(argc != 2)
    {
        std::cout<<"./TcpSerer port"<<std::endl;
        exit(USAGE_ERROR);
    }

    uint16_t port = std::atoi(argv[1]);
    std::unique_ptr<TcpServer> ptr(new TcpServer("0.0.0.0",port));

    ptr->Init();
    ptr->Run();
    return 0;
}