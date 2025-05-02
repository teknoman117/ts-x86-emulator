#ifndef MEMORY_DEVICE_HPP
#define MEMORY_DEVICE_HPP

#include <cstdint>
#include <optional>
#include <tuple>

class MemoryDevice {
public:
    virtual ~MemoryDevice() = default;

    constexpr static int MAPPING_WRITEABLE = 1<<1;
    constexpr static int MAPPING_READABLE = 1<<2;

    // void* is a pointer to local memory, int is a collection of access flags
    using Mapping = std::tuple<void*, int>;

    virtual int mappable(size_t address) = 0;

    virtual std::optional<Mapping> map(size_t address, bool write = false) = 0;
    virtual void unmap(size_t address) = 0;
};

#endif /* MEMORY_DEVICE_HPP */