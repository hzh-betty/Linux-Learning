#pragma once
#include "sock.hpp"
#include "protocol.hpp"
#include <memory>
#include <ctime>
#include <cassert>

class TcpClient
{
public:
    TcpClient(const std::string &ip, const uint16_t &port)
        : sockptr_(new Sock), ip_(ip), port_(port)
    {
        srand(time(nullptr) ^ getpid());
    }

    void Init()
    {
        sockptr_->Socket();
    }

    void Run()
    {
        if (sockptr_->Connect(ip_, port_))
        {
            Test();
        }
    }

    ~TcpClient()
    {
        if (sockptr_->Fd() >= 0)
        {
            sockptr_->Close();
        }
    }

private:
    void Test()
    {
        const std::string opers = "+-*/%&^";
        int cnt = 1;
        std::string inbuffer;
        while (cnt <= 10)
        {
            std::cout << "===============第" << cnt << "次测试 " << "===============" << std::endl;
            // 1. 获取操作数与操作符
            int x = rand() % 100 + 1;
            usleep(1234);
            int y = rand() % 100;
            usleep(4321);
            char oper = opers[rand() % opers.size()];
            Request req(x, oper, y);
            req.DebugPrint();

            // 2. 序列化与封装报头
            std::string package;
            req.Serialize(&package);
            package = Encode(package);

            // 3. 发送给服务端
            write(sockptr_->Fd(), package.c_str(), package.size());

            // 4. 接收服务端响应
            char buffer[128];
            ssize_t n = read(sockptr_->Fd(), buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                // "len"\n"result code"\n

                // 5. 去掉报头与反序列化
                buffer[n] = 0;
                inbuffer += buffer;
                std::string content;
                bool ret = Decode(&inbuffer, &content);
                if (!ret)
                    continue; // 未获取到完整报文
                Response resp;
                ret = resp.Deserialize(content);
                assert(ret);
                resp.DebugPrint();
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

            std::cout << "========================================" << std::endl;
            sleep(1);
            cnt++;
        }
    }

private:
    std::unique_ptr<Sock> sockptr_;
    std::string ip_;
    uint16_t port_;
};