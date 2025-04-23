#ifndef MACHINE_CONFIG_HPP
#define MACHINE_CONFIG_HPP

#include <optional>
#include <string>

#include "Config.hpp"

enum class MachineType {
    Unknown,
    TS_3100,
    TS_3200,
    TS_3300,
    TS_3400,
};

class MachineConfig final {
    using Path = std::filesystem::path;

    bool valid_{true};
    Path userDataPath{Config::GetLocalDataPath().value_or(Path{})};

    std::string name{"default"};
    MachineType type{MachineType::TS_3100};
    Path cmosPath{};
    Path flashPath{};
    Path diskPath{};

public:
    MachineConfig() = delete;
    MachineConfig(int argc, char** argv);
    ~MachineConfig() = default;
    MachineConfig(const MachineConfig&) = default;
    MachineConfig(MachineConfig&&) = default;
    MachineConfig& operator=(const MachineConfig&) = default;
    MachineConfig& operator=(MachineConfig&&) = default;

    bool valid() const { return valid_; }

    const Path& getCMOSPath() const { return cmosPath; }
    const Path& getFlashPath() const { return flashPath; }
    const Path& getDiskPath() const { return diskPath; }
};

#endif /* MACHINE_CONFIG_HPP */