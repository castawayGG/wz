#include "auto_switch.h"
#include "../util/logger.h"

namespace wz {

AutoSwitch::AutoSwitch(ProfileManager& profileMgr, ProcessMonitor& procMonitor)
    : m_profileMgr(profileMgr), m_procMonitor(procMonitor)
{
    // Default Warzone process mappings
    m_mappings["cod.exe"]              = "warzone_default";
    m_mappings["modernwarfare.exe"]    = "warzone_default";
    m_mappings["warzone.exe"]          = "warzone_default";
}

AutoSwitch::~AutoSwitch() {
    Stop();
}

void AutoSwitch::AddMapping(const std::string& processName, const std::string& profileName) {
    m_mappings[processName] = profileName;
}

void AutoSwitch::RemoveMapping(const std::string& processName) {
    m_mappings.erase(processName);
}

void AutoSwitch::ClearMappings() {
    m_mappings.clear();
}

void AutoSwitch::SetFallbackProfile(const std::string& name) {
    m_fallbackProfile = name;
}

void AutoSwitch::Start() {
    if (m_started) return;
    m_started = true;
    m_procMonitor.SetChangeCallback([this](const std::string& name) {
        OnProcessChange(name);
    });
    m_procMonitor.Start(500);
    LOG_INFO("AutoSwitch started");
}

void AutoSwitch::Stop() {
    if (!m_started) return;
    m_started = false;
    m_procMonitor.Stop();
}

void AutoSwitch::OnProcessChange(const std::string& processName) {
    auto it = m_mappings.find(processName);
    if (it != m_mappings.end()) {
        LOG_INFO("AutoSwitch: " << processName << " -> " << it->second);
        m_profileMgr.SetActiveProfile(it->second);
    } else if (!m_fallbackProfile.empty()) {
        m_profileMgr.SetActiveProfile(m_fallbackProfile);
    }
}

} // namespace wz
