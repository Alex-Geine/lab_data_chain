cmake_minimum_required(VERSION 3.15)

project(mapper)

add_executable(mapper Modulyator.cpp)
target_link_libraries(mapper zmq)