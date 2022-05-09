cmake_minimum_required(VERSION 3.13)  # CMake version check
project(simple_example)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 11)            # Enable c++14 standard


add_executable(smash smash.cpp Commands.cpp Commands.h signals.cpp signals.h)

