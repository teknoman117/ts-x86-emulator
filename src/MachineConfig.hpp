#ifndef MACHINE_CONFIG_HPP
#define MACHINE_CONFIG_HPP

#include <optional>
#include <string>

enum class MachineType {
    Unknown,
    TS_3100,
    TS_3200,
    TS_3300,
    TS_3400,
};

class MachineConfig final {
    std::string name{"default"};
    MachineType type{MachineType::Unknown};

public:
    MachineConfig() = default;
    ~MachineConfig() = default;
    MachineConfig(const MachineConfig&) = default;
    MachineConfig(MachineConfig&&) = default;
    MachineConfig& operator=(const MachineConfig&) = default;
    MachineConfig& operator=(MachineConfig&&) = default;
};

#endif /* MACHINE_CONFIG_HPP */