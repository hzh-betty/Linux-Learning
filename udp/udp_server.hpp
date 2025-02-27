#include <iostream>
#include <string>
#include <functional>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
static const std::string defaultIp = "0.0.0.0";
static const uint16_t defaultPort = 8080;
using func_t = std::function<std::string(const std::string &)>;
const int size = 100;
class UdpServer
{
public:
    UdpServer(std::string ip = defaultIp, uint16_t port = defaultPort)
        : sockfd_(-1), ip_(ip), port_(port), running_(false)
    {
    }
    void Init()
    {
        // 1. 创建套接字
        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0)
        {
            std::cerr << "socket error..." << std::endl;
            exit(1);
        }

        // 2. 绑定套接字
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port_);
        local.sin_addr.s_addr = inet_addr(ip_.c_str());
        if (bind(sockfd_, (const sockaddr *)&local, sizeof(local)) < 0)
        {
            std::cerr << "bind error" << std::endl;
            exit(2);
        }
    }

    void Start(func_t func)
    {
        running_ = true;
        while (running_)
        {
            // 1.接受服务端的数据
            char inbuffer[size];
            memset(inbuffer, 0, size);
            struct sockaddr_in clientInfo;
            socklen_t len = sizeof(clientInfo);
            ssize_t n = recvfrom(sockfd_, inbuffer, size - 1, 0, (struct sockaddr *)&clientInfo, &len);
            if (n < 0)
            {
                std::cerr << "recvfrom error" << std::endl;
                continue;
            }
            // 2.处理数据病回显给客户端
            inbuffer[n] = 0;
            std::cout << inbuffer << std::endl;
            std::string info = inbuffer;
            std::string echo_info = func(inbuffer);
            n = sendto(sockfd_, echo_info.c_str(), echo_info.size(), 0, (struct sockaddr *)&clientInfo, len);
            if (n < 0)
            {
                std::cerr << "sendto error" << std::endl;
                continue;
            }
        }
    }

    ~UdpServer()
    {
        if (sockfd_ >= 0)
        {
            close(sockfd_);
        }
    }

private:
    int sockfd_;     // udp套接字
    std::string ip_; // 目的ip地址

    uint16_t port_; // 端口号
    bool running_;  // 是否启动
};