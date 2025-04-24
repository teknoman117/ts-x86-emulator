#include "Logger.hpp"

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/defaultconfigurator.h>
#include <log4cxx/logmanager.h>

#include "MachineConfig.hpp"

// Configure the logging system with a particular machine configuration
void ConfigureLogger(const MachineConfig& config) {
    using namespace log4cxx;

    // TODO: have 'config' specify a logging configuration
    BasicConfigurator::configure();
}

// Get a logger
auto GetLogger(const std::string& name) -> LoggerPtr {
    using namespace log4cxx;

    if (name.empty()) {
        return LogManager::getRootLogger();
    } else {
        return LogManager::getLogger(name);
    }
}