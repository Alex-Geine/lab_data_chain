cmake_minimum_required(VERSION 3.15.0)
project(demapper VERSION 0.1.0 LANGUAGES C CXX)

add_executable(demapper QAM32_demapper.cpp)
target_link_libraries(demapper zmq)