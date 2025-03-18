#include "tcp_server.hpp"
#include "calculator.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "./TcpServer port" << std::endl;
        exit(USAGE_ERR);
    }

    uint16_t port = std::atoi(argv[1]);
    std::unique_ptr<TcpServer> ptr(new TcpServer(port, bind(&Calculator::calculator, std::placeholders::_1)));
    ptr->Init();
    ptr->Run();
    return 0;
}