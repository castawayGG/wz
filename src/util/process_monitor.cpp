#include "process_monitor.h"
#include "logger.h"
#include <Psapi.h>
#include <algorithm>
#include <cctype>

#pragma comment(lib, "Psapi.lib")

namespace wz {

ProcessMonitor::ProcessMonitor() = default;

ProcessMonitor::~ProcessMonitor() {
    Stop();
}

void ProcessMonitor::SetChangeCallback(Callback cb) {
    m_callback = std::move(cb);
}

void ProcessMonitor::Start(DWORD pollIntervalMs) {
    if (m_running.load()) return;
    m_running.store(true);
    m_thread = std::thread(&ProcessMonitor::ThreadProc, this, pollIntervalMs);
}

void ProcessMonitor::Stop() {
    m_running.store(false);
    if (m_thread.joinable()) m_thread.join();
}

std::string ProcessMonitor::GetForegroundProcessName() const {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return {};

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (!pid) return {};

    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc) return {};

    char buf[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    QueryFullProcessImageNameA(hProc, 0, buf, &size);
    CloseHandle(hProc);

    // Extract just the filename
    std::string full(buf);
    auto pos = full.find_last_of("\\/");
    std::string name = (pos != std::string::npos) ? full.substr(pos + 1) : full;

    // Lowercase
    std::transform(name.begin(), name.end(), name.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return name;
}

void ProcessMonitor::ThreadProc(DWORD intervalMs) {
    while (m_running.load()) {
        std::string current = GetForegroundProcessName();
        if (current != m_lastProcess) {
            m_lastProcess = current;
            LOG_DEBUG("Foreground process changed: " << current);
            if (m_callback) m_callback(current);
        }
        Sleep(intervalMs);
    }
}

} // namespace wz
