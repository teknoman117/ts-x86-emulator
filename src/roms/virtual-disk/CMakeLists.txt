cmake_minimum_required(VERSION 3.28)

set(ROM_BASE "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/roms")
set(VIRTUAL_DISK_ROM "${ROM_BASE}/virtual-disk.rom")

# Compile the option rom
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/virtual-disk.rom"
    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/virtual-disk.asm"
    COMMAND
        ${CMAKE_ASM_NASM_COMPILER} -f bin -o "${CMAKE_CURRENT_BINARY_DIR}/virtual-disk.rom" "${CMAKE_CURRENT_LIST_DIR}/virtual-disk.asm"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating virtual-disk.rom"
)

# Generate the option rom binary file with the proper checksum
add_custom_command(
    OUTPUT "${VIRTUAL_DISK_ROM}"
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/virtual-disk.rom"
        checksum-tool
    COMMAND
        ${PROJECT_BINARY_DIR}/host/checksum-tool/checksum-tool "${CMAKE_CURRENT_BINARY_DIR}/virtual-disk.rom" "${VIRTUAL_DISK_ROM}"
    COMMENT "Generating virtual-disk.rom (with checksum)"
)

add_custom_target(virtual-disk DEPENDS "${VIRTUAL_DISK_ROM}")