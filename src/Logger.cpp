#include "Logger.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

void Logger::setLogFile(const std::string &filename) {
  std::lock_guard<std::mutex> lock(logMutex);
  logFile.open(filename, std::ios::app);
}

void Logger::log(LogLevel level, const std::string &message) {
  std::lock_guard<std::mutex> lock(logMutex);

  std::stringstream ss;
  ss << getTimestamp() << " [" << levelToString(level) << "] " << message << std::endl;

  if (logFile.is_open()) {
    logFile << ss.str();
    logFile.flush();
  }

  std::cout << ss.str();
}

std::string Logger::getTimestamp() const {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

std::string Logger::levelToString(LogLevel level) const {
  switch (level) {
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARNING:
      return "WARNING";
    case LogLevel::ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}
