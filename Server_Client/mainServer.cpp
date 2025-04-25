#include "ServerClient.h"

int main()
{
    std::string port = "tcp://*:5555";
    std::cout << "Before before 1" << std::endl;
    ServerClient server(port, true);
    std::cout << "Before before 2" << std::endl;
    std::string in_data{"Hello"};
    std::cout << "Before before 3" << std::endl;
    std::string out_data{};

    std::cout << "Before before" << std::endl;
    for (;;)
    {
        std::cout << "Before get data" << std::endl;
        server.GetData(in_data);

        std::cout << "Server receive: " << in_data << std::endl;

        server.SendData(out_data);
    }

    return 0;
}