#include "MachineConfig.hpp"

#include <format>

#include <CLI/CLI.hpp>

#include "Config.hpp"

namespace {
    std::map<std::string, MachineType> machineTypeMap{
        {"TS-3100", MachineType::TS_3100},
        {"TS-3200", MachineType::TS_3200},
        {"TS-3300", MachineType::TS_3300},
        {"TS-3400", MachineType::TS_3400},
    };

    const std::filesystem::path defaultFlashForMachine(MachineType& type) {
        switch (type) {
            case MachineType::TS_3100:
                return Config::GetInstallDataPath() / "roms" / "ts-3100.bin";
            case MachineType::TS_3200:
                return Config::GetInstallDataPath() / "roms" / "ts-3200.bin";
            case MachineType::TS_3300:
                return Config::GetInstallDataPath() / "roms" / "ts-3300.bin";
            case MachineType::TS_3400:
                return Config::GetInstallDataPath() / "roms" / "ts-3400.bin";
            default:
                return {};
        }
    }

    const std::filesystem::path defaultDiskForMachine(MachineType& type) {
        switch (type) {
            case MachineType::TS_3100:
                return Config::GetInstallDataPath() / "disks" / "ts-3100.img";
            case MachineType::TS_3200:
                return Config::GetInstallDataPath() / "disks" / "ts-3200.img";
            case MachineType::TS_3300:
                return Config::GetInstallDataPath() / "disks" / "ts-3300.img";
            case MachineType::TS_3400:
                return Config::GetInstallDataPath() / "disks" / "ts-3400.img";
            default:
                return {};
        }
    }
}

MachineConfig::MachineConfig(int argc, char** argv) {
    CLI::App app{"Emulator for Technologic Systems x86 Single Board Computers"};

    bool disableAutoConfiguration{false};
    auto disableAutoConfigurationOption = app.add_flag("--no-save-config", disableAutoConfiguration,
            "Disable automatic configuration")->configurable(false);

    // option for the configuration/data directory
    auto userDataOption = app.add_option("--data-directory", userDataPath,
            "Data directory for virtual machine storage")->configurable(false);
    if (userDataPath.empty()) {
        userDataOption->required();
    } else {
        userDataOption->capture_default_str();
    }

    // virtual machine name
    app.add_option("-n,--name", name, "Name of virtual machine instance")
            ->capture_default_str();

    // if we've disabled the configuration system, we don't need to specify the data directory
    app.preparse_callback([&] (size_t) {
        if (disableAutoConfigurationOption->count()) {
            userDataOption->required(false);
        }
    });

    // attempt to parse for data directory so we can generate defaults for 
    try {
        app.allow_extras(true);
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp& e) {
        // continue
        // we will exit in the next invocation, but for now, continue
    } catch (const CLI::ParseError& e) {
        app.exit(e);
        valid_ = false;
        return;
    }

    // If the data directory isn't empty, add our machine name to it
    if (!userDataPath.empty()) {
        userDataPath /= "machines";
        userDataPath /= name;
    }

    // add configuration file option
    auto configOption = app.set_config("-c,--config", "config.toml",
            "Load configuration from file", false);
    configOption->transform(CLI::FileOnDefaultPath(userDataPath, false));

    app.add_option("-t,--type", type, "Machine type")
            ->transform(CLI::CheckedTransformer(machineTypeMap, CLI::ignore_case))
            ->capture_default_str();

    cmosPath = userDataPath / "cmos.bin";
    app.add_option("-r,--cmos", cmosPath, "Path to CMOS binary")
            ->option_text("PATH")
            ->capture_default_str();

    flashPath = userDataPath / "flash.bin";
    app.add_option("-f,--flash", flashPath, "Path to flash binary")
            ->option_text("PATH")
            ->capture_default_str();
    
    diskPath = userDataPath / "disk.bin";
    app.add_option("-d,--disk", diskPath, "Path to disk image")
            ->option_text("PATH")
            ->capture_default_str();

    try {
        app.allow_extras(false);
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        app.exit(e);
        valid_ = false;
        return;
    }

    // populate the machine directory if necessary
    if (!disableAutoConfiguration) {
        if (!std::filesystem::exists(userDataPath)) {
            std::filesystem::create_directories(userDataPath);
        } else if (!std::filesystem::is_directory(userDataPath)) {
            throw std::runtime_error("user data directory is not a directory");
        }

        // copy over images if needed
        if (!std::filesystem::exists(flashPath)) {
            std::filesystem::copy(defaultFlashForMachine(type), flashPath);
        }

        if (!std::filesystem::exists(diskPath)) {
            std::filesystem::copy(defaultDiskForMachine(type), diskPath);
        }

        // save configuration file if it doesn't already exist
        const auto configPath = userDataPath / "config.toml";
        if (!std::filesystem::exists(configPath)) {
            std::ofstream configFile(configPath);
            configFile << app.config_to_str(true, true);
        }
    }
}