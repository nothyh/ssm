#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <fstream>

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger
{
public:
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    static Logger &instance();
    void set_log_file(const std::string &file_name);
    void log(LogLevel level, const std::string &message, bool to_user = false);
    void enable_debug();

private:
    Logger() = default;
    std::ofstream log_file_;
    std::mutex log_mutex_;
    bool show_debug_ = false;
};

#define LOG_DEBUG(msg) Logger::instance().log(LogLevel::DEBUG, msg)
#define LOG_INFO(msg) Logger::instance().log(LogLevel::INFO, msg, true)
#define LOG_WARNING(msg) Logger::instance().log(LogLevel::WARNING, msg, true)
#define LOG_ERROR(msg) Logger::instance().log(LogLevel::ERROR, msg, true)

#endif