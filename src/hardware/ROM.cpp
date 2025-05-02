#include "ROM.hpp"
#include "util/Math.hpp"

extern "C" {
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
}

#define PAGE_SIZE 4096ULL

ROM::ROM(const std::filesystem::path& rom) {
    fd = open(rom.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        throw std::runtime_error("failed to open rom file");
    }

    // get the length
    auto offset = lseek(fd, 0, SEEK_END);
    if (offset < 0) {
        throw std::runtime_error("failed to seek in the rom file");
    }
    length = offset;
    lseek(fd, 0, SEEK_SET);

    mask = roundUpToPowerOfTwo(length) - 1;

    // mmap the rom file
    data = mmap(nullptr, length, PROT_READ, MAP_SHARED, fd, 0);
    if (data == (void*) -1) {
        throw std::runtime_error("failed to mmap rom file");
    }
}

ROM::~ROM() {
    munmap(data, length);
    close(fd);
}

int ROM::mappable(size_t address) {
    return MemoryDevice::MAPPING_READABLE;
}

std::optional<MemoryDevice::Mapping> ROM::map(size_t address, bool write) {
    if (write) {
        return {};
    }

    address = address & mask;
    if (address >= length) {
        return {};
    }

    return { { (void *) (static_cast<uint8_t*>(data) + address), MemoryDevice::MAPPING_READABLE } };
}

void ROM::unmap(size_t address) {
    address = address & mask;
    if (address >= length) {
        return;
    }

    madvise((void *)(static_cast<uint8_t*>(data) + address), PAGE_SIZE, MADV_DONTNEED);
}