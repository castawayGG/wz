#include "logger.h"
#include <ctime>
#include <iomanip>
#include <iostream>

namespace wz {

Logger& Logger::Instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() {
    QueryPerformanceFrequency(&m_freq);
}

Logger::~Logger() {
    if (m_file.is_open()) m_file.close();
}

void Logger::SetLevel(LogLevel level) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_level = level;
}

void Logger::SetFile(const std::string& path) {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_file.is_open()) m_file.close();
    m_file.open(path, std::ios::app);
}

void Logger::EnableConsole(bool enable) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_console = enable;
}

void Logger::Log(LogLevel level, const char* file, int line, const std::string& msg) {
    if (level < m_level) return;

    // Timestamp
    SYSTEMTIME st{};
    GetLocalTime(&st);
    char ts[32];
    snprintf(ts, sizeof(ts), "%02d:%02d:%02d.%03d",
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // Extract just the filename portion
    const char* fname = file;
    for (const char* p = file; *p; ++p)
        if (*p == '\\' || *p == '/') fname = p + 1;

    char buf[1024];
    snprintf(buf, sizeof(buf), "[%s][%s][%s:%d] %s",
        ts, LevelStr(level), fname, line, msg.c_str());

    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_file.is_open()) {
        m_file << buf << '\n';
        m_file.flush();
    }
    if (m_console) {
        std::cout << buf << '\n';
    }
}

LARGE_INTEGER Logger::GetTimestamp() const {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return t;
}

double Logger::ElapsedMs(LARGE_INTEGER start, LARGE_INTEGER end) const {
    return static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0
           / static_cast<double>(m_freq.QuadPart);
}

const char* Logger::LevelStr(LogLevel l) {
    switch (l) {
        case LogLevel::DEBUG: return "DBG";
        case LogLevel::INFO:  return "INF";
        case LogLevel::WARN:  return "WRN";
        case LogLevel::ERR:   return "ERR";
        default:              return "???";
    }
}

} // namespace wz
