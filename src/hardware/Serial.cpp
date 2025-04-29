#include "Serial.hpp"
#include "util/Lock.hpp"

#include <cstring>

extern "C" {
    #include <pty.h>
    #include <unistd.h>
    #include <linux/kvm.h>
    #include <sys/eventfd.h>
    #include <sys/ioctl.h>
}

Serial16450::Serial16450(std::shared_ptr<uvw::loop>& loop_, const std::string& name,
        int vmFd, uint32_t gsi_)
    : loop{loop_}, mutex{loop->resource<uvw::mutex>()}, gsi{gsi_}, fds{}, registers{}
{
    logger = GetLogger(std::format("machine.serial.{}", name));

    // create a pty
    ptyPath.resize(PATH_MAX + 1, 0);
    if (openpty(&fds.pty_master, &fds.pty_slave, ptyPath.data(), nullptr, nullptr) < 0) {
        auto msg = std::format("failed to create pty: {}", strerror(errno));
        LOG4CXX_ERROR(logger, msg);
        throw std::runtime_error(msg);
    }

    auto linkPath = std::format("/tmp/{}.pty", name);
    std::filesystem::remove(linkPath);
    std::filesystem::create_symlink(getPtyPath(), linkPath);

    pty = loop->resource<uvw::tty_handle>(fds.pty_master, true);
    pty->mode(uvw::tty_handle::tty_mode::RAW);
    pty->on<uvw::data_event>([this] (auto& event, auto& handle) {
        // buffer data since we don't have a way of setting the read size
        {
            Lock lock(mutex);
            char *data = event.data.get();
            std::for_each(data, data + event.length, [this] (auto& p) {
                queue.push(p);
            });
        }
        handle.stop();

        // kick timer to handle interrupt logic
        if (!readTimer->active()) {
            readTimer->start(uvw::timer_handle::time(0), uvw::timer_handle::time(0));
        }
    });

    // create timer for "rx ready"
    readTimer = loop->resource<uvw::timer_handle>();
    readTimer->on<uvw::timer_event>([this] (auto& event, auto& handle) {
        // whenever the timeout occurs for the receive timer, re-enable the read interrupts
        bool empty = true;
        {
            Lock lock(mutex);
            empty = queue.empty();
        }
        if (empty) {
            pty->read();
        } else {
            registers.readable = true;
            registers.readInterruptFlag = true;
            if (registers.readInterruptEnabled) {
                // can't fire a new interrupt unless there aren't any pending
                if (!registers.writeInterruptEnabled || !registers.writeInterruptFlag) {
                    LOG4CXX_TRACE(logger, "triggering interrupt (read condition)");
                    triggerInterrupt();
                }
            }
        }
    });

    readNotify = loop->resource<uvw::async_handle>();
    readNotify->on<uvw::async_event>([this] (auto& event, auto& handle) {
        if (!readTimer->active()) {
            uvw::timer_handle::time delay((1600ULL * registers.divisor) / 18432ULL);
            readTimer->start(delay, uvw::timer_handle::time(0));
        }
    });

    // create timer for "tx ready"
    writeTimer = loop->resource<uvw::timer_handle>();
    writeTimer->on<uvw::timer_event>([this] (auto& event, auto& handle) {
        // whenever the timeout occurs for the send timer, re-enable the write interrupts
        registers.writable = true;
        registers.writeInterruptFlag = true;
        if (registers.writeInterruptEnabled) {
            // can't fire a new interrupt unless there aren't any pending
            if (!registers.readInterruptEnabled || !registers.readInterruptFlag) {
                LOG4CXX_TRACE(logger, "triggering interrupt (write ready condition)");
                triggerInterrupt();
            }
        }
    });

    writeNotify = loop->resource<uvw::async_handle>();
    writeNotify->on<uvw::async_event>([this] (auto& event, auto& handle) {
        if (!writeTimer->active()) {
            uvw::timer_handle::time delay((1600ULL * registers.divisor) / 18432ULL);
            writeTimer->start(delay, uvw::timer_handle::time(0));
        }
    });

    // create an irqfd for triggering interrupts
    fds.vm = vmFd;
    fds.irq = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (fds.irq == -1) {
        auto msg = std::format("failed create eventfd: {}", strerror(errno));
        LOG4CXX_ERROR(logger, msg);
        throw std::runtime_error(msg);
    }

    fds.resample = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (fds.resample == -1) {
        auto msg = std::format("failed create eventfd: {}", strerror(errno));
        LOG4CXX_ERROR(logger, msg);
        throw std::runtime_error(msg);
    }

    struct kvm_irqfd irqfd {
        .fd = (__u32) fds.irq,
        .gsi = gsi,
        .flags = KVM_IRQFD_FLAG_RESAMPLE,
        .resamplefd = (__u32) fds.resample
    };
    if (ioctl(fds.vm, KVM_IRQFD, &irqfd) == -1) {
        auto msg = std::format("failed add irqfd: {}", strerror(errno));
        LOG4CXX_ERROR(logger, msg);
        throw std::runtime_error(msg);
    }

#if 0
    // whenever the refresh event occurs, retrigger the interrupt if needed
    mEventLoop.addEvent(fds.refresh, EPOLLIN, [this] (uint32_t events) {
        if (events & EPOLLERR) {
            LOG_ERROR("error occurred with refresh event");
            return;
        }

        std::unique_lock<std::mutex> lock(mMutex);
        uint64_t data = 0;
        read(fds.refresh, &data, sizeof data);
        if ((registers.readInterruptEnabled && registers.readInterruptFlag)
                || (registers.writeInterruptEnabled && registers.writeInterruptFlag)) {
            LOG_INFO("triggering interrupt (kvm irq refresh)")
            triggerInterrupt();
        }
    });
#endif

    // kick off pty
    pty->read();
    registers.writable = true;
}

Serial16450::~Serial16450()
{
    pty.reset();
    close(fds.pty_slave);
    close(fds.pty_master);

    if (fds.vm != -1 && fds.irq != -1) {
        struct kvm_irqfd irqfd {
            .fd = (__u32) fds.irq,
            .gsi = gsi,
            .flags = KVM_IRQFD_FLAG_DEASSIGN,
            .resamplefd = (__u32) fds.resample
        };
        ioctl(fds.vm, KVM_IRQFD, &irqfd);
        close(fds.irq);
        close(fds.resample);
    }
}

void Serial16450::triggerInterrupt()
{
    uint64_t data = 1;
    write(fds.irq, &data, sizeof data);
}

void Serial16450::iowrite8(uint16_t address, uint8_t data)
{
    Lock lock(mutex);

    // 16450 uart occupies 8 bytes of address space
    Register r = static_cast<Register>(address & 0x7);
    switch (r) {
        case Register::Data_DivisorLowByte:
            if (!(registers.lineControl & 0x80) /* DLAB bit */) {
                pty->write((char *) &data, 1);
                registers.writable = false;
                registers.writeInterruptFlag = false;
                writeNotify->send();
            } else {
                reinterpret_cast<uint8_t*>(&registers.divisor)[0] = data;
            }
            break;
        case Register::InterruptControl_DivisorHighByte:
            if (!(registers.lineControl & 0x80) /* DLAB bit */) {
                bool pReadInterruptEnabled = registers.readInterruptEnabled;
                bool pWriteInterruptEnabled = registers.writeInterruptEnabled;
                registers.interruptControl = data & 0x0f;
                registers.readInterruptEnabled = !!(registers.interruptControl & 0x01);
                registers.writeInterruptEnabled = !!(registers.interruptControl & 0x02);
                LOG4CXX_TRACE(logger, "interrupt control: read: " << registers.readInterruptEnabled
                        << ", write: " << registers.writeInterruptEnabled);

                // if a particular interrupt was re-enabled, set the flag if the condition is met
                if (registers.readInterruptEnabled) {
                    registers.readInterruptFlag = registers.readable;
                }
                if (registers.writeInterruptEnabled) {
                    registers.writeInterruptFlag = registers.writable;
                }

                // trigger interrupt is a new interrupt condition has occurred and we weren't in
                // an interrupt cycle previously
                if ((!pReadInterruptEnabled || !registers.readInterruptFlag)
                        && (!pWriteInterruptEnabled || !registers.writeInterruptFlag)) {
                    if ((registers.readInterruptEnabled && registers.readInterruptFlag)
                            || (registers.writeInterruptEnabled && registers.writeInterruptFlag)) {
                        LOG4CXX_TRACE(logger, "triggering interrupt (pending before control register write condition)");
                        triggerInterrupt();
                    }
                }
            } else {
                reinterpret_cast<uint8_t*>(&registers.divisor)[1] = data;
            }
            break;
        case Register::InterruptStatus_FifoControl:
            // fifo register isn't implemented in 16450
            break;
        case Register::LineControl:
            registers.lineControl = data;
            break;
        case Register::ModemControl:
            registers.modemControl = data & 0x1F;
            break;
        case Register::LineStatus:
            // line status register isn't writable
            break;
        case Register::ModemStatus:
            // modem status register isn't writable
            break;
        case Register::Scratchpad:
            registers.scratchpad = data;
            break;
    }
}

uint8_t Serial16450::ioread8(uint16_t address)
{
    Lock lock(mutex);

    // 16450 uart occupies 8 bytes of address space
    Register r = static_cast<Register>(address & 0x7);
    switch (r) {
        case Register::Data_DivisorLowByte:
            if (!(registers.lineControl & 0x80) /* DLAB bit */) {
                if (!queue.empty()) {
                    registers.receive = queue.front();
                    queue.pop();
                }
                registers.readable = false;
                registers.readInterruptFlag = false;
                readNotify->send();
                return registers.receive;
            }
            return reinterpret_cast<uint8_t*>(&registers.divisor)[0];
        case Register::InterruptControl_DivisorHighByte:
            if (!(registers.lineControl & 0x80) /* DLAB bit */) {
                return registers.interruptControl;
            }
            return reinterpret_cast<uint8_t*>(&registers.divisor)[1];
        case Register::InterruptStatus_FifoControl:
            if (registers.readInterruptEnabled && registers.readInterruptFlag) {
                return 0x04;
            }
            if (registers.writeInterruptEnabled && registers.writeInterruptFlag) {
                // reading the interrupt status register clears the write interrupt condition
                // TODO: is the 16450 uart like the AVR uart where the txready signal *always*
                //       generates an interrupt? or does "clearing" the condition really cause
                //       it to stop generating the txready interrupt?
                registers.writeInterruptFlag = false;
                return 0x02;
            }
            // no interrupt
            return 0x01;
        case Register::LineControl:
            return registers.lineControl;
        case Register::ModemControl:
            return registers.modemControl;
        case Register::LineStatus:
            return (registers.writable ? 0x60 : 0x00) | (registers.readable ? 0x01 : 0x00);
        case Register::ModemStatus:
            // modem status register: we don't implement modem controls
            return 0;
        case Register::Scratchpad:
            return registers.scratchpad;
    }
    return 0xff;
}