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
    SOCKET_ERR,
    BIND_ERR,
    LISTEN_ERR,
};
static constexpr int backlog = 10;

class Sock
{
public:
    Sock() = default;
    ~Sock() = default;

public:
    void Socket()
    {
        // 1. 创建套接字
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd_ < 0)
        {
            std::cerr << "socket error" << std::endl;
            exit(SOCKET_ERR);
        }

        // 2. 开启端口复用
        int opt = 1;
        setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }

    void Bind(uint16_t port)
    {
        // 1.填写绑定信息
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = INADDR_ANY;

        // 2. 绑定套接字
        if (bind(sockfd_, (const sockaddr *)&local, sizeof(local)) < 0)
        {
            std::cerr << "bind error" << std::endl;
            exit(BIND_ERR);
        }
    }

    void Listen()
    {
        // 1.监听套接字
        if (listen(sockfd_, backlog) < 0)
        {
            std::cerr << "listen error" << std::endl;
            exit(LISTEN_ERR);
        }
    }

    int Accept(std::string *clientip, uint16_t *clientport)
    {
        // 1. 获取对端网络信息
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        memset(&client, 0, sizeof(client));
        int newfd = accept(sockfd_, (sockaddr *)&client, &len);
        if (newfd < 0)
        {
            std::cerr << "accept error" << std::endl;
            return -1;
        }
        char ipstr[64] = {0};
        inet_ntop(AF_INET, &client.sin_addr, ipstr, sizeof(ipstr));
        *clientip = ipstr;
        *clientport = ntohs(client.sin_port);

        // 2.返回套接字
        return newfd;
    }

    bool Connect(const std::string &ip, const uint16_t &port)
    {
        // 1.填写服务端信息
        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(ip.c_str());

        // 2. 链接服务端
        int n = connect(sockfd_, (const sockaddr *)&server, sizeof(server));
        if (n == -1)
        {
            std::cerr << "connect to" << ip << ":" << port << "error" << std::endl;
            return false;
        }

        return true;
    }

    int Fd() const
    {
        return sockfd_;
    }

    void Close() const
    {
        close(sockfd_);
    }

private:
    int sockfd_;
};