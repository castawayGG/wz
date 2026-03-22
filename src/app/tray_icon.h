#pragma once
#include <Windows.h>
#include <string>
#include <functional>
#include <vector>

namespace wz {

// Menu item IDs for tray context menu
enum class TrayMenuId {
    ToggleMode    = 1001,
    Profile0      = 2000, // profiles start at 2000
    ProfileMax    = 2099,
    OpenSettings  = 3001,
    Exit          = 3002,
};

using TrayActionCallback = std::function<void(int menuId)>;

class TrayIcon {
public:
    TrayIcon();
    ~TrayIcon();

    bool Create(HINSTANCE hInst, const std::string& tooltip);
    void Destroy();

    // Update tooltip text and icon state
    void SetTooltip(const std::string& text);
    void SetActive(bool active);     // changes icon color/state

    // Populate profile list in menu
    void SetProfiles(const std::vector<std::string>& profiles, int activeIndex);

    void SetActionCallback(TrayActionCallback cb);

    // Call from WM_APP+1 handler
    void OnTrayMessage(WPARAM wp, LPARAM lp);

private:
    void ShowContextMenu();
    void BuildMenu();

    NOTIFYICONDATAW    m_nid{};
    HWND               m_hwnd{nullptr};
    HICON              m_iconActive{nullptr};
    HICON              m_iconInactive{nullptr};
    TrayActionCallback m_cb;
    std::vector<std::string> m_profiles;
    int                m_activeProfile{0};
    bool               m_created{false};
};

} // namespace wz
