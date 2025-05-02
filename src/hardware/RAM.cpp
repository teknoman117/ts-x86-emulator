#include "RAM.hpp"
#include "util/Math.hpp"

#include <stdexcept>

extern "C" {
    #include <sys/mman.h>
}

#define PAGE_SIZE 4096ULL

RAM::RAM(size_t length) : length{length}, mask{roundUpToPowerOfTwo(length)-1} {
    data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (data == (void*) -1) {
        throw std::runtime_error("failed to map memory for ram");
    }
}

int RAM::mappable(size_t address) {
    return MemoryDevice::MAPPING_READABLE | MemoryDevice::MAPPING_WRITEABLE;
}

std::optional<MemoryDevice::Mapping> RAM::map(size_t address, bool) {
    address = address & mask;
    if (address >= length) {
        return {};
    }

    return { { (void *) (static_cast<uint8_t*>(data) + address), MemoryDevice::MAPPING_READABLE } };
}

void RAM::unmap(size_t address) {
    address = address & mask;
    if (address >= length) {
        return;
    }

    madvise((void *)(static_cast<uint8_t*>(data) + address), PAGE_SIZE, MADV_DONTNEED);
}