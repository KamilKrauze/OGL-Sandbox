#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdarg>
#include <iostream>

enum class LogLevel : unsigned char
{
    Debug,
    Verbose,
    Info,
    Warning,
    Error,
    Success,
};

template<LogLevel lvl>
inline void log(const char* format, ...)
{
    const char* lvlMsg = '\0';
    const char* lvl_clr = '\0';
    switch (lvl)
    {
    case LogLevel::Debug:
        lvl_clr = "\033[95m";
        lvlMsg = "DEBUG";
        break;
    case LogLevel::Verbose:
        lvl_clr = "\033[37m";
        lvlMsg = "VERBOSE";
        break;
    case LogLevel::Info:
        lvl_clr = "\033[34m";
        lvlMsg = "INFO";
        break;
    case LogLevel::Warning:
        lvl_clr = "\033[93m";
        lvlMsg = "WARNING";
        break;
    case LogLevel::Error:
        lvl_clr = "\033[97m\033[41m";
        lvlMsg = "ERROR";
        break;
    case LogLevel::Success:
        lvl_clr = "\033[97m\033[42m";
        lvlMsg = "SUCCESS";
        break;
    }
    
    std::cout << lvl_clr << "[" << lvlMsg << "]: ";

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    std::cout << "\033[0m\n";
}

#define LOG_DEBUG(format, ...)		    log<LogLevel::Verbose>(format, ##__VA_ARGS__)
#define LOG_VERBOSE(format, ...)		log<LogLevel::Verbose>(format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)			log<LogLevel::Info>(format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...)		log<LogLevel::Warning>(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)			log<LogLevel::Error>(format, ##__VA_ARGS__)
#define LOG_SUCCESS(format, ...)		log<LogLevel::Success>(format, ##__VA_ARGS__)

#endif // !LOGGER_HPP