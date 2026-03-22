#pragma once
#include <Windows.h>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace wz {

struct HotkeyDef {
    UINT modifiers; // MOD_ALT, MOD_CONTROL, MOD_SHIFT, MOD_WIN, or 0
    UINT vk;        // Virtual key code
    std::function<void()> callback;
};

class HotkeyManager {
public:
    static HotkeyManager& Instance();

    // Register a global hotkey. Returns hotkey ID or -1 on failure.
    int Register(UINT modifiers, UINT vk, std::function<void()> callback);

    // Unregister by ID
    void Unregister(int id);

    // Unregister all
    void UnregisterAll();

    // Call from message loop when WM_HOTKEY is received
    void OnHotkey(int id);

private:
    HotkeyManager() = default;
    ~HotkeyManager();
    HotkeyManager(const HotkeyManager&) = delete;

    std::unordered_map<int, HotkeyDef> m_hotkeys;
    std::mutex m_mutex;
    int m_nextId{1000};
};

} // namespace wz
