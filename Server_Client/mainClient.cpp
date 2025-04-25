#include "ServerClient.h"

int main()
{
    std::string port = "tcp://localhost:5555";
    ServerClient client(port, false);
    std::string in_data{"Hello"};
    std::string out_data{};

    std::cout << "Before before" << std::endl;
    for (;;)
    {
        std::cout << "Before sending" << std::endl;
        client.SendData(in_data);
        std::cout << "After sending" << std::endl;
        client.GetData(out_data);
        std::cout << "Clinent receive: " << out_data << std::endl;
    }

    return 0;
}