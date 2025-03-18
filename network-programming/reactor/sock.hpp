#pragma once
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// 定义可能出现的错误码
enum
{
    SocketErr = 1,
    BindErr,
    ListenErr
};

// 定义最大连接数
const int backlog = 10;

class Sock
{
public:
    Sock() = default;
    ~Sock() = default;
public:
    // 创建套接字
    void Socket()
    {
        // 1. 使用IPv4协议族，流式套接字（TCP）创建套接字
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (_sockfd < 0)
        {
            std::cerr << "socket error..." << std::endl;
            exit(SocketErr);
        }
        int opt = 1;

        // 2. 设置套接字选项，允许地址重用
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    }

    // 绑定套接字到指定端口
    void Bind(uint16_t port)
    {
        struct sockaddr_in local;
        // 1. 初始化结构体
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;

        // 2. 将端口转换为网络字节序
        local.sin_port = htons(port);

        // 3. 绑定任意本地IP地址
        local.sin_addr.s_addr = INADDR_ANY;
        if (bind(_sockfd, (const struct sockaddr *)&local, sizeof(local)) < 0)
        {
            std::cerr << "bind error..." << std::endl;
            exit(BindErr);
        }
    }

    // 监听套接字
    void Listen()
    {
        if (listen(_sockfd, backlog) < 0)
        {
            std::cerr << "listen error..." << std::endl;
            exit(ListenErr);
        }
    }

    // 接受客户端连接
    int Accept(std::string *clientip, uint16_t *clientport)
    {
        // 1. 接受客户端连接，返回新的套接字描述符
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        int newfd = accept(_sockfd, (struct sockaddr *)&peer, &len);
        if (newfd < 0)
        {
           //std::cout << "accept error..." << std::endl;
            return -1;
        }
        char ipstr[64];

        // 2. 将网络字节序的IP地址转换为点分十进制字符串
        inet_ntop(AF_INET, &peer.sin_addr, ipstr, sizeof(ipstr));
        *clientip = ipstr;

        // 3. 将网络字节序的端口转换为主机字节序
        *clientport = ntohs(peer.sin_port);
        return newfd;
    }

    // 连接到指定IP和端口
    bool Connect(const std::string &ip, const uint16_t &port)
    {
        // 1. 将点分十进制IP字符串转换为网络字节序
        struct sockaddr_in peer;
        memset(&peer, 0, sizeof(peer));
        peer.sin_family = AF_INET;
        peer.sin_port = htons(port);

        inet_pton(AF_INET, ip.c_str(), &peer.sin_addr);
        int n = connect(_sockfd, (const struct sockaddr *)&peer, sizeof(peer));
        if (n == -1)
        {
            // 如果连接失败，输出错误信息并返回false
            std::cerr << "connect to " << ip << ":" << port << "error" << std::endl;
            return false;
        }
        return true;
    }

    // 关闭套接字
    void Close()
    {
        close(_sockfd);
    }

    // 获取套接字描述符
    int Fd()
    {
        return _sockfd;
    }

private:
    int _sockfd;
};