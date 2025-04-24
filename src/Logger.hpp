#ifndef LOG_HPP
#define LOG_HPP

#include <log4cxx/logger.h>

#include <string>

using LoggerPtr = log4cxx::LoggerPtr;

class MachineConfig;

// Configure the logging system with a particular machine configuration
void ConfigureLogger(const MachineConfig& config);

// Get a logger
auto GetLogger(const std::string& name = std::string()) -> LoggerPtr;

#endif /* LOG_HPP */