#include"epoll_server.hpp"
int main(int argc,char*argv[])
{
    if(argc != 2)
    {
        exit(-1);
    }
    uint16_t port = std::stoi(argv[1]);
    std::unique_ptr<EpollServer> ptr(new EpollServer(port));
    ptr->Init();
    ptr->Start();
    return 0;
}