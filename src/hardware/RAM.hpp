#ifndef RAM_HPP
#define RAM_HPP

#include "MemoryDevice.hpp"

class RAM final : public MemoryDevice {
    size_t length;
    size_t mask{0};
    void* data;

public:
    RAM(size_t length);
    ~RAM() = default;

    // MemoryDevice implementation
    int mappable(size_t address) override;
    std::optional<MemoryDevice::Mapping> map(size_t address, bool write = false) override;
    void unmap(size_t address) override;
};

#endif /* RAM_HPP */