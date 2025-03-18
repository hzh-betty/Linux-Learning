#pragma once
#include "sock.hpp"
#include "threadpool.hpp"
#include "protocol.hpp"
#include <functional>
#include <memory>
class TcpServer
{
    using func_t = std::function<Response(Request &)>;

public:
    TcpServer(uint16_t port, func_t callback)
        : listensock_(new Sock), port_(port), callback_(callback)
    {
    }

    // 初始化服务端
    void Init()
    {
        listensock_->Socket(SOCK_STREAM);
        listensock_->Bind(port_);
        listensock_->Listen();
    }

    // 启动服务端
    void Run()
    {
        running_ = true;
        while (running_)
        {
            std::string clientip;
            uint16_t clientport;
            int newfd = listensock_->Accept(&clientip, &clientport);
            if (newfd == -1)
                continue;
            std::cout << "accept a new link,sockfd: " << newfd
                      << " clientip: " << clientip << " clientport: " << clientport << std::endl;
            ThreadPool::getInstance()->start(5);
            ThreadPool::getInstance()->submitTask([this, newfd]()
                                                  { Service(newfd); });
        }
    }
    ~TcpServer()
    {
        if (listensock_->Fd())
        {
            listensock_->Close();
        }
    }

private:
    // 提供服务
    void Service(int sockfd)
    {
        std::string package;
        while (running_)
        {
            // len"\n"10 + 20"\n
            //  1. 读取客户端数据
            char buffer[1024];
            ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                package += buffer;

                // 2.去掉报头与反序列化
                std::string content;
                bool ret = Decode(&package, &content);
                if (!ret)
                    continue; // 并没有得到一个完整的报文
                Request req;
                req.Deserialize(content);

                // 3.计算
                Response resq = callback_(req);

                // 4. 序列化与封装报头并发送数据
                std::string result;
                resq.Serialize(&result);
                result = Encode(result);
                write(sockfd, result.c_str(), result.size());
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
    std::unique_ptr<Sock> listensock_; // 套接字
    uint16_t port_;                    // 端口号
    func_t callback_;                  // 回调函数
    bool running_;                     // 是否启动
};