#include "udp_server.hpp"
#include <memory>

std::string handler(const std::string &info)
{
    std::string echo_info = "echo: ";
    echo_info += info;
    return echo_info;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "\t\n./UdpServer port " << std::endl;
        return -1;
    }
    uint16_t serverport = std::stoi(argv[1]);
    std::string serverip = "0.0.0.0";
    std::unique_ptr<UdpServer> ptr(new UdpServer(serverip, serverport));
    ptr->Init();
    ptr->Start(handler);
    return 0;
}