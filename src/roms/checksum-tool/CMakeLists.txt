cmake_minimum_required(VERSION 3.28)
project(checksum-tool VERSION 0.1 LANGUAGES CXX)

add_executable(checksum-tool checksum-tool.cpp)
target_compile_features(checksum-tool PRIVATE cxx_std_17)