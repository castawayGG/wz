#include "macro_engine.h"
#include "../util/logger.h"
#include <algorithm>

namespace wz {

MacroEngine::MacroEngine(MacroOutputCallback outputCb)
    : m_outputCb(std::move(outputCb)) {}

MacroEngine::~MacroEngine() {
    Stop();
    if (m_thread.joinable()) m_thread.join();
}

void MacroEngine::AddMacro(const MacroDef& def) {
    std::lock_guard<std::mutex> lk(m_mutex);
    // Replace if exists
    auto it = std::find_if(m_macros.begin(), m_macros.end(),
        [&](const MacroDef& d){ return d.name == def.name; });
    if (it != m_macros.end()) *it = def;
    else m_macros.push_back(def);
}

void MacroEngine::RemoveMacro(const std::string& name) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_macros.erase(std::remove_if(m_macros.begin(), m_macros.end(),
        [&](const MacroDef& d){ return d.name == name; }), m_macros.end());
}

void MacroEngine::ClearMacros() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_macros.clear();
}

bool MacroEngine::Play(const std::string& name, MacroTriggerMode mode) {
    MacroDef def;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = std::find_if(m_macros.begin(), m_macros.end(),
            [&](const MacroDef& d){ return d.name == name; });
        if (it == m_macros.end()) {
            LOG_WARN("Macro not found: " << name);
            return false;
        }
        def = *it;
    }

    // Toggle: if playing the same, stop
    if (m_playing.load() && mode == MacroTriggerMode::Toggle) {
        Stop();
        return true;
    }

    Stop();
    if (m_thread.joinable()) m_thread.join();

    m_stopReq.store(false);
    m_playing.store(true);
    m_thread = std::thread(&MacroEngine::PlayThreadProc, this, def, mode);
    LOG_INFO("Playing macro: " << name);
    return true;
}

void MacroEngine::Stop() {
    m_stopReq.store(true);
    // Don't join here — caller may be on same thread
}

bool MacroEngine::IsPlaying() const {
    return m_playing.load();
}

void MacroEngine::PlayThreadProc(MacroDef def, MacroTriggerMode mode) {
    do {
        for (auto& step : def.steps) {
            if (m_stopReq.load()) break;

            USHORT btn = 0;
            BYTE   lt  = 0, rt = 0;
            if (step.press) {
                if (step.triggerAxis == 1) lt = step.triggerValue;
                else if (step.triggerAxis == 2) rt = step.triggerValue;
                else btn = step.button;
            }

            if (m_outputCb) m_outputCb(btn, lt, rt);
            Sleep(step.delayMs);

            // Release
            if (m_outputCb) m_outputCb(0, 0, 0);
        }
    } while (!m_stopReq.load() && def.loop);

    m_playing.store(false);
}

// ── Pre-built Warzone macros ───────────────────────────────────────────────

MacroDef MacroEngine::MakeSlideCancel() {
    MacroDef def;
    def.name = "slide_cancel";
    def.loop = false;

    // B press (crouch/slide)
    def.steps.push_back({XUSB_GAMEPAD_B, 0, 255, true, 50});
    def.steps.push_back({XUSB_GAMEPAD_B, 0, 255, false, 20});
    // B again
    def.steps.push_back({XUSB_GAMEPAD_B, 0, 255, true, 50});
    def.steps.push_back({XUSB_GAMEPAD_B, 0, 255, false, 20});
    // A (jump)
    def.steps.push_back({XUSB_GAMEPAD_A, 0, 255, true, 50});
    def.steps.push_back({XUSB_GAMEPAD_A, 0, 255, false, 0});
    return def;
}

MacroDef MacroEngine::MakeTacticalSprint() {
    MacroDef def;
    def.name = "tactical_sprint";
    def.loop = false;

    // Double-tap LS (left thumb stick click)
    def.steps.push_back({XUSB_GAMEPAD_LEFT_THUMB, 0, 255, true,  80});
    def.steps.push_back({XUSB_GAMEPAD_LEFT_THUMB, 0, 255, false, 30});
    def.steps.push_back({XUSB_GAMEPAD_LEFT_THUMB, 0, 255, true,  80});
    def.steps.push_back({XUSB_GAMEPAD_LEFT_THUMB, 0, 255, false, 0});
    return def;
}

MacroDef MacroEngine::MakeBunnyHop() {
    MacroDef def;
    def.name = "bunny_hop";
    def.loop = true; // Hold to repeat

    def.steps.push_back({XUSB_GAMEPAD_A, 0, 255, true,  50});
    def.steps.push_back({XUSB_GAMEPAD_A, 0, 255, false, 200});
    return def;
}

} // namespace wz
