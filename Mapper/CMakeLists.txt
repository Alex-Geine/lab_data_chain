cmake_minimum_required(VERSION 3.15)

project(mapper)

add_executable(mapper Modulyator.cpp QAM32Modulator.cpp)
target_link_libraries(mapper zmq)