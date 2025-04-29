#include "HexDisplay.hpp"
#include "Logger.hpp"

#include <format>

void HexDisplay::iowrite8(uint16_t address, uint8_t value)
{
    LOG4CXX_INFO(GetLogger("machine.hexdisplay"),
            std::format("{:#02x}", value));
}

void HexDisplay::iowrite16(uint16_t address, uint16_t value)
{
    LOG4CXX_INFO(GetLogger("machine.hexdisplay"),
            std::format("{:#04x}", value));
}

void HexDisplay::iowrite32(uint16_t address, uint32_t value)
{
    LOG4CXX_INFO(GetLogger("machine.hexdisplay"),
            std::format("{:#08x}", value));
}

void HexDisplay::iowrite64(uint16_t address, uint64_t value)
{
    LOG4CXX_INFO(GetLogger("machine.hexdisplay"),
            std::format("{:#016x}", value));
}