cmake_minimum_required(VERSION 3.28)

if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(checksum-tool ${PROJECT_BINARY_DIR}/host/checksum-tool)
else()

endif()

if(VIRTUAL_DISK)
    add_subdirectory(virtual-disk)
endif()