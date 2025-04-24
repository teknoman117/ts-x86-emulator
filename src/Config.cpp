#include "Config.hpp"

#include <filesystem>
#include <format>
#include <optional>

extern "C" {
    #include <unistd.h>
}

#include "InstallConfig.hpp"

using Path = std::filesystem::path;

namespace {
    Path getPathFromEnv(const char *name) {
        auto path = std::getenv(name);
        return (path) ? Path{path} : Path{};
    }

    class RuntimePaths final {
        Path basePath{};
        Path dataPath{};
        std::optional<Path> userDataPath{};

    public:
        RuntimePaths() {
            // figure out the base installation path 
            auto executable = std::filesystem::read_symlink(std::format("/proc/{}/exe", getpid()));
            Path binDir = Config::Install::BinaryDirectory();

            auto n = std::distance(binDir.begin(), binDir.end());
            auto m = std::distance(executable.begin(), executable.end());
        
            auto end = executable.begin();
            std::advance(end, m - n - 1);
            std::for_each(executable.begin(), end, [this] (auto& p) {
                basePath /= p;
            });

            // compute the install data path
            dataPath = basePath;
            dataPath /= Config::Install::DataDirectory();

            // figure out the user's local data directory
            auto userDataPath_ = getPathFromEnv("XDG_DATA_HOME");
            if (userDataPath_.empty()) {
                userDataPath_ = getPathFromEnv("HOME");
                if (!userDataPath_.empty()) {
                    userDataPath_ /= ".local";
                    userDataPath_ /= "share";
                }
            }

            if (!userDataPath_.empty()) {
                bool exists = false;
                userDataPath_ /= Config::Install::ProjectName();
                if (!(exists = std::filesystem::exists(userDataPath_))) {
                    exists = std::filesystem::create_directory(userDataPath_);
                }

                if (exists) {
                    userDataPath = userDataPath_;
                }
            }
        }

        ~RuntimePaths() = default;

        RuntimePaths(const RuntimePaths&) = delete;
        RuntimePaths(RuntimePaths&&) = delete;
        RuntimePaths& operator=(const RuntimePaths&) = delete;
        RuntimePaths& operator=(RuntimePaths&&) = delete;

        const Path& getBasePath() const { return basePath; }
        const Path& getDataPath() const { return dataPath; }
        const std::optional<Path>& getUserDataPath() const { return userDataPath; }
    };

    static RuntimePaths runtimePaths{};
}

namespace Config {
    const Path& GetInstallBasePath() {
        return runtimePaths.getBasePath();
    }

    const Path& GetInstallDataPath() {
        return runtimePaths.getDataPath();
    }

    const std::optional<Path>& GetLocalDataPath() {
        return runtimePaths.getUserDataPath();
    }

    const std::filesystem::path GetRomPath(const std::string& name) {
        return GetInstallDataPath() / "roms" / name;
    }
} // namespace Config