cmake_minimum_required(VERSION 3.29)

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

target_compile_definitions(emulator PRIVATE
    $<$<BOOL:${VIRTUAL_DISK}>:VIRTUAL_DISK>
)

add_dependencies(emulator roms)