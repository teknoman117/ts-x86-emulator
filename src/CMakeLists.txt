cmake_minimum_required(VERSION 3.14)

add_executable(kvm-emulator
    main.cpp
    EventLoop.cpp
    hardware/i386EXClockPrescaler.cpp
    hardware/ChipSelectUnit.cpp
    hardware/Timer.cpp
    hardware/Serial.cpp
    hardware/HexDisplay.cpp
    hardware/DS12887.cpp
)

target_compile_definitions(kvm-emulator PRIVATE
    $<$<BOOL:${VIRTUAL_DISK}>:VIRTUAL_DISK>
)