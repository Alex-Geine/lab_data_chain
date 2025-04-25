cmake_minimum_required(VERSION 3.15)

project(coder)

add_executable(coder coder.cpp)
target_link_libraries(coder zmq)