#include "sock.hpp"
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <pthread.h>
const std::string wwwroot = "./wwwroot";
const std::string sep = "\r\n";
const std::string homepage = "index.html";
class HttpServer;

class ThreadData
{
public:
    ThreadData(int fd, HttpServer *s) : sockfd(fd), svr(s)
    {
    }

public:
    int sockfd;
    HttpServer *svr;
};

class HttpRequst
{
public:
    // 反序列化
    void Deserialize(std::string req)
    {
        while (true)
        {
            // 1. 寻找请求行与请求报头
            size_t pos = req.find(sep);

            // 2.遇见空行
            if (pos == std::string::npos)
                break;
            std::string temp = req.substr(0, pos);
            if (temp.empty())
                break;

            // 3,保存请求行与请求报头
            reqHeader_.push_back(temp);
            req.erase(0, pos + sep.size());
        }

        // 4.保存正文
        req.erase(0, sep.size());
        text_ = req;
    }

    // 解析请求报文
    void Parse()
    {
        // 1.提取报头信息
        std::stringstream ss(reqHeader_[0]);
        ss >> method_ >> url_ >> httpVersion_;

        // 2. 访问首页
        filePath_ = wwwroot;
        if (url_ == "/" || url_ == "index.html")
        {
            filePath_ += "/";
            filePath_ += homepage;
        }
        else
        {
            filePath_ += url_;
        }

        // 3.提取报文类型
        size_t pos = filePath_.rfind(".");
        if (pos == std::string::npos)
        {
            suffix_ = ".html";
        }
        else
        {
            suffix_ = filePath_.substr(pos);
        }
    }

    void DebugPrint()
    {
        for (auto &line : reqHeader_)
        {
            std::cout << "--------------------------------" << std::endl;
            std::cout << line << "\n";
        }
        std::cout << "--------------------------------" << std::endl;
        std::cout << std::endl;
        std::cout << "method: " << method_ << std::endl;
        std::cout << "url: " << url_ << std::endl;
        std::cout << "httpVersion: " << httpVersion_ << std::endl;
        std::cout << "filePath: " << filePath_ << std::endl;
        std::cout << text_ << std::endl;
    }

    std::vector<std::string> reqHeader_; // 报头
    std::string text_;                   // 正文
    std::string method_;                 // 请求方法
    std::string url_;
    std::string httpVersion_;
    std::string filePath_; // 访问文件路径
    std::string suffix_;   // 后缀
};

class HttpServer
{
public:
    HttpServer(const uint16_t &port)
        : port_(port)
    {
        contentType_.insert({".html", "text/html"});
        contentType_.insert({".png", "imag/png"});
    }

    bool Start()
    {
        // 1. 创建，绑定，监听套接字
        listensock_.Socket();
        listensock_.Bind(port_);
        listensock_.Listen();

        // 2. 获取链接
        while (true)
        {
            std::string clientip;
            uint16_t clientport;
            int sockfd = listensock_.Accept(&clientip, &clientport);
            if (sockfd < 0)
                continue;

            pthread_t tid;
            ThreadData *td = new ThreadData(sockfd, this);
            pthread_create(&tid, nullptr, ThreadRun, td);
        }
    }

private:
    // 多线程处理多个客户端请求
    static void *ThreadRun(void *args)
    {
        pthread_detach(pthread_self());
        ThreadData *td = reinterpret_cast<ThreadData *>(args);
        td->svr->HanderHttp(td->sockfd);
        delete td;
        return nullptr;
    }

    // 处理HTTP报文
    void HanderHttp(int sockfd)
    {
        // 1.接收报文
        char buffer[10240];
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        if (n > 0)
        {
            // 2. 处理报文
            buffer[n] = 0;
            std::cout << buffer << std::endl;
            HttpRequst req;
            req.Deserialize(buffer);
            req.Parse();
            req.DebugPrint();

            // 3. 返回响应报文
            bool ok = true;
            std::string text = ReadHtmlContent(req.filePath_);

            // 4. 读取失败
            if (text.empty())
            {
                ok = false;
                std::string errstr = wwwroot;
                errstr += "/";
                errstr += "err.html";
                text = ReadHtmlContent(errstr);
            }

            // 4. 构建响应行
            std::string response_line;
            if (ok)
                response_line = "HTTP/1.0 200 OK\r\n";
            else
                response_line = "HTTP/1.0 404 Not Found\r\n";

            // 5. 构建响应报头与空行
            std::string response_header = "Content-Length: ";
            response_header += std::to_string(text.size()); // Content-Length: 11
            response_header += sep;
            response_header += "Content-Type: ";
            response_header += SuffixToDesc(req.suffix_);
            response_header += sep;
            response_header += "Set-Cookie: name=betty&&passwd=12345";
            response_header += sep;
            std::string blank_line = sep;

            // 6. 构建响应报文
            std::string response = response_line;
            response += response_header;
            response += blank_line;
            response += text;

            // 7.发生给客户端
            send(sockfd, response.c_str(), response.size(), 0);
        }
    }
    std::string SuffixToDesc(std::string &suffix)
    {
        auto iter = contentType_.find(suffix);
        if (iter == contentType_.end())
            return contentType_[".html"];
        else
        {
            return contentType_[suffix];
        }
    }

    static std::string ReadHtmlContent(const std::string &htmlpath)
    {
        // 1. 打开html文件
        std::ifstream in(htmlpath, std::ios::binary);
        if (!in.is_open())
            return "";

        // 2. 获取文件长度
        in.seekg(0, std::ios_base::end);
        auto len = in.tellg();
        in.seekg(0, std::ios_base::beg);

        // 3. 返回数据
        std::string content;
        content.resize(len);
        in.read((char *)content.c_str(), content.size());
        in.close();
        return content;
    }

private:
    Sock listensock_;
    uint16_t port_;
    std::unordered_map<std::string, std::string> contentType_;
};