#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum ExitCode
{
    USAGE_ERROR = 1,
    SOCKET_ERROR,
};

class TcpClient
{
public:
    TcpClient(const std::string ip, const uint16_t &port)
        : sockfd_(-1), ip_(ip), port_(port), running_(false)
    {
    }
    /*初始化客户端*/
    void Init()
    {
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0)
        {
            std::cerr << "socket error" << std::endl;
            exit(SOCKET_ERROR);
        }
    }

    /*运行客户端*/
    void Run()
    {
        // 1. 与服务端建立链接
        running_ = true;
        struct sockaddr_in client;
        memset(&client, 0, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_port = htons(port_);
        socklen_t len = sizeof(client);
        int isconnection = 3;
        while (isconnection--)
        {
            if (connect(sockfd_, (const sockaddr *)&client, len) == 0)
            {
                // 2. 发送消息请求
                Request();
            }
            else
            {
                sleep(1);
                std::cout << "connect error" << std::endl;
            }
        }
    }

    ~TcpClient()
    {
        if (sockfd_ >= 0)
        {
            close(sockfd_);
        }
    }

private:
    void Request()
    {
        std::string message;
        char buffer[1024];
        while (running_)
        {
            std::cout << "Please Enter@ ";
            getline(std::cin, message);
            write(sockfd_, message.c_str(), message.size());

            ssize_t n = read(sockfd_, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                std::cout << buffer << std::endl;
            }
            else if (n == 0)
            {
                break;
            }
            else
            {
                std::cerr << "read error" << std::endl;
                break;
            }
        }
    }

private:
    int sockfd_;
    std::string ip_;
    uint16_t port_;
    bool running_;
};