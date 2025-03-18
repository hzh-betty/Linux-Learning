#include "httpserver.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        exit(1);
    }
    uint16_t port = std::stoi(argv[1]);
    std::unique_ptr<HttpServer> svr(new HttpServer(port));
    svr->Start();
    return 0;
}
