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

class ConvolutionalEncoder {
private:
    uint8_t m_registerSize;  // Размер регистра сдвига (K)
    uint8_t m_codeRate;      // Code rate (обычно 1/2, 1/3 и т.д.)
    std::vector<uint8_t> m_polynomials;  // Порождающие полиномы (в виде масок)
    uint8_t m_register;      // Текущее состояние регистра

public:
    ConvolutionalEncoder(uint8_t codeRate, uint8_t registerSize, 
                         const std::vector<uint8_t>& polynomials)
        : m_codeRate(codeRate), m_registerSize(registerSize), 
          m_polynomials(polynomials), m_register(0) {
        
        if (polynomials.size() != codeRate) {
            throw std::invalid_argument("Number of polynomials must match code rate denominator");
        }
    }

    void encode(const std::vector<uint8_t>& inputBits, std::vector<uint8_t>& outputBits) {
        outputBits.clear();
        outputBits.reserve(inputBits.size() * m_codeRate);

        for (uint8_t bit : inputBits) {
            // Проверка на допустимые значения битов
            if (bit != 0 && bit != 1) {
                throw std::invalid_argument("Input bits must be 0 or 1");
            }

            // Обновляем регистр (добавляем новый бит)
            m_register = (m_register << 1) | (bit & 0x1);

            // Для каждого полинома вычисляем выходной бит
            for (uint8_t poly : m_polynomials)
            {
                uint8_t outputBit = 0;
                // Вычисляем XOR битов, которые соответствуют единицам в полиноме
                for (uint8_t i = 0; i < m_registerSize; ++i) {
                    if (poly & (1 << i)) {
                        outputBit ^= (m_register >> i) & 0x1;
                    }
                }

                outputBits.push_back(outputBit);
            }
        }
    }

    void reset() {
        m_register = 0;
    }
};

// Пример использования:
int main(int argc, char* argv[]) {
    if (argc != 2)
    {
        std::cout << "Error! Arguments are bad!" << std::endl;
        for (int i = 0; i < argc; ++i)
            std::cout << argv[i] << std::endl;
        return 1;
    }

    std::string client_port = std::string(argv[1]);

    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    socket.connect("tcp://localhost:" + client_port);

    // set up some static data to send
    std::string data(100, '0');
    char* char_data = nullptr;

    // model init
    PayloadGenerator generator;
    generator.setSeed(352357623);

    // Параметры кодера: code rate = 1/2, K=3, полиномы 0b101 (5) и 0b111 (7)
    ConvolutionalEncoder encoder(2, 3, {5, 7});

    // Входные биты (пример)
    std::vector<uint8_t> inputBits(100);
    // Выходные биты
    std::vector<uint8_t> outputBits;


    for (auto request_num = 0; request_num < 10; ++request_num) 
    {
        generator.getData(inputBits);
        // Кодирование
        encoder.encode(inputBits, outputBits);

        for (int i = 0; i < 100; ++i)
            data[i] = (char)inputBits[i];

        socket.send(zmq::buffer(data), zmq::send_flags::none);
        
        // wait for reply from server
        zmq::message_t reply{};
        socket.recv(reply, zmq::recv_flags::none);

        std::cout << "Received " << reply.to_string(); 
        std::cout << " (" << request_num << ")";
        std::cout << std::endl;
    }

    socket.close();
    return 0;
}