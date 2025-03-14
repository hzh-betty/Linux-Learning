#include "poll_server.hpp"
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "argc != 2" << std::endl;
        exit(-1);
    }

    uint16_t port = std::stoi(argv[1]);

    std::unique_ptr<PollServer> ptr(new PollServer(port));

    ptr->Init();
    ptr->Start();
    return 0;
}