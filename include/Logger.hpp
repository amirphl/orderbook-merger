#include <fstream>
#include <mutex>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class Logger {
 public:
  static Logger &getInstance() {
    static Logger instance;
    return instance;
  }

  void log(LogLevel level, const std::string &message);
  void setLogFile(const std::string &filename);

 private:
  Logger() = default;
  std::ofstream logFile;
  std::mutex logMutex;

  std::string getTimestamp() const;
  std::string levelToString(LogLevel level) const;
};

#define LOG_DEBUG(msg) Logger::getInstance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(LogLevel::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(LogLevel::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(LogLevel::ERROR, msg)
