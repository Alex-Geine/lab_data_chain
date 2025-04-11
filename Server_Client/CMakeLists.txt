cmake_minimum_required(VERSION 3.15)

project(server_client)

add_executable(client Client.cpp)
add_executable(server Server.cpp)

target_link_libraries(client zmq)
target_link_libraries(server zmq)