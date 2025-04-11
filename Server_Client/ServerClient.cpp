#include "ServerClient.h"

// Server

// Init Server/Client via socet_name. Example: "tcp://*:5555"
Server::Server(std::string socet_name)
{
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    socket = {context, zmq::socket_type::rep};

    socket.bind(string);
}

// Get data: wait and get data
void Server::GetData(std::string& data)
{
    socket.recv(request, zmq::recv_flags::none);

    data = request.to_string();
    return;
}

// Send data
void SendData(const std::string& data)
{
    socket.send(zmq::buffer(data), zmq::send_flags::none);
    return;
}