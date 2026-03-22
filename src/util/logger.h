#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <sstream>
#include <Windows.h>

namespace wz {

enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERR = 3 };

class Logger {
public:
    static Logger& Instance();

    void SetLevel(LogLevel level);
    void SetFile(const std::string& path);
    void EnableConsole(bool enable);

    void Log(LogLevel level, const char* file, int line, const std::string& msg);

    // Latency measurement helpers
    LARGE_INTEGER GetTimestamp() const;
    double ElapsedMs(LARGE_INTEGER start, LARGE_INTEGER end) const;

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream   m_file;
    std::mutex      m_mutex;
    LogLevel        m_level{LogLevel::INFO};
    bool            m_console{false};
    LARGE_INTEGER   m_freq{};

    static const char* LevelStr(LogLevel l);
};

} // namespace wz

#define WZ_LOG(level, msg) \
    do { \
        std::ostringstream _oss; _oss << msg; \
        wz::Logger::Instance().Log(level, __FILE__, __LINE__, _oss.str()); \
    } while(0)

#define LOG_DEBUG(msg) WZ_LOG(wz::LogLevel::DEBUG, msg)
#define LOG_INFO(msg)  WZ_LOG(wz::LogLevel::INFO,  msg)
#define LOG_WARN(msg)  WZ_LOG(wz::LogLevel::WARN,  msg)
#define LOG_ERR(msg)   WZ_LOG(wz::LogLevel::ERR,   msg)
