#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "sock.hpp"

static constexpr size_t FD_NUM_MAX = (sizeof(fd_set) * 8);
static constexpr int defaulId = -1;
static constexpr uint16_t defaultPort = 8888;

class SelectServer
{
public:
    SelectServer(uint16_t port = defaultPort)
        : port_(port), listensock_(new Sock())
    {
        InitArry();
    }

    // 初始化服务端
    void Init()
    {
        // 1. 创建套接字
        listensock_->Socket();
        // 2. 绑定套接字
        listensock_->Bind(port_);
        // 3. 监听套接字
        listensock_->Listen();
    }

    void Start()
    {
        int fd = listensock_->Fd();
        rfd_array[0] = fd;
        while (true)
        {
            // 1. 初始化
            fd_set rfds;
            FD_ZERO(&rfds);
            fd_set wfds;
            FD_ZERO(&wfds);

            // 2. 填入需要关心的文件描述符
            int maxfd = rfd_array[0];
            for (size_t i = 0; i < FD_NUM_MAX; i++)
            {
                if (rfd_array[i] == defaulId)
                    continue;
                FD_SET(rfd_array[i], &rfds);
                if (maxfd < rfd_array[i])
                {
                    maxfd = rfd_array[i];
                }
            }

            for (size_t i = 0; i < FD_NUM_MAX; i++)
            {
                if (wfd_array[i] == defaulId)
                    continue;
                FD_SET(wfd_array[i], &wfds);
                if (maxfd < wfd_array[i])
                {
                    maxfd = wfd_array[i];
                }
            }

            // 3.进行select
            int n = select(maxfd + 1, &rfds, &wfds, nullptr, nullptr);
            switch (n)
            {
            case 0:
                std::cout << "time out" << std::endl;
                break;
            case -1:
                std::cerr << "select error" << std::endl;
                break;
            default:
                //std::cout << "selectserver get a link" << std::endl;
                Dispatcher(rfds, wfds);
                break;
            }
        }
    }
    ~SelectServer()
    {
        if (listensock_->Fd() > 0)
        {
            listensock_->Close();
        }
    }

private:
    void InitArry()
    {
        for (size_t i = 0; i < FD_NUM_MAX; i++)
        {
            rfd_array[i] = defaulId;
            wfd_array[i] = defaulId;
        }
    }

    // 进行事件派发
    void Dispatcher(fd_set rdfs, fd_set wdfs)
    {
        for (size_t i = 0; i < FD_NUM_MAX; i++)
        {
            int rfd = rfd_array[i];
            int wfd = wfd_array[i];
            if (rfd == defaulId && wfd == defaulId)
                continue;

            if (FD_ISSET(rfd, &rdfs))
            {
                if (rfd == listensock_->Fd())
                {
                    Accepter();
                }
                else
                {
                    Recver(rfd, i);
                }
            }
            else if (FD_ISSET(wfd, &wdfs))
            {
                Writer(wfd, i);
            }
        }
    }

    // 获取新连接放入rfd_array
    void Accepter()
    {
        std::string ip;
        uint16_t port;
        int newfd = listensock_->Accept(&ip, &port);
        if (newfd < 0)
            return;
        AddReadFd(newfd);
    }

    void Recver(int fd, size_t pos)
    {
        ssize_t n = read(fd, buffer_, sizeof(buffer_) - 1);
        if (n > 0)
        {
            buffer_[n] = 0;
            AddWriteFd(fd);
            std::cout << "get a message " << buffer_ ;
        }
        else if (n == 0)
        {
            close(fd);
            rfd_array[pos] = defaulId;
            std::cout << fd << " sock has close " << std::endl;
        }
        else
        {
            close(fd);
            rfd_array[pos] = defaulId;
            std::cout << "read error" << std::endl;
        }
    }

    void Writer(int fd, size_t pos)
    {
        std::string echo_str = "echo: ";
        echo_str += buffer_;
        ssize_t n = write(fd, echo_str.c_str(), echo_str.size());
        if(n > 0)
        {
            wfd_array[pos] = defaulId;
        }
        else if (n < 0)
        {
            close(fd);
            wfd_array[pos] = defaulId;
            std::cout << "write error" << std::endl;
        }
    }

    // 添加读描述符
    void AddReadFd(int newfd)
    {
        size_t pos = 0;
        for (; pos < FD_NUM_MAX; pos++)
        {
            if (rfd_array[pos] != defaulId)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (pos == FD_NUM_MAX)
        {
            std::cerr << "server is full, close newfd now!" << std::endl;
            close(newfd);
        }
        else
        {
            rfd_array[pos] = newfd;
        }
    }

    // 添加写描述符
    void AddWriteFd(int newfd)
    {
        size_t pos = 0;
        for (; pos < FD_NUM_MAX; pos++)
        {
            if (wfd_array[pos] != defaulId)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (pos == FD_NUM_MAX)
        {
            std::cerr << "server is full, close newfd now!" << std::endl;
            close(newfd);
        }
        else
        {
            wfd_array[pos] = newfd;
        }
    }

private:
    std::unique_ptr<Sock> listensock_;
    uint16_t port_;
    int rfd_array[FD_NUM_MAX]; // 可读的文件描述符
    int wfd_array[FD_NUM_MAX]; // 可读的文件描述符
    char buffer_[1024];
};