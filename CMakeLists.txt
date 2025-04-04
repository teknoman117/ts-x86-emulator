cmake_minimum_required(VERSION 3.28)
project(ts-3x00-emulator VERSION 0.1 LANGUAGES C CXX)

option(VIRTUAL_DISK "Enable a virtual C: drive" ON)

include(cmake/roms.cmake)

add_subdirectory(src)