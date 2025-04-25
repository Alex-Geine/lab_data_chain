#include <vector>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#include <zmq.hpp>
#include "SignalGenerator.h"

// Пример использования:
int main(int argc, char* argv[]) {
    if (argc != 2)
    {
        std::cout << "Error! Arguments are bad!" << std::endl;
        for (int i = 0; i < argc; ++i)
            std::cout << argv[i] << std::endl;
        return 1;
    }
    std::string server_port = std::string(argv[1]);
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket{context, zmq::socket_type::rep};
    socket.bind("tcp://*:" + server_port);

    // prepare some static data for responses
    std::string good_req{"OK Congrads"};
    std::string bad_req;
    std::string str;

    PayloadGenerator generator;
    generator.setSeed(352357623);

    // Входные биты (пример)
    std::vector<uint8_t> inputBits(100);

    
    std::cout << "Scoreboard" << std::endl;

    int counter = 0;
    for (int i = 0; i < 10; ++i) 
    {
        generator.getData(inputBits);
        zmq::message_t request;

        // receive a request from client
        socket.recv(request, zmq::recv_flags::none);
        str = request.to_string();

        for (int i = 0; i < str.size(); ++i)
            if ((uint8_t)str[i] != inputBits[i])
                counter++;

        std::cout << "Reseive data string: " << std::endl;
        for (int i = 0;i < str.size();++i)
            std::cout << (uint32_t)str[i] << " ";
        std::cout << std::endl;

        // send the reply to the client
        if (counter == 0)
            socket.send(zmq::buffer(good_req), zmq::send_flags::none);
        else
        {
            bad_req = {"ERROR in data! Number of dumped bits: " + std::to_string(counter)};
            socket.send(zmq::buffer(bad_req), zmq::send_flags::none);
        }
        counter = 0;
    }

    socket.close();

    return 0;
}