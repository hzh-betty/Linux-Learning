#include "tcp_client.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "./TcpClient ip port" << std::endl;
        exit(USAGE_ERR);
    }
    std::string ip = argv[1];
    uint16_t port = std::atoi(argv[2]);
    std::unique_ptr<TcpClient> ptr(new TcpClient(ip, port));
    ptr->Init();
    ptr->Run();
    return 0;
}