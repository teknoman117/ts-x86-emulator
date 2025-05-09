#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <filesystem>
#include <optional>
#include <string>

namespace Config {
    // return the installation base path
    const std::filesystem::path& GetInstallBasePath();

    // return the installation data path (disk images, roms, etc.)
    const std::filesystem::path& GetInstallDataPath();

    // return the user local data path
    const std::optional<std::filesystem::path>& GetLocalDataPath();

    // return the path to a rom
    const std::filesystem::path GetRomPath(const std::string& name);
} // namespace Config

#endif /* CONFIG_HPP */