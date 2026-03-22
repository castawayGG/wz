#pragma once
#include <Windows.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>

namespace wz {

// Monitors the foreground process and calls callbacks when it changes.
class ProcessMonitor {
public:
    using Callback = std::function<void(const std::string& processName)>;

    ProcessMonitor();
    ~ProcessMonitor();

    // Start monitoring. pollIntervalMs = how often to poll (default 500ms)
    void Start(DWORD pollIntervalMs = 500);
    void Stop();

    // Register callback invoked when foreground process changes
    void SetChangeCallback(Callback cb);

    // Get current foreground process name (lowercase)
    std::string GetForegroundProcessName() const;

private:
    void ThreadProc(DWORD intervalMs);

    Callback        m_callback;
    std::thread     m_thread;
    std::atomic<bool> m_running{false};
    std::string     m_lastProcess;
};

} // namespace wz
