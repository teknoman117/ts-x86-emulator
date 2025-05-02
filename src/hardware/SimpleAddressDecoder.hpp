#ifndef SIMPLE_ADDRESS_DECODER_HPP
#define SIMPLE_ADDRESS_DECODER_HPP

#include "MemoryDevice.hpp"

class SimpleAddressDecoder final : public MemoryDevice {
    size_t address;
    size_t mask;

public:
    SimpleAddressDecoder(size_t address, size_t mask) : address{address}, mask{mask} {}
    ~SimpleAddressDecoder() = default;

    MemoryDevice *device{nullptr};

    // MemoryDevice implementation
    int mappable(size_t address_) override {
        if (device && ((address_ & mask) == (address & mask))) {
            return device->mappable(address_);
        } else {
            return 0;
        }
    }

    std::optional<MemoryDevice::Mapping> map(size_t address_, bool write = false) override {
        if (device && ((address_ & mask) == (address & mask))) {
            return device->map(address_, write);
        } else {
            return {};
        }
    }

    void unmap(size_t address_) override {
        if (device && ((address_ & mask) == (address & mask))) {
            return device->unmap(address_);
        }
    }
};

#endif /* SIMPLE_ADDRESS_DECODER_HPP */