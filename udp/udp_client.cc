#include"udp_client.hpp"
#include<memory>

int main(int argc,char*argv[])
{
    if(argc!=3)
    {
        std::cout<<"\r\n./UdpClient ip port"<<std::endl;
        return -1;
    }
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);
    std::unique_ptr<UdpClient> ptr(new UdpClient(serverip,serverport));
    ptr->Init();
    ptr->Start();
    return 0;
}