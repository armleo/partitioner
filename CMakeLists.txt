cmake_minimum_required(VERSION 3.10.0)
project(partitioner VERSION 0.1.0 LANGUAGES C CXX)

add_executable(partitioner main.cpp)

include(CTest)
enable_testing()

