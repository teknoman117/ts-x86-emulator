#ifndef SERIAL_HPP_
#define SERIAL_HPP_

#include "DevicePio.hpp"
#include "Logger.hpp"

#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <queue>

#include <uvw.hpp>

// maps a serial port to a unix pty

class Serial16450 : public DevicePio
{
    enum class Register : uint16_t {
        Data_DivisorLowByte = 0,
        InterruptControl_DivisorHighByte = 1,
        InterruptStatus_FifoControl = 2,
        LineControl = 3,
        ModemControl = 4,
        LineStatus = 5,
        ModemStatus = 6,
        Scratchpad = 7
    };

    std::shared_ptr<uvw::loop> loop;
    LoggerPtr logger;
    uint32_t gsi;

    std::queue<uint8_t> queue;
    std::shared_ptr<uvw::mutex> mutex;

    std::shared_ptr<uvw::tty_handle> pty;
    std::shared_ptr<uvw::timer_handle> readTimer;
    std::shared_ptr<uvw::timer_handle> writeTimer;
    std::shared_ptr<uvw::async_handle> readNotify;
    std::shared_ptr<uvw::async_handle> writeNotify;

    std::vector<char> ptyPath;

    struct __descriptors {
        int pty_master{-1};
        int pty_slave{-1};
        int vm{-1};
        int irq{-1};
        int resample{-1};
        __descriptors() = default;
    } fds{};

    struct {
        // direct registers
        uint8_t receive;
        uint16_t divisor;
        uint8_t interruptControl;
        uint8_t lineControl;
        uint8_t modemControl;
        uint8_t scratchpad;
        // used to construct other registers
        bool readable;
        bool writable;
        bool readInterruptFlag;
        bool writeInterruptFlag;
        bool readInterruptEnabled;
        bool writeInterruptEnabled;
    } registers;

    void triggerInterrupt();

public:
    Serial16450(std::shared_ptr<uvw::loop>& loop, const std::string& name, int vmFd, uint32_t gsi);

    Serial16450() = delete;
    Serial16450(const Serial16450&) = delete;
    Serial16450(Serial16450&& port) = delete;

    virtual ~Serial16450();

    Serial16450& operator=(const Serial16450&) = delete;
    Serial16450& operator=(Serial16450&& port) = delete;

    constexpr std::string_view getPtyPath() const { return ptyPath.data(); }

    // DevicePio implementation
    void iowrite8(uint16_t address, uint8_t value) override;
    uint8_t ioread8(uint16_t address) override;
};

#endif /* SERIAL_HPP_ */