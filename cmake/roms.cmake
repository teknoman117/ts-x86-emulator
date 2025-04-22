cmake_minimum_required(VERSION 3.28)

include(FetchContent)

set(ROM_BASE "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/roms")
set(DISK_BASE "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/disks")

# TS-3x00 DOS
FetchContent_Declare(
    TS_3X00_DOS
    URL "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/binaries/DOS404.ZIP"
    URL_HASH "SHA256=97d989911df75a8a0ba2f2428f4af8e41ad0b765e84f2fc3af1e34e205a21ca6"
)

# TS-3100 Images
FetchContent_Declare(
    TS_3100_BIOS
    URL "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/binaries/3100BIOS.ZIP"
    URL_HASH "SHA256=cba00daa05e4c2c03c256dcb3e7c8a520aaec0a51081e2efc23a2212559edb16"
)

FetchContent_Declare(
    TS_3100_DISK
    URL "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/binaries/3100DISK.ZIP"
    URL_HASH "SHA256=527543502db2b63754d4e312ae6f5b1b2c3341686f0adea5d00ef71689306ef7"
)

FetchContent_Declare(
    TS_3100_UTILS
    URL "http://ftp.embeddedts.com/ftp/ts-x86-sbc/old-downloads/Disks/TS-3100.ZIP"
    URL_HASH "SHA256=6d8bd7119d5bdcbd63976252ea6e8611048acf8dc5b4ec67bf55db2f260a92bb"
)

FetchContent_MakeAvailable(
    TS_3X00_DOS
    TS_3100_BIOS
    TS_3100_DISK
    TS_3100_UTILS
)

# Generate TS-3100 Flash Image
set(TS_3100_FLASH_IMAGE "${ROM_BASE}/ts-3100.bin")

set(TS_3100_FLASH_IMAGE_SOURCES
    "${ts_3100_disk_SOURCE_DIR}/3100DISK.BIN"
    "${ts_3x00_dos_SOURCE_DIR}/DOS404.BIN"
    "${ts_3100_bios_SOURCE_DIR}/3100BIOS.BIN"
)

add_custom_command(
    OUTPUT
        ${TS_3100_FLASH_IMAGE}
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${ROM_BASE}
    COMMAND
        ${CMAKE_COMMAND} -E cat
            ${TS_3100_FLASH_IMAGE_SOURCES}
            > ${TS_3100_FLASH_IMAGE}
    DEPENDS
        ${TS_3100_FLASH_IMAGE_SOURCES}  
)


# Generate TS-3100 Virtual Disk
set(TS_3100_DISK_IMAGE "${DISK_BASE}/ts-3100.img")

set(CYLINDERS 32)
set(HEADS 255)
set(SECTORS 63)

math(EXPR TOTAL_SECTORS "${CYLINDERS} * ${HEADS} * ${SECTORS}")
math(EXPR TOTAL_SIZE "${TOTAL_SECTORS} * 512")
math(EXPR PARTITION_SECTORS "${TOTAL_SECTORS} - 64")

add_custom_command(
    OUTPUT
        ${TS_3100_DISK_IMAGE}
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${DISK_BASE}
    COMMAND
        ${CMAKE_COMMAND} -E echo \"drive c: file=\\\"${TS_3100_DISK_IMAGE}\\\" partition=1\"
            > ${PROJECT_BINARY_DIR}/ts-3100.conf
    COMMAND
        truncate -s ${TOTAL_SIZE} ${TS_3100_DISK_IMAGE}
    COMMAND
        ${CMAKE_COMMAND} -E env "MTOOLSRC=${PROJECT_BINARY_DIR}/ts-3100.conf"
            mpartition -ca -I -s ${SECTORS} -h ${HEADS} -t ${CYLINDERS} -T 6 c:
    COMMAND 
        ${CMAKE_COMMAND} -E env "MTOOLSRC=${PROJECT_BINARY_DIR}/ts-3100.conf"
            mformat -T ${PARTITION_SECTORS} -h ${HEADS} -s ${SECTORS} -H 0 c:
    COMMAND 
        ${CMAKE_COMMAND} -E env "MTOOLSRC=${PROJECT_BINARY_DIR}/ts-3100.conf"
            mcopy ${ts_3100_utils_SOURCE_DIR}/* c:
)

# Add a target to build all of the roms
add_custom_target(roms DEPENDS "${TS_3100_FLASH_IMAGE}" "${TS_3100_DISK_IMAGE}")
