#pragma once
#include <iostream>
#include "nocopy.hpp"
#include <unistd.h>
#include <sys/epoll.h>
class Epoller
{
    static constexpr size_t size = 128;

public:
    // 创建epoll模型
    Epoller(int timeout = -1)
        : timeout_(timeout)
    {
        epfd_ = epoll_create(size);
        if (epfd_ == -1)
        {
            std::cerr << "epoll_create error..." << std::endl;
            return;
        }
    }

    // 对epoll模型进行增加，删除，修改
    bool EpollUpdate(int oper, int sock, const uint32_t &events)
    {
        int n;
        // 1. 删除节点
        if (oper == EPOLL_CTL_DEL)
        {
            n = epoll_ctl(epfd_, oper, sock, nullptr);
        }
        // 2. 增加或者修改
        else
        {
            struct epoll_event ev;
            ev.events = events;
            ev.data.fd = sock;
            n = epoll_ctl(epfd_, oper, sock, &ev);
        }
        if (n != 0)
        {
            std::cerr << "epoll_ctl error..." << std::endl;
            return false;
        }
        return true;
    }

    // 获取对应的就绪节点
    int EpollWait(struct epoll_event revents[], int num)
    {
        int n = epoll_wait(epfd_, revents, num, timeout_);
        return n;
    }
    ~Epoller()
    {
        if (epfd_ >= 0)
        {
            close(epfd_);
        }
    }

private:
    int epfd_;
    int timeout_;
};