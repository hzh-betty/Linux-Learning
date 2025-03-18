#include "epoller.hpp"
#include "sock.hpp"
#include <memory>
#include <string>
class EpollServer
{
    static constexpr size_t size = 128;

public:
    EpollServer(const uint16_t &port)
        : port_(port), epollfd_(new Epoller()), listensock_(new Sock())
    {
    }

    void Init()
    {
        // 创建，绑定，监听
        listensock_->Socket();
        listensock_->Bind(port_);
        listensock_->Listen();
    }

    void Start()
    {
        // 将监听套接字加入epoll
        bool ret = epollfd_->EpollUpdate(EPOLL_CTL_ADD, listensock_->Fd(), EPOLLIN);
        struct epoll_event revents[size];
        while (true)
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
    }
    ~EpollServer()
    {
        if (listensock_->Fd() >= 0)
        {
            listensock_->Close();
        }
    }

private:
    // 事件派发
    void Dispatcher(struct epoll_event revents[], size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            uint32_t events = revents[i].events;
            int fd = revents[i].data.fd;

            // 读事件
            if (events & EPOLLIN)
            {
                if (fd == listensock_->Fd())
                {
                    Accepter();
                }
                else
                {
                    Recver(fd);
                }
            }
            // 写事件
            else if (events & EPOLLOUT)
            {
                Writer(fd);
            }
        }
    }
    void Accepter()
    {
        // 1. 获取新链接
        std::string ip;
        uint16_t port;
        int newfd = listensock_->Accept(&ip, &port);
        if (newfd < 0)
            return;

        // 2. 添加读事件
        epollfd_->EpollUpdate(EPOLL_CTL_ADD, newfd, EPOLLIN);
    }

    void Recver(int fd)
    {
        ssize_t n = read(fd, buffer_, sizeof(buffer_) - 1);
        if (n > 0)
        {
            buffer_[n] = 0;
            std::cout << "server get a message " << buffer_;
            epollfd_->EpollUpdate(EPOLL_CTL_MOD, fd, EPOLLOUT);
        }
        else if (n == 0)
        {
            std::cout << "client has quit" << std::endl;
            epollfd_->EpollUpdate(EPOLL_CTL_DEL, fd, 0);
            close(fd);
        }
        else
        {
            std::cerr << "read error" << std::endl;
            epollfd_->EpollUpdate(EPOLL_CTL_DEL, fd, 0);
            close(fd);
        }
    }

    void Writer(int fd)
    {
        std::string echo_str = "echo :";
        echo_str += buffer_;
        ssize_t n = write(fd, echo_str.c_str(), echo_str.size());
        if (n > 0)
        {
            epollfd_->EpollUpdate(EPOLL_CTL_MOD, fd, EPOLLIN);
        }
        else if (n < 0)
        {
            std::cerr << "write error" << std::endl;
            epollfd_->EpollUpdate(EPOLL_CTL_DEL, fd, 0);
            close(fd);
        }
    }

private:
    std::unique_ptr<Epoller> epollfd_;
    std::unique_ptr<Sock> listensock_;
    uint16_t port_;
    char buffer_[1024];
};
