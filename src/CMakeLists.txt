cmake_minimum_required(VERSION 3.28)

add_subdirectory(roms)

add_executable(emulator
    main.cpp
    EventLoop.cpp
    hardware/i386EXClockPrescaler.cpp
    hardware/ChipSelectUnit.cpp
    hardware/Timer.cpp
    hardware/Serial.cpp
    hardware/HexDisplay.cpp
    hardware/DS12887.cpp
)

target_link_libraries(emulator PRIVATE log4cxx uvw::uvw)

target_compile_definitions(emulator PRIVATE
    $<$<BOOL:${VIRTUAL_DISK}>:VIRTUAL_DISK>
)

target_compile_features(emulator PRIVATE cxx_std_17)

add_dependencies(emulator roms virtual-disk)