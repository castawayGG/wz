#pragma once
#include <Windows.h>
#include <vigem/Client.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace wz {

// A single step in a macro sequence
struct MacroStep {
    USHORT  button{0};   // XUSB_GAMEPAD_* bit, or 0 for trigger
    BYTE    triggerAxis{0}; // 1=LT, 2=RT
    BYTE    triggerValue{255};
    bool    press{true}; // true=press, false=release
    DWORD   delayMs{50}; // delay AFTER this step
};

struct MacroDef {
    std::string          name;
    std::vector<MacroStep> steps;
    bool                 loop{false}; // repeat until cancelled
};

enum class MacroTriggerMode { PressToPlay, Toggle, HoldToRepeat };

// Callback: apply a partial XUSB_REPORT overlay during macro playback
using MacroOutputCallback = std::function<void(USHORT buttons, BYTE lt, BYTE rt)>;

class MacroEngine {
public:
    explicit MacroEngine(MacroOutputCallback outputCb);
    ~MacroEngine();

    void AddMacro(const MacroDef& def);
    void RemoveMacro(const std::string& name);
    void ClearMacros();

    // Play a macro by name (non-blocking: runs on internal thread)
    bool Play(const std::string& name, MacroTriggerMode mode = MacroTriggerMode::PressToPlay);

    // Stop currently playing macro
    void Stop();

    bool IsPlaying() const;

    // Pre-built Warzone macros
    static MacroDef MakeSlideCancel();
    static MacroDef MakeTacticalSprint();
    static MacroDef MakeBunnyHop();

private:
    void PlayThreadProc(MacroDef def, MacroTriggerMode mode);

    std::vector<MacroDef>   m_macros;
    MacroOutputCallback     m_outputCb;
    std::thread             m_thread;
    std::atomic<bool>       m_playing{false};
    std::atomic<bool>       m_stopReq{false};
    mutable std::mutex      m_mutex;
};

} // namespace wz
