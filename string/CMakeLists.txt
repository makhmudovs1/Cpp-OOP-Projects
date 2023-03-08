cmake_minimum_required(VERSION 3.24)
project(string)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(string
        string.cpp
        string.hpp)
