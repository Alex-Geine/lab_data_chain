#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#include <zmq.hpp>

class ServerClient
{
private:
    zmq::message_t request;
    zmq::socket_t socket;
public: 
    // Init Server/Client via socet_name. Example: "tcp://*:5555"
    ServerClient(std::string socet_name, bool isServer);

    // Get data: wait and get data
    void GetData(std::string& data);

    // Send data
    void SendData(const std::string& data);
};