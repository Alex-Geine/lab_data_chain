cmake_minimum_required(VERSION 3.15)

project(decoder)

add_executable(decoder decoder.cpp)
target_link_libraries(decoder zmq)