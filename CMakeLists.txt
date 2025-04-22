cmake_minimum_required(VERSION 3.28)
project(ts-x86-emulator VERSION 0.1 LANGUAGES C CXX ASM_NASM)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(VIRTUAL_DISK "Enable a virtual C: drive" ON)

include(GNUInstallDirs)

include(cmake/dependencies.cmake)
include(cmake/roms.cmake)

add_subdirectory(src)