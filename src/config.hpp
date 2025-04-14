#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <log4cxx/logger.h>
 
using LoggerPtr = log4cxx::LoggerPtr;
 
extern auto getLogger(const std::string& name = std::string()) -> LoggerPtr;

#endif /* CONFIG_HPP */