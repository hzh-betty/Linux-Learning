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
    USAGE_ERR,
    SOCK_ERR,
    BIND_ERR,
    LISTN_ERR,
};

static constexpr int backlog = 10;

class Sock
{
public:
    // 创建套接字
    void Socket(int type)
    {
        sockfd_ = socket(AF_INET, type, 0);
        if (sockfd_ < 0)
        {
            std::cerr << "socket error" << std::endl;
            exit(SOCK_ERR);
        }
        if(type == SOCK_STREAM)
        {
            int opt = 1; // 防止偶发性的服务器无法进行，立即重启
            setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        }
    }

    // 绑定套接字
    void Bind(uint16_t port)
    {
        // 1. 添加本地网络信息
        struct sockaddr_in local;
        socklen_t len = sizeof(local);
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = INADDR_ANY;

        // 2. 绑定
        if (bind(sockfd_, (const sockaddr *)&local, len) < 0)
        {
            std::cerr << "bind error" << std::endl;
            exit(BIND_ERR);
        }
    }

    // 监听套接字
    void Listen()
    {
        if (listen(sockfd_, backlog) < 0)
        {
            std::cerr << "listen error" << std::endl;
            exit(LISTN_ERR);
        }
    }

    // 接收套接字
    int Accept(std::string *ip, uint16_t *port)
    {
        // 1. 获取链接
        struct sockaddr_in client;
        memset(&client, 0, sizeof(client));
        socklen_t len = sizeof(client);
        int newfd = accept(sockfd_, (sockaddr *)&client, &len);
        if (newfd < 0)
        {
            std::cerr << "accept error" << std::endl;
            return -1;
        }

        // 2.填写对端网络信息
        char ipstr[40];
        inet_ntop(AF_INET, &client.sin_addr, ipstr, sizeof(ipstr));
        *ip = ipstr;
        *port = ntohs(client.sin_port);
        return newfd;
    }

    // 链接服务端
    bool Connect(const std::string &ip, const uint16_t &port)
    {
        // 1. 填写服务端信息
        struct sockaddr_in server;
        memset(&server, 0, sizeof(server));
        socklen_t len = sizeof(server);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = inet_addr(ip.c_str());

        // 2. 链接服务端
        if(connect(sockfd_,(const sockaddr*)&server,len) < 0)
        {
            std::cerr<<"connect error"<<std::endl;
            return false;    
        }

        return true;
    }

    int Fd()
    {
        return sockfd_;
    }
    
    void Close()
    {
        close(sockfd_);
    }

private:
    int sockfd_;
};