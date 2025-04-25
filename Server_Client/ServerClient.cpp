#include "ServerClient.h"

// Server

// Init Server/Client via socet_name. Example: "tcp://*:5555"
ServerClient::ServerClient(std::string socet_name, bool isServer)
{
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    if (isServer)
    {
        std::cout << "isServer" << std::endl;
        socket = zmq::socket_t{context, zmq::socket_type::rep};
        socket.bind("tcp://*:5555");
    }
    else
    {
        std::cout << "isClient" << std::endl;
        socket = zmq::socket_t{context, zmq::socket_type::req};
        socket.connect("tcp://localhost:5555");
    }

    std::cout << "socket_t: " << socket << std::endl;

    std::cout << "Done" << std::endl;
}

// Get data: wait and get data
void ServerClient::GetData(std::string& data)
{
    socket.recv(request, zmq::recv_flags::none);

    data = request.to_string();
    return;
}

// Send data
void ServerClient::SendData(const std::string& data)
{
    socket.send(zmq::buffer(data), zmq::send_flags::none);
    return;
}

//! Set data to user storage
void c_dpi_set_data_to_user_storage(unsigned int* size)
{
    uint8_t* data = new uint8_t[3]{1,2,3};
    svScope scope = svGetScope();
    svPutUserData(scope, c_dpi_soc_elab_data_buffer_get_data, data);

    *size = 3;
    return;
};

//! Get data from storage
void c_dpi_get_data_from_user_storage()
{
    uint8_t* data = nullptr;
    svScope scope = svGetScope();
    data = (uint8_t*)svGetUserData(scope, c_dpi_soc_elab_data_buffer_get_data);

    std::cout << "data[0]: " << data[0]<< ", data[1]: " << data[1] << ", data[2]: " << data[2] << std::endl;

    return;
}