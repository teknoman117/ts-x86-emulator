cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# Set log4cxx options
#set(ENABLE_COVERAGE OFF CACHE BOOL "Disable coverage testing")
#set(BUILD_TESTING OFF CACHE BOOL "Disable tests")
#set(BUILD_EXAMPLES OFF CACHE BOOL "Disable examples")
#set(LOG4CXX_NETWORKING_SUPPORT OFF CACHE BOOL "Disable log4cxx networking support")

# Fetch log4cxx
#FetchContent_Declare(
#    log4cxx
#    GIT_REPOSITORY https://github.com/apache/logging-log4cxx.git
#    GIT_TAG        rel/v1.4.0
#)
#FetchContent_MakeAvailable(log4cxx)

find_package(fmt REQUIRED)
find_package(log4cxx REQUIRED)

# Fetch uvw
set(USE_LIBCPP OFF CACHE BOOL "Disable using libc++ for uvw")

FetchContent_Declare(
    uvw
    GIT_REPOSITORY https://github.com/skypjack/uvw
    GIT_TAG        v3.4.0_libuv_v1.48
)
FetchContent_MakeAvailable(uvw)