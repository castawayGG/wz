#include "profile_manager.h"
#include "import_export.h"
#include "../util/logger.h"
#include <algorithm>
#include <filesystem>

namespace wz {

ProfileManager::ProfileManager() {
    // Populate with built-in defaults
    m_profiles.push_back(MakeWarzoneDefault());
    m_profiles.push_back(MakeWarzoneSniper());
    m_profiles.push_back(MakeWarzoneAggressive());
    m_profiles.push_back(MakeWarzoneVehicle());
    m_profiles.push_back(MakeGenericFPS());
    m_activeIndex = 0;
}

bool ProfileManager::LoadFromDirectory(const std::string& dir) {
    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) return false;

    bool any = false;
    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (entry.path().extension() != ".json") continue;
        Profile p;
        if (ImportExport::LoadFromFile(entry.path().string(), p)) {
            // Replace existing or add
            auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                [&](const Profile& x){ return x.name == p.name; });
            if (it != m_profiles.end()) *it = p;
            else m_profiles.push_back(p);
            any = true;
            LOG_INFO("Loaded profile: " << p.name);
        }
    }
    return any;
}

bool ProfileManager::SaveProfile(const Profile& profile, const std::string& dir) {
    std::string path = dir + "\\" + profile.name + ".json";
    return ImportExport::SaveToFile(profile, path);
}

void ProfileManager::AddProfile(const Profile& profile) {
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&](const Profile& x){ return x.name == profile.name; });
    if (it != m_profiles.end()) *it = profile;
    else m_profiles.push_back(profile);
}

bool ProfileManager::RemoveProfile(const std::string& name) {
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&](const Profile& x){ return x.name == name; });
    if (it == m_profiles.end()) return false;
    m_profiles.erase(it);
    if (m_activeIndex >= static_cast<int>(m_profiles.size()))
        m_activeIndex = 0;
    return true;
}

Profile* ProfileManager::GetProfile(const std::string& name) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
        [&](const Profile& x){ return x.name == name; });
    return (it != m_profiles.end()) ? &(*it) : nullptr;
}

const std::vector<Profile>& ProfileManager::GetProfiles() const {
    return m_profiles;
}

bool ProfileManager::SetActiveProfile(const std::string& name) {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (int i = 0; i < static_cast<int>(m_profiles.size()); ++i) {
        if (m_profiles[i].name == name) {
            m_activeIndex = i;
            LOG_INFO("Active profile: " << name);
            if (m_changeCb) m_changeCb(m_profiles[i]);
            return true;
        }
    }
    return false;
}

Profile* ProfileManager::GetActiveProfile() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_profiles.empty()) return nullptr;
    return &m_profiles[m_activeIndex];
}

const std::string& ProfileManager::GetActiveProfileName() const {
    static std::string empty;
    if (m_profiles.empty()) return empty;
    return m_profiles[m_activeIndex].name;
}

void ProfileManager::CycleNext() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_profiles.empty()) return;
    m_activeIndex = (m_activeIndex + 1) % static_cast<int>(m_profiles.size());
    LOG_INFO("Cycled to profile: " << m_profiles[m_activeIndex].name);
    if (m_changeCb) m_changeCb(m_profiles[m_activeIndex]);
}

void ProfileManager::SetChangeCallback(ProfileChangeCallback cb) {
    m_changeCb = std::move(cb);
}

// ── Factory methods ────────────────────────────────────────────────────────

static MappingConfig MakeWarzoneMapping() {
    MappingConfig cfg;
    cfg.scrollDpadEnabled = true;
    cfg.walkModifierVK    = VK_MENU; // Left Alt

    // Key bindings
    auto& kb = cfg.keyBindings;
    kb[VK_SPACE]    = {XUSB_GAMEPAD_A,              0, HoldBehavior::Normal, 0};
    kb['C']         = {XUSB_GAMEPAD_B,              0, HoldBehavior::Normal, 0};
    kb['R']         = {XUSB_GAMEPAD_X,              0, HoldBehavior::Normal, 0};
    kb['F']         = {XUSB_GAMEPAD_Y,              0, HoldBehavior::Normal, 0};
    kb['G']         = {XUSB_GAMEPAD_LEFT_SHOULDER,  0, HoldBehavior::Normal, 0};
    kb['Q']         = {XUSB_GAMEPAD_RIGHT_SHOULDER, 0, HoldBehavior::Normal, 0};
    kb[VK_TAB]      = {XUSB_GAMEPAD_BACK,           0, HoldBehavior::Normal, 0};
    kb[VK_ESCAPE]   = {XUSB_GAMEPAD_START,          0, HoldBehavior::Normal, 0};
    kb[VK_SHIFT]    = {XUSB_GAMEPAD_LEFT_THUMB,     0, HoldBehavior::Normal, 0};
    kb[VK_CONTROL]  = {XUSB_GAMEPAD_RIGHT_THUMB,    0, HoldBehavior::Normal, 0};
    kb['1']         = {XUSB_GAMEPAD_DPAD_LEFT,      0, HoldBehavior::Normal, 0};
    kb['2']         = {XUSB_GAMEPAD_DPAD_RIGHT,     0, HoldBehavior::Normal, 0};

    // Mouse bindings: LMB=RT, RMB=LT
    auto& mb = cfg.mouseBindings;
    mb[0] = {0, 2, HoldBehavior::Normal, 0}; // LMB -> RT
    mb[1] = {0, 1, HoldBehavior::Normal, 0}; // RMB -> LT
    mb[3] = {XUSB_GAMEPAD_LEFT_SHOULDER,  0, HoldBehavior::Normal, 0}; // X1 -> LB
    mb[4] = {XUSB_GAMEPAD_A,             0, HoldBehavior::Normal, 0}; // X2 -> A

    return cfg;
}

Profile ProfileManager::MakeWarzoneDefault() {
    Profile p;
    p.name        = "warzone_default";
    p.version     = "1.0";
    p.baseMapping = MakeWarzoneMapping();

    // Mouse -> Right stick
    p.mouseConfig.sensitivityX       = 15.0f;
    p.mouseConfig.sensitivityY       = 13.0f;
    p.mouseConfig.adsSensMultiplier  = 0.6f;
    p.mouseConfig.smoothingAlpha     = 0.3f;
    p.mouseConfig.jitterThreshold    = 2.0f;
    p.mouseConfig.autoCenterDecayMs  = 50.0f;
    p.mouseConfig.curve.type         = CurveType::SCurve;
    p.mouseConfig.deadZone.innerPercent = 5.0f;
    p.mouseConfig.deadZone.outerPercent = 95.0f;

    // WASD -> Left stick
    p.wasdConfig.rampUpMs         = 30.0f;
    p.wasdConfig.walkMultiplier   = 0.5f;
    p.wasdConfig.diagonalNorm     = true;

    // ADS shift layer (when RMB held, lower sensitivity)
    LayerDef adsLayer;
    adsLayer.name                      = "ads";
    adsLayer.activatorVK               = VK_RBUTTON;
    adsLayer.sensitivityMultiplier     = 0.6f;
    adsLayer.config                    = MakeWarzoneMapping();
    p.layers.push_back(adsLayer);

    // Pre-built macros
    p.macros.push_back(MacroEngine::MakeSlideCancel());
    p.macros.push_back(MacroEngine::MakeTacticalSprint());
    p.macros.push_back(MacroEngine::MakeBunnyHop());

    return p;
}

Profile ProfileManager::MakeWarzoneSniper() {
    Profile p = MakeWarzoneDefault();
    p.name = "warzone_sniper";
    p.mouseConfig.sensitivityX      = 8.0f;
    p.mouseConfig.sensitivityY      = 7.0f;
    p.mouseConfig.smoothingAlpha    = 0.15f;
    p.mouseConfig.adsSensMultiplier = 0.4f;
    return p;
}

Profile ProfileManager::MakeWarzoneAggressive() {
    Profile p = MakeWarzoneDefault();
    p.name = "warzone_aggressive";
    p.mouseConfig.sensitivityX      = 22.0f;
    p.mouseConfig.sensitivityY      = 20.0f;
    p.mouseConfig.smoothingAlpha    = 0.5f;
    p.wasdConfig.rampUpMs           = 15.0f;
    return p;
}

Profile ProfileManager::MakeWarzoneVehicle() {
    Profile p;
    p.name = "warzone_vehicle";

    MappingConfig cfg;
    cfg.scrollDpadEnabled = true;
    auto& kb = cfg.keyBindings;
    kb[VK_SPACE]   = {XUSB_GAMEPAD_A,     0, HoldBehavior::Normal, 0};
    kb['C']        = {XUSB_GAMEPAD_B,     0, HoldBehavior::Normal, 0};
    kb[VK_SHIFT]   = {XUSB_GAMEPAD_X,     0, HoldBehavior::Normal, 0};
    kb['F']        = {XUSB_GAMEPAD_Y,     0, HoldBehavior::Normal, 0};
    kb[VK_ESCAPE]  = {XUSB_GAMEPAD_START, 0, HoldBehavior::Normal, 0};
    kb[VK_TAB]     = {XUSB_GAMEPAD_BACK,  0, HoldBehavior::Normal, 0};

    auto& mb = cfg.mouseBindings;
    mb[0] = {0, 2, HoldBehavior::Normal, 0};
    mb[1] = {0, 1, HoldBehavior::Normal, 0};
    p.baseMapping = cfg;

    p.mouseConfig = {};
    p.wasdConfig  = {};
    p.wasdConfig.rampUpMs = 60.0f; // Slower ramp for vehicles
    return p;
}

Profile ProfileManager::MakeGenericFPS() {
    Profile p = MakeWarzoneDefault();
    p.name = "generic_fps";
    p.mouseConfig.sensitivityX = 12.0f;
    p.mouseConfig.sensitivityY = 12.0f;
    p.mouseConfig.curve.type   = CurveType::Linear;
    return p;
}

} // namespace wz
