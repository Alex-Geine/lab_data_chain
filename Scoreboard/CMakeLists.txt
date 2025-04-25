cmake_minimum_required(VERSION 3.15)

project(scoreboard)

add_executable(scoreboard scoreboard.cpp)
target_link_libraries(scoreboard zmq)