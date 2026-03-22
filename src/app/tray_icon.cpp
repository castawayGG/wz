#include "tray_icon.h"
#include "../util/logger.h"
#include <shellapi.h>
#include <commctrl.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "ComCtl32.lib")

namespace wz {

static const UINT WM_TRAY = WM_APP + 1;
static const wchar_t* kTrayWndClass = L"WzTrayWindow";

TrayIcon::TrayIcon() = default;

TrayIcon::~TrayIcon() {
    Destroy();
}

bool TrayIcon::Create(HINSTANCE hInst, const std::string& tooltip) {
    // Create message-only window to receive tray messages
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = [](HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
        auto* self = reinterpret_cast<TrayIcon*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (msg == WM_TRAY && self) { self->OnTrayMessage(wp, lp); return 0; }
        if (msg == WM_COMMAND && self && self->m_cb) {
            self->m_cb(static_cast<int>(LOWORD(wp))); return 0; }
        return DefWindowProcW(hwnd, msg, wp, lp);
    };
    wc.hInstance     = hInst;
    wc.lpszClassName = kTrayWndClass;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(0, kTrayWndClass, L"WzTray",
        0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInst, this);
    if (!m_hwnd) return false;
    SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Load icons from system (use stock icons as fallback)
    m_iconActive   = LoadIcon(nullptr, IDI_APPLICATION);
    m_iconInactive = LoadIcon(nullptr, IDI_INFORMATION);

    // Setup NOTIFYICONDATA
    m_nid         = {};
    m_nid.cbSize  = sizeof(m_nid);
    m_nid.hWnd    = m_hwnd;
    m_nid.uID     = 1;
    m_nid.uFlags  = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAY;
    m_nid.hIcon   = m_iconActive;

    // Convert tooltip to wide
    std::wstring wtip(tooltip.begin(), tooltip.end());
    wcsncpy_s(m_nid.szTip, wtip.c_str(), 127);

    Shell_NotifyIconW(NIM_ADD, &m_nid);
    m_created = true;
    LOG_INFO("Tray icon created");
    return true;
}

void TrayIcon::Destroy() {
    if (!m_created) return;
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    if (m_hwnd) { DestroyWindow(m_hwnd); m_hwnd = nullptr; }
    m_created = false;
}

void TrayIcon::SetTooltip(const std::string& text) {
    std::wstring wtip(text.begin(), text.end());
    wcsncpy_s(m_nid.szTip, wtip.c_str(), 127);
    m_nid.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::SetActive(bool active) {
    m_nid.uFlags = NIF_ICON;
    m_nid.hIcon  = active ? m_iconActive : m_iconInactive;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::SetProfiles(const std::vector<std::string>& profiles, int activeIndex) {
    m_profiles      = profiles;
    m_activeProfile = activeIndex;
}

void TrayIcon::SetActionCallback(TrayActionCallback cb) {
    m_cb = std::move(cb);
}

void TrayIcon::OnTrayMessage(WPARAM /*wp*/, LPARAM lp) {
    if (lp == WM_RBUTTONUP || lp == WM_CONTEXTMENU) {
        ShowContextMenu();
    }
}

void TrayIcon::ShowContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayMenuId::ToggleMode), L"Toggle Gamepad Mode");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);

    // Profile submenu
    HMENU hProfiles = CreatePopupMenu();
    for (int i = 0; i < static_cast<int>(m_profiles.size()); ++i) {
        std::wstring wname(m_profiles[i].begin(), m_profiles[i].end());
        UINT flags = MF_STRING;
        if (i == m_activeProfile) flags |= MF_CHECKED;
        AppendMenuW(hProfiles, flags,
            static_cast<UINT>(TrayMenuId::Profile0) + i, wname.c_str());
    }
    AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hProfiles), L"Profiles");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, static_cast<UINT>(TrayMenuId::Exit), L"Exit");

    SetForegroundWindow(m_hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, nullptr);
    DestroyMenu(hMenu);
}

} // namespace wz
