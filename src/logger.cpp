#include "logger.h"
#include <mutex>
#include <chrono>
#ifdef __cpp_lib_format
#include <format>
#endif
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>
Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

void Logger::set_log_file(const std::string &file_name)
{
    std::lock_guard<std::mutex> lock(log_mutex_);
    if (log_file_.is_open())
    {
        log_file_.close();
    }
    log_file_.open(file_name, std::ios::app);
}

void Logger::log(LogLevel level, const std::string &message, bool to_user)
{
    using namespace std::chrono;
    std::lock_guard<std::mutex>
        lock(log_mutex_);
    system_clock::time_point now_tp = system_clock::now();
    std::time_t now_c = system_clock::to_time_t(now_tp);
    std::tm now_tm;
    localtime_r(&now_c, &now_tm);

    std::ostringstream prefix;
    prefix << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << " [";
    switch (level)
    {
    case LogLevel::INFO:
        prefix << "[Trace]";
        break;
    case LogLevel::WARNING:
        prefix << "[Warning]";
        break;
    case LogLevel::ERROR:
        prefix << "[Error]";
        break;
    case LogLevel::DEBUG:
        prefix << "[Debug]";
        break;
    }

    prefix << "] ";
    std::string log_message = prefix.str() + message;
    if (level >= LogLevel::DEBUG && show_debug_)
    {
        std::cerr << log_message << std::endl;
    }
    if (log_file_.is_open())
    {
        log_file_ << log_message << std::endl;
    }
    if (to_user)
    {
        std::cout << log_message << std::endl;
    }
}

void Logger::enable_debug()
{
    show_debug_ = true;
}