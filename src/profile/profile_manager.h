#pragma once
#include "profile_schema.h"
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <functional>

namespace wz {

class ProfileManager {
public:
    ProfileManager();
    ~ProfileManager() = default;

    // Load all profiles from a directory
    bool LoadFromDirectory(const std::string& dir);

    // Save a profile to directory
    bool SaveProfile(const Profile& profile, const std::string& dir);

    // CRUD
    void AddProfile(const Profile& profile);
    bool RemoveProfile(const std::string& name);
    Profile* GetProfile(const std::string& name);
    const std::vector<Profile>& GetProfiles() const;

    // Current active profile
    bool SetActiveProfile(const std::string& name);
    Profile* GetActiveProfile();
    const std::string& GetActiveProfileName() const;

    // Cycle to next profile
    void CycleNext();

    // Callbacks
    using ProfileChangeCallback = std::function<void(const Profile&)>;
    void SetChangeCallback(ProfileChangeCallback cb);

    // Build default Warzone profile
    static Profile MakeWarzoneDefault();
    static Profile MakeWarzoneSniper();
    static Profile MakeWarzoneAggressive();
    static Profile MakeWarzoneVehicle();
    static Profile MakeGenericFPS();

private:
    std::vector<Profile>    m_profiles;
    int                     m_activeIndex{0};
    ProfileChangeCallback   m_changeCb;
    mutable std::mutex      m_mutex;
};

} // namespace wz
