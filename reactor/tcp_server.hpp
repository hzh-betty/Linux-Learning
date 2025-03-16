#pragma once
#include "sock.hpp"
#include "epoller.hpp"
#include "comm.hpp"
#include "nocopy.hpp"
#include <memory>
#include <functional>
#include <unordered_map>

class Connection;
uint32_t EVENT_IN = EPOLLIN | EPOLLET;
uint32_t EVENT_OUT = EPOLLOUT | EPOLLET;
using func_t = std::function<void(std::shared_ptr<Connection>)>;
using except_func = std::function<void(std::shared_ptr<Connection>)>;

class Connection
{
public:
    Connection(int sock)
        : sockfd_(sock)
    {
    }

    void setHandler(func_t recvCB,
                    func_t sendCB,
                    except_func exceptCB)
    {
        recvCB_ = recvCB;
        sendCB_ = sendCB;
        exceptCB_ = exceptCB;
    }

    int SockFd() { return sockfd_; }

    void AppendInBuffer(const std::string &info)
    {
        inbuffer_ += info;
    }

    void AppendOutBuffer(const std::string &info)
    {
        outbuffer_ += info;
    }

    std::string &getInbuffer()
    {
        return inbuffer_;
    }
    std::string &getOutbuffer()
    {
        return outbuffer_;
    }

private:
    int sockfd_;
    std::string inbuffer_;  // 读缓冲区
    std::string outbuffer_; // 写缓冲区
public:
    func_t recvCB_;
    func_t sendCB_;
    except_func exceptCB_;
};

class TcpServer : public nocopy
{
    static constexpr size_t size = 128;

public:
    TcpServer(uint16_t port, func_t callback)
        : quit_(true), listensock_(new Sock()),
          epollfd_(new Epoller()), port_(port), callback_(callback)
    {
    }

    void Init()
    {
        // 1. 创建，绑定，监听套接字
        listensock_->Socket();
        listensock_->Bind(port_);
        listensock_->Listen();

        // 2. 添加监听事件
        AddConnection(listensock_->Fd(), EVENT_IN, std::bind(&TcpServer::Acepter, this, std::placeholders::_1), nullptr, nullptr);
    }

    void Start()
    {
        quit_ = false;
        struct epoll_event revents[size];
        while (!quit_)
        {
            int n = epollfd_->EpollWait(revents, size);
            switch (n)
            {
            case 0:
                std::cout << "time out" << std::endl;
                break;
            case -1:
                std::cerr << "wait error" << std::endl;
                break;
            default:
                Dispatcher(revents, n);
                break;
            }
        }

        quit_ = true;
    }

    ~TcpServer()
    {
        if (listensock_->Fd() >= 0)
        {
            listensock_->Close();
        }
    }

private:
    bool IsConnectionSafe(int fd)
    {
        auto iter = connections_.find(fd);
        if (iter == connections_.end())
            return false;
        else
            return true;
    }

    void Dispatcher(struct epoll_event *revents, int num)
    {
        for (int i = 0; i < num; i++)
        {
            uint32_t events = revents[i].events;
            int fd = revents[i].data.fd;

            // 统一把事件异常转换成为读写问题
            if (events & EPOLLERR)
                events |= (EPOLLIN | EPOLLOUT);
            if (events & EPOLLHUP)
                events |= (EPOLLIN | EPOLLOUT);

            if ((events & EPOLLIN) && IsConnectionSafe(fd))
            {
                if (connections_[fd]->recvCB_)
                    connections_[fd]->recvCB_(connections_[fd]);
            }

            if ((events & EPOLLOUT) && IsConnectionSafe(fd))
            {
                if (connections_[fd]->sendCB_)
                    connections_[fd]->sendCB_(connections_[fd]);
            }
        }
    }
    void AddConnection(int sockfd, uint32_t events, func_t recvCB,
                       func_t sendCB,
                       except_func exceptCB)
    {
        // 1. 设置为非阻塞
        setNonBlock(sockfd);

        // 2. 添加回调函数
        auto newCon = std::make_shared<Connection>(sockfd);
        newCon->setHandler(recvCB, sendCB, exceptCB);

        // 3. 添加进unordered_map
        connections_[sockfd] = newCon;

        // 4. 添加关心的事件
        epollfd_->EpollUpdate(EPOLL_CTL_ADD, sockfd, events);
    }

    void Acepter(std::shared_ptr<Connection> connection)
    {
        while (true)
        {
            std::string ip;
            uint16_t port;
            int newfd = listensock_->Accept(&ip, &port);
            if (newfd > 0)
            {
                AddConnection(newfd, EVENT_IN, std::bind(&TcpServer::Recver, this, std::placeholders::_1),
                              std::bind(&TcpServer::Sender, this, std::placeholders::_1),
                              std::bind(&TcpServer::Excepter, this, std::placeholders::_1));
            }
            else
            {
                // 被信号中断
                if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
    }

    void Recver(std::shared_ptr<Connection> connection)
    {
        while (true)
        {
            char buffer[1204];
            ssize_t n = read(connection->SockFd(), buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                connection->AppendInBuffer(buffer);
            }
            else if (n == 0)
            {
                connection->exceptCB_(connection);
                return;
            }
            else
            {
                // 读取数据完毕
                if (errno == EWOULDBLOCK)
                {
                    break;
                }
                // 被信号中断
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    // 异常
                    connection->exceptCB_(connection);
                    return;
                }
            }
        }
        callback_(connection);
    }

    void Sender(std::shared_ptr<Connection> connection)
    {
        std::string &outbuffer = connection->getOutbuffer();
        while (true)
        {
            ssize_t n = write(connection->SockFd(), outbuffer.c_str(), outbuffer.size());
            if (n > 0)
            {
                outbuffer.erase(0, n);
            }
            else if (n == 0)
            {
                break;
            }
            else
            {
                if (errno == EWOULDBLOCK)
                    break;
                else if (errno == EINTR)
                    continue;
                else
                {
                    connection->exceptCB_(connection);
                    return;
                }
            }
        }
        if (!outbuffer.empty())
        {
            EnalbeEvent(connection->SockFd(), true, true);
        }
        else
        {
            EnalbeEvent(connection->SockFd(), true, false);
        }
    }

    void Excepter(std::shared_ptr<Connection> connection)
    {
        // 1. 不再关心
        int sock = connection->SockFd();
        epollfd_->EpollUpdate(EPOLL_CTL_DEL, sock, 0);

        // 2. 关闭文件描述符
        close(sock);

        // 3. 删除
        connections_.erase(sock);
    }

    void EnalbeEvent(int sock, bool readalble, bool writealbe)
    {
        uint32_t events;
        events |= (readalble ? EPOLLIN : 0) | (readalble ? EPOLLOUT : 0) | EPOLLET;
        epollfd_->EpollUpdate(EPOLL_CTL_MOD, sock, events);
    }

private:
    bool quit_;
    std::unique_ptr<Sock> listensock_;
    std::unique_ptr<Epoller> epollfd_;
    std::unordered_map<int, std::shared_ptr<Connection>> connections_;
    func_t callback_;
    uint16_t port_;
};