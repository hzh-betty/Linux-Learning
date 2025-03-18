#include "tcp_server.hpp"
#include "calculator.hpp"

void Handler(std::shared_ptr<Connection> connection)
{
    // 1. 去掉报头与反序列化
    std::string content;
    std::string &str = connection->getInbuffer();
    bool ret = Decode(&str, &content);
    Request req;
    req.Deserialize(content);

    // 2. 计算结果
    Response resq = Calculator::calculator(req);

    // 3. 序列化与封装报头
    std::string result;
    resq.Serialize(&result);
    result = Encode(result);

    connection->AppendOutBuffer(result);

    connection->sendCB_(connection);
}
int main(int argc, char *argv[])
{
    if (argc != 2)
        return -1;

    uint16_t port = std::stoi(argv[1]);
    std::unique_ptr<TcpServer> ptr(new TcpServer(port, Handler));
    ptr->Init();
    ptr->Start();
    return 0;
}