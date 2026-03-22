#include "hotkey_manager.h"
#include "logger.h"

namespace wz {

HotkeyManager& HotkeyManager::Instance() {
    static HotkeyManager inst;
    return inst;
}

HotkeyManager::~HotkeyManager() {
    UnregisterAll();
}

int HotkeyManager::Register(UINT modifiers, UINT vk, std::function<void()> callback) {
    std::lock_guard<std::mutex> lk(m_mutex);
    int id = m_nextId++;
    if (!RegisterHotKey(nullptr, id, modifiers, vk)) {
        LOG_WARN("Failed to register hotkey id=" << id << " vk=0x" << std::hex << vk);
        return -1;
    }
    m_hotkeys[id] = HotkeyDef{modifiers, vk, std::move(callback)};
    LOG_DEBUG("Registered hotkey id=" << id);
    return id;
}

void HotkeyManager::Unregister(int id) {
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_hotkeys.find(id);
    if (it != m_hotkeys.end()) {
        UnregisterHotKey(nullptr, id);
        m_hotkeys.erase(it);
    }
}

void HotkeyManager::UnregisterAll() {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (auto& [id, def] : m_hotkeys)
        UnregisterHotKey(nullptr, id);
    m_hotkeys.clear();
}

void HotkeyManager::OnHotkey(int id) {
    std::function<void()> cb;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        auto it = m_hotkeys.find(id);
        if (it == m_hotkeys.end()) return;
        cb = it->second.callback;
    }
    if (cb) cb();
}

} // namespace wz
