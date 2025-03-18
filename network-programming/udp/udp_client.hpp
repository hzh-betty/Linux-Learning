#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
class UdpClient
{
public:
    UdpClient(std::string &ip, uint16_t port)
        : sockfd_(-1), ip_(ip), port_(port), running_(false)
    {
    }

    void Init()
    {
        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ < 0)
        {
            std::cerr << "socket error" << std::endl;
        }
    }

    void Start()
    {
        running_ = true;
        while (running_)
        {
            // 1. 填入服务端信息
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(port_);
            server.sin_addr.s_addr = inet_addr(ip_.c_str());

            // 2. 向服务端写入信息
            socklen_t len = sizeof(server);
            std::string message;
            std::cout << "Please Enter# ";
            getline(std::cin, message);
            ssize_t n = sendto(sockfd_, message.c_str(), message.size(), 0, (struct sockaddr *)&server, len);
            if (n < 0)
            {
                std::cerr << "sendto error" << std::endl;
            }

            // 3. 接收服务端的信息
            char echo_info[1024];
            memset(echo_info, 0, sizeof(echo_info));
            n = recvfrom(sockfd_, echo_info, 1023, 0, (struct sockaddr *)&server, &len);
            if (n < 0)
            {
                std::cerr << "recvfrom error" << std::endl;
            }
            echo_info[n] = 0;
            std::cout << echo_info << std::endl;
        }
    }
    ~UdpClient()
    {
        if (sockfd_ >= 0)
        {
            close(sockfd_);
        }
    }

private:
    int sockfd_;     // 通信套接字
    std::string ip_; // 服务端IP
    uint16_t port_;  // 服务端端口号
    bool running_;   // 是否运行
};
