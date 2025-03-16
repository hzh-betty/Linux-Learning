#pragma once
#include <iostream>
#include <string>
static const std::string blank_space_sep = " ";
static const std::string protocol_sep = "\n";
// "len""\n""x op y""\n"XXXX

// 添加报头
std::string Encode(const std::string &content)
{
    std::string package = std::to_string(content.size());
    package += protocol_sep;
    package += content;
    package += protocol_sep;
    return package;
}

// 去掉报头  "len"\n"x op y"\n
bool Decode(std::string *package, std::string *content)
{
    size_t pos = (*package).find(protocol_sep);
    if(pos == std::string::npos)
    {
        std::cerr<<"未找到分割字符"<<std::endl;
        return false;
    }
    std::string len_str = (*package).substr(0,pos);
    size_t len = std::stoi(len_str);
    size_t total_len = len_str.size() + len + 2;
    if(total_len > (*package).size())
    {
        std::cerr<<"未获取一个完整的报文"<<std::endl;
        return false;
    }
    *content = (*package).substr(pos + 1,len);
    (*package).erase(0,total_len);
    return true;
}
class Request
{
public:
    Request() = default;
    Request(int x,char op,int y)
    :x_(x),op_(op),y_(y)
    {}

    // 序列化
    bool Serialize(std::string*out)
    {
        std::string s = std::to_string(x_);
        s+=blank_space_sep;
        s+=op_;
        s+=blank_space_sep;
        s+=std::to_string(y_);
        *out = s;
        return true;
    }

    //反序列化
    bool Deserialize(const std::string&in)
    {
         std::size_t left = in.find(blank_space_sep);
         if (left == std::string::npos)
             return false;
         std::string part_x = in.substr(0, left);
         std::size_t right = in.rfind(blank_space_sep);
         if (right == std::string::npos)
             return false;
         std::string part_y = in.substr(right + 1);
         if (left + 2 != right)
             return false;
         op_ = in[left + 1];
         x_ = std::stoi(part_x);
         y_ = std::stoi(part_y);
         return true;
    }

    void DebugPrint()
    {
        std::cout << "新请求构建完成：" << x_ << op_ << y_ << "= ?" << std::endl;
    }
public:
    int x_;
    char op_;
    int y_;
};

class Response
{
public:
    Response() = default;
    Response(int result, int code)
        : result_(result), code_(code)
    {
    }
    bool Serialize(std::string *out)
    {
        // 序列化"result code"
        std::string s = std::to_string(result_);
        s += blank_space_sep;
        s += std::to_string(code_);
        *out = s;
        return true;
    }
    bool Deserialize(const std::string &in)
    {
        //反序列化
        size_t pos = in.find(blank_space_sep);
        if (pos == std::string::npos)
        {
            return false;
        }
        std::string part_left = in.substr(0, pos);
        std::string part_right = in.substr(pos + 1);
        result_ = std::stoi(part_left);
        code_ = std::stoi(part_right);
        return true;
    }
    void DebugPrint()
    {
        std::cout << "结果响应完成, result: " << result_ << ", code: " << code_ << std::endl;
    }

public:
    int result_;
    int code_;
};