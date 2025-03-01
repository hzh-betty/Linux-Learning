#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "threadpool.hpp"
enum ExitCode
{
    USAGE_ERROR = 1,
    SOCKET_ERROR,
    BIND_ERROR,
    LISTEN_ERROR,
};

/*基于TCP的回声服务端*/
static const std::string defaultip = "0.0.0.0";
static constexpr uint16_t defaulport = 8080;
static constexpr int backlog = 10;

class TcpServer
{
public:
    TcpServer(const std::string &ip = defaultip, const uint16_t &port = defaulport)
        : listensock_(-1), ip_(ip), port_(port), running_(false)
    {
    }

    /*初始化服务端--获取，绑定，监听套接字*/
    void Init()
    {
        // 1. 创建套接字
        listensock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listensock_ < 0)
        {
            std::cerr << "socket create error " << std::endl;
            exit(SOCKET_ERROR);
        }

        // 2. 设置端口复用
        int opt = 1;
        setsockopt(listensock_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

        // 3. 绑定套接字
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port_);
        local.sin_addr.s_addr = inet_addr(ip_.c_str());
        socklen_t len = sizeof(local);
        if (bind(listensock_, (const sockaddr *)&local, len) < 0)
        {
            std::cerr << "bind error" << std::endl;
            exit(BIND_ERROR);
        }

        // 4. 监听套接字
        if (listen(listensock_, backlog) < 0)
        {
            std::cerr << "listen error" << std::endl;
            exit(LISTEN_ERROR);
        }
    }

    /* 启动服务端 */
    void Run()
    {
        running_ = true;
        while (running_)
        {
            // 1. 获取链接
            struct sockaddr_in client;
            memset(&client, 0, sizeof(client));
            socklen_t len = sizeof(client);
            int sockfd = accept(listensock_, (sockaddr *)&client, &len);
            if (sockfd < 0)
            {
                std::cerr << "accept error" << std::endl;
                continue;
            }
            uint16_t clientport = ntohs(client.sin_port);
            char clientip[32];
            inet_ntop(AF_INET, &(client.sin_addr), clientip, len);
            std::cout << "server get a link " << sockfd << " [" << clientip << "] " << "port " << clientport << std::endl;

            // 2. 多线程处理服务
            ThreadPool::getInstance()->start(5);
            auto task = std::bind(&TcpServer::Service, this, sockfd);
            ThreadPool::getInstance()->submitTask(task);
        }
    }
    ~TcpServer()
    {
        if (listensock_ >= 0)
        {
            close(listensock_);
        }
    }

private:
    void Service(int sockfd)
    {
        char buffer[1024];
        while (running_)
        {
            ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                std::cout << "client say# " << buffer << std::endl;
                std::string echo_str = "server echo@ ";
                echo_str += buffer;
                write(sockfd, echo_str.c_str(), echo_str.size());
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
        close(sockfd);
    }

private:
    int listensock_; // 监听套接字
    std::string ip_; // ip
    uint16_t port_;  // 端口
    bool running_;   // 是否启动
};