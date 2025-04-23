cmake_minimum_required(VERSION 3.28)

add_subdirectory(roms)

configure_file("${CMAKE_CURRENT_LIST_DIR}/InstallConfig.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/InstallConfig.hpp")

add_executable(emulator
    main.cpp
    EventLoop.cpp
    MachineConfig.cpp
    Config.cpp
    hardware/i386EXClockPrescaler.cpp
    hardware/ChipSelectUnit.cpp
    hardware/Timer.cpp
    hardware/Serial.cpp
    hardware/HexDisplay.cpp
    hardware/DS12887.cpp
)

target_link_libraries(emulator PRIVATE log4cxx uvw::uvw CLI11::CLI11)

target_compile_definitions(emulator PRIVATE
    $<$<BOOL:${VIRTUAL_DISK}>:VIRTUAL_DISK>
)

target_include_directories(emulator PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_compile_features(emulator PRIVATE cxx_std_20)

set_target_properties(emulator PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}"
)

add_dependencies(emulator roms virtual-disk)