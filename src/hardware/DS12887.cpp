#include "DS12887.hpp"
#include "Logger.hpp"

#include <fstream>
#include <ctime>
#include <format>

DS12887::DS12887(const MachineConfig& config) : registers{}, selectedRegister{}, ram{}, cmosPath{config.getCMOSPath()}
{
    registers.D.validRamAndTime = true;

    std::ifstream nvram(cmosPath, std::ios_base::binary);
    if (nvram.is_open()) {
        nvram.read((char *) ram.data(), ram.size());
    }
}

// DevicePio implementation
void DS12887::iowrite8(uint16_t address, uint8_t value)
{
    auto logger = GetLogger("machine.rtc");

    bool isRegisterSelect = !(address & 1);
    if (isRegisterSelect) {
        selectedRegister = static_cast<Register>(value & 0x7F);
        return;
    }

    switch (selectedRegister) {
        case Register::Seconds:
            LOG4CXX_TRACE(logger, std::format("seconds write: {:#02x}", value));
            break;
        case Register::SecondsAlarm:
            if (!registers.B.DataMode) {
                registers.secondsAlarm = (value & 0x0f)
                        + 10 * ((value >> 4) & 0x07);
            } else {
                registers.secondsAlarm = value & 0x3f;
            }
            break;
        case Register::Minutes:
            LOG4CXX_TRACE(logger, std::format("minutes write: {:#02x}", value));
            break;
        case Register::MinutesAlarm:
            if (!registers.B.DataMode) {
                registers.minutesAlarm = (value & 0x0f)
                        + 10 * ((value >> 4) & 0x07);
            } else {
                registers.minutesAlarm = value & 0x3f;
            }
            break;
        case Register::Hours:
            LOG4CXX_TRACE(logger, std::format("hours write: {:#02x}", value));
            break;
        case Register::HoursAlarm:
            if (!registers.B.DataMode) {
                registers.hoursAlarm = (value & 0x0f)
                        + 10 * ((value >> 4) & 0x03);
            } else {
                registers.hoursAlarm = value & 0x1f;
            }
            if (!registers.B.twentyFourHourTime) {
                registers.hoursAlarm += (value & 0x80) ? 12 : 0;
            }
            break;
        case Register::Weekday:
            LOG4CXX_TRACE(logger, std::format("weekday write: {:#02x}", value));
            break;
        case Register::Day:
            LOG4CXX_TRACE(logger, std::format("day write: {:#02x}", value));
            break;
        case Register::Month:
            LOG4CXX_TRACE(logger, std::format("month write: {:#02x}", value));
            break;
        case Register::Year:
            LOG4CXX_TRACE(logger, std::format("year write: {:#02x}", value));
            break;
        case Register::A:
            *reinterpret_cast<uint8_t*>(&registers.A) = value & 0x7f;
            break;
        case Register::B:
            *reinterpret_cast<uint8_t*>(&registers.B) = value;
            break;
        case Register::C:
            // this register is not writable
            break;
        case Register::D:
            // this register is not writable
            break;
        case Register::Century:
            LOG4CXX_TRACE(logger, std::format("century write: {:#02x}", value));
            break;
        default:
        {
            ram[static_cast<uint8_t>(selectedRegister) - 14] = value;
            std::ofstream nvram(cmosPath, std::ios_base::binary);
            nvram.write((const char *) ram.data(), ram.size());
        }
    };
}

uint8_t mapToBCD(uint8_t value)
{
    return ((value / 10) << 4) + ((value % 10) & 0x0f);
}

uint8_t DS12887::ioread8(uint16_t address)
{
    bool isRegisterSelect = !(address & 1);
    if (isRegisterSelect) {
        LOG4CXX_WARN(GetLogger("machine.rtc"), "rtc index register was read");
        return 0;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    switch (selectedRegister) {
        case Register::Seconds:
            if (!registers.B.DataMode) {
                return mapToBCD(tm.tm_sec);
            } else {
                return tm.tm_sec;
            }
        case Register::SecondsAlarm:
            if (!registers.B.DataMode) {
                return mapToBCD(registers.secondsAlarm);
            } else {
                return registers.secondsAlarm;
            }
        case Register::Minutes:
            if (!registers.B.DataMode) {
                return mapToBCD(tm.tm_min);
            } else {
                return tm.tm_min;
            }
        case Register::MinutesAlarm:
            if (!registers.B.DataMode) {
                return mapToBCD(registers.minutesAlarm);
            } else {
                return registers.minutesAlarm;
            }
        case Register::Hours:
            if (!registers.B.DataMode) {
                if (registers.B.twentyFourHourTime) {
                    return mapToBCD(tm.tm_hour);
                } else {
                    return mapToBCD(tm.tm_hour % 12)
                            + ((tm.tm_hour / 12) ? 0x80 : 0x00);
                }
            } else {
                if (registers.B.twentyFourHourTime) {
                    return tm.tm_hour;
                } else {
                    return (tm.tm_hour % 12)
                            + ((tm.tm_hour / 12) ? 0x80 : 0x00);
                }
            }
        case Register::HoursAlarm:
            if (!registers.B.DataMode) {
                if (registers.B.twentyFourHourTime) {
                    return mapToBCD(registers.hoursAlarm);
                } else {
                    return mapToBCD(registers.hoursAlarm % 12)
                            + ((registers.hoursAlarm / 12) ? 0x80 : 0x00);
                }
            } else {
                if (registers.B.twentyFourHourTime) {
                    return registers.hoursAlarm;
                } else {
                    return (registers.hoursAlarm % 12)
                            + ((registers.hoursAlarm / 12) ? 0x80 : 0x00);
                }
            }
        case Register::Weekday:
            return tm.tm_wday + 1;
        case Register::Day:
            if (!registers.B.DataMode) {
                return mapToBCD(tm.tm_mday);
            } else {
                return tm.tm_mday;
            }
        case Register::Month:
            if (!registers.B.DataMode) {
                return mapToBCD(tm.tm_mon + 1);
            } else {
                return tm.tm_mon + 1;
            }
        case Register::Year:
            if (!registers.B.DataMode) {
                return mapToBCD(tm.tm_year % 100);
            } else {
                return tm.tm_year % 100;
            }
        case Register::Century:
            return mapToBCD((tm.tm_year + 1900) / 100);
        case Register::A:
            return *reinterpret_cast<uint8_t*>(&registers.A);
        case Register::B:
            return *reinterpret_cast<uint8_t*>(&registers.B);
        case Register::C:
            return *reinterpret_cast<uint8_t*>(&registers.C);
        case Register::D:
            return *reinterpret_cast<uint8_t*>(&registers.D);
    };

    return ram[static_cast<uint8_t>(selectedRegister) - 14];
}
