#include <vector>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include "SignalGenerator.h"
#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#include <zmq.hpp>

// Пример использования:
int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        std::cout << "Error! Arguments are bad!" << std::endl;
        for (int i = 0; i < argc; ++i)
            std::cout << argv[i] << std::endl;
        return 1;
    }
    std::string client_port = std::string(argv[1]);
    std::string server_port = std::string(argv[2]);

    // CLIENT 5556
    zmq::context_t context_client{1};
    zmq::context_t context_server{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket_client{context_client, zmq::socket_type::req};
    socket_client.connect("tcp://localhost:" + client_port);

    // SERVER 5555
    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket_server{context_server, zmq::socket_type::rep};
    socket_server.bind("tcp://*:" + server_port);

    zmq::message_t request;
    zmq::message_t reply;
    std::string coder_data;

    for (auto request_num = 0; request_num < 10; ++request_num) 
    {
        // receive a request from coder
        socket_server.recv(request, zmq::recv_flags::none);
        coder_data = request.to_string();

        // send reply to coder
        socket_server.send(zmq::buffer("OK!"), zmq::send_flags::none);

        // Processing ..

        // send data to scoreboard
        socket_client.send(zmq::buffer(coder_data), zmq::send_flags::none);

        // get rep from scoreboard and continue
        socket_client.recv(reply, zmq::recv_flags::none);
        std::cout << reply.to_string() << std::endl;
    }
    socket_client.close();
    socket_server.close();

    return 0;
}