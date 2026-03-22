#pragma once
#include "../util/process_monitor.h"
#include "profile_manager.h"
#include <unordered_map>
#include <string>

namespace wz {

class AutoSwitch {
public:
    AutoSwitch(ProfileManager& profileMgr, ProcessMonitor& procMonitor);
    ~AutoSwitch();

    // Map process name (lowercase, e.g. "cod.exe") -> profile name
    void AddMapping(const std::string& processName, const std::string& profileName);
    void RemoveMapping(const std::string& processName);
    void ClearMappings();

    void SetFallbackProfile(const std::string& name);

    void Start();
    void Stop();

private:
    void OnProcessChange(const std::string& processName);

    ProfileManager&   m_profileMgr;
    ProcessMonitor&   m_procMonitor;
    std::unordered_map<std::string, std::string> m_mappings;
    std::string       m_fallbackProfile{"warzone_default"};
    bool              m_started{false};
};

} // namespace wz
