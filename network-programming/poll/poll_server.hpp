#include "sock.hpp"
#include <poll.h>
#include <unistd.h>
#include <memory>
#include <string>
static constexpr size_t EVENT_FD_NUM = 1024;
static constexpr int defaultId = -1;
static constexpr short nonEvent = 0;
class PollServer
{
public:
    PollServer(uint16_t port)
        : port_(port), listensock_(new Sock())
    {
        InitEvent();
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
        // 设置监听套接字到eventFds
        eventFds_[0].fd = listensock_->Fd();
        eventFds_[0].events = POLLIN;
        while (true)
        {
            int n = poll(eventFds_, EVENT_FD_NUM, -1);
            switch (n)
            {
            case 0:
                std::cout << "timeout" << std::endl;
                break;
            case -1:
                std::cout << "poll error" << std::endl;
                break;
            default:
                Dispatcher();
                break;
            }
        }
    }

    ~PollServer()
    {
        if (listensock_->Fd())
        {
            listensock_->Close();
        }
    }

private:
    void InitEvent()
    {
        for (size_t i = 0; i < EVENT_FD_NUM; i++)
        {
            eventFds_[i].fd = defaultId;
            eventFds_[i].events = nonEvent;
            eventFds_[i].revents = nonEvent;
        }
    }

    // 向eventFds进行注册
    void EnrolFd(size_t pos, int fd, short events)
    {
        eventFds_[pos].fd = fd;
        eventFds_[pos].events |= events;
        eventFds_[pos].revents = nonEvent;
    }

    void Dispatcher()
    {
        for (size_t i = 0; i < EVENT_FD_NUM; i++)
        {
            int fd = eventFds_[i].fd;
            if (fd == defaultId)
                continue;

            if (eventFds_[i].revents & POLLIN)
            {
                if (fd == listensock_->Fd())
                {
                    Accepter();
                }
                else
                {
                    Recver(fd, i);
                }
            }
            else if (eventFds_[i].revents & POLLOUT)
            {
                Writer(fd, i);
            }
        }
    }

    void Accepter()
    {
        // 1. 获取链接
        std::string ip;
        uint16_t port;
        int newfd = listensock_->Accept(&ip, &port);
        if (newfd < 0)
            return;

        // 2. 添加读事件
        size_t pos = 0;
        for (; pos < EVENT_FD_NUM; pos++)
        {
            int fd = eventFds_[pos].fd;
            if (fd != defaultId)
                continue;
            else
                break;
        }

        if (pos == EVENT_FD_NUM)
        {
            std::cerr << "server is full, close now !" << std::endl;
            close(newfd);
        }
        else
        {
            EnrolFd(pos, newfd, POLLIN);
        }
    }

    // 读事件就绪
    void Recver(int fd, size_t pos)
    {
        ssize_t n = read(fd, buffer_, sizeof(buffer_) - 1);
        if (n > 0)
        {
            buffer_[n] = 0;
            std::cout << "server get a message :" << buffer_;
            // 注册写事件
            EnrolFd(pos, fd, POLLOUT);
        }
        else if (n == 0)
        {
            ClearPoll(pos, fd);
            std::cout << "client quit!" << std::endl;
        }
        else
        {
            ClearPoll(pos, fd);
            std::cout << "read error!" << std::endl;
        }
    }

    // 写事件就绪
    void Writer(int fd, size_t pos)
    {
        std::string echo_str = "echo: ";
        echo_str += buffer_;
        ssize_t n = write(fd, echo_str.c_str(), echo_str.size());
        if (n > 0)
        {
            eventFds_[pos].events ^= POLLOUT;
            eventFds_[pos].revents = nonEvent;
        }
        else if (n < 0)
        {
            ClearPoll(pos, fd);
            std::cout << "write error" << std::endl;
        }
    }

    // 异常退出清理资源
    void ClearPoll(size_t pos, int fd)
    {
        close(fd);
        eventFds_[pos].fd = defaultId;
        eventFds_[pos].events = nonEvent;
        eventFds_[pos].revents = nonEvent;
    }

private:
    std::unique_ptr<Sock> listensock_;
    uint16_t port_;
    struct pollfd eventFds_[EVENT_FD_NUM];
    char buffer_[1024];
};