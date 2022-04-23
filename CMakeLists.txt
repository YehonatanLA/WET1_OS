cmake_minimum_required(VERSION 3.20)
project(WET)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(WET
        Commands.cpp
        Commands.h
        signals.cpp
        signals.h
        smash.cpp)
