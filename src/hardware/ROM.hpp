#ifndef ROM_HPP
#define ROM_HPP

#include "MemoryDevice.hpp"

#include <filesystem>

class ROM final : public MemoryDevice {
    int fd{-1};
    void *data{nullptr};
    size_t length{0};
    size_t mask{0};

public:
    ROM(const std::filesystem::path& rom);
    virtual ~ROM();

    // MemoryDevice implementation
    int mappable(size_t address) override;
    std::optional<MemoryDevice::Mapping> map(size_t address, bool write = false) override;
    void unmap(size_t address) override;
};

#endif /* ROM_HPP */