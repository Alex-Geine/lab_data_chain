// Modulyator.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "QAM32Modulator.h"
#include <fstream>

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <chrono>
#include <thread>

#include "SignalGenerator.h"

#include <zmq.hpp>

using namespace std;

int main(int argc, char* argv[])
{
    bool isAllChain = false;

    // Получаем данные из коммандной строки
    if ((argc != 3) || (argc != 2))
    {
        std::cout << "Error! Arguments are bad!" << std::endl;
        for (int i = 0; i < argc; ++i)
            std::cout << argv[i] << std::endl;
        return 1;
    }
    if (argc == 3)
        isAllChain = true;

    std::string client_port = std::string(argv[1]);    
    std::string server_port = " 5558 ";
    if (isAllChain)
        server_port = std::string(argv[2]);

    // Подключаем клиент/сервер

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
    
    // Модулятор
    QAM32Modulator modulyator;
    PayloadGenerator generator;
    std::string output_signal;
    std::string coder_data;
    std::string input_bits;
    std::vector<uint8_t> gen_data(20);
    generator.setSeed(352357623);

    // Генерация тестовых битов (пример)
    //string input_bits = "101100110110011011000111010110";

    for (auto request_num = 0; request_num < 10; ++request_num)
    {
        // If all chain
        if (isAllChain)
        {
            // receive a request from coder
            socket_server.recv(request, zmq::recv_flags::none);
            coder_data = request.to_string();

            // send reply to coder
            socket_server.send(zmq::buffer("OK!"), zmq::send_flags::none);
            input_bits.resize(coder_data.size());
            
            // Convert data to mapper representation
            for (uint32_t i = 0; i < coder_data.size(); ++i)
                input_bits[i] = (coder_data[i]) ? '1' : '0';
        }
        else   // if mapper-demapper
        {
            generator.getData(gen_data);
            input_bits.resize(gen_data.size());
            for (uint32_t i = 0; i < gen_data.size(); ++i)
                input_bits[i] = (gen_data[i]) ? '1' : '0';
        }

        // Run mapper
        modulyator.run(input_bits, output_signal);

        // Send data to demapper
        socket_client.send(zmq::buffer(output_signal), zmq::send_flags::none);

        socket_client.recv(reply, zmq::recv_flags::none);
        std::cout << reply.to_string() << std::endl;
    }
    socket_client.close();
    socket_server.close();
    
    return 0;
}