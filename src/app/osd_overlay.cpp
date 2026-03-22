#include "osd_overlay.h"
#include "../util/logger.h"
#include <string>
#include <sstream>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

namespace wz {

static const wchar_t* kOsdClass = L"WzOsdOverlay";

OsdOverlay::OsdOverlay() = default;

OsdOverlay::~OsdOverlay() {
    Destroy();
}

bool OsdOverlay::Create(HINSTANCE hInst) {
    m_running.store(true);
    m_thread = std::thread(&OsdOverlay::MessageLoop, this, hInst);
    return true;
}

void OsdOverlay::Destroy() {
    m_running.store(false);
    if (m_hwnd) PostMessage(m_hwnd, WM_QUIT, 0, 0);
    if (m_thread.joinable()) m_thread.join();
}

void OsdOverlay::UpdateState(const OsdState& state) {
    m_state = state;
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, TRUE);
}

void OsdOverlay::Flash(const std::string& message, DWORD durationMs) {
    m_flashMsg = message;
    m_flashEnd = GetTickCount() + durationMs;
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, TRUE);
}

void OsdOverlay::SetVisible(bool visible) {
    m_visible = visible;
    if (m_hwnd) ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
}

bool OsdOverlay::IsVisible() const { return m_visible; }

void OsdOverlay::MessageLoop(HINSTANCE hInst) {
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = kOsdClass;
    wc.hbrBackground = nullptr;
    RegisterClassExW(&wc);

    // Position: top-left corner, small
    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        kOsdClass, L"WzOsd",
        WS_POPUP | WS_VISIBLE,
        10, 10, 320, 80,
        nullptr, nullptr, hInst, this);

    if (!m_hwnd) { m_running.store(false); return; }
    SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Semi-transparent black background
    SetLayeredWindowAttributes(m_hwnd, RGB(1, 1, 1), 200, LWA_ALPHA | LWA_COLORKEY);

    // Create font
    m_font = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");

    SetTimer(m_hwnd, 1, 50, nullptr); // 20Hz repaint for flash fade

    MSG msg{};
    while (m_running.load() && GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (m_font) { DeleteObject(m_font); m_font = nullptr; }
    DestroyWindow(m_hwnd);
    UnregisterClassW(kOsdClass, hInst);
    m_hwnd = nullptr;
}

LRESULT CALLBACK OsdOverlay::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* self = reinterpret_cast<OsdOverlay*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_CREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }
    if (msg == WM_PAINT && self) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        self->PaintOsd(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    if (msg == WM_TIMER && self) {
        // Auto-hide flash
        if (!self->m_flashMsg.empty() && GetTickCount() > self->m_flashEnd)
            self->m_flashMsg.clear();
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }
    if (msg == WM_ERASEBKGND) return 1;
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void OsdOverlay::PaintOsd(HDC hdc) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);

    // Black background
    HBRUSH bg = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);

    // Text
    SetTextColor(hdc, RGB(0, 255, 120));
    SetBkMode(hdc, TRANSPARENT);
    HFONT oldFont = (HFONT)SelectObject(hdc, m_font);

    std::string line1 = "WZ | Profile: " + m_state.profileName
        + " [" + m_state.layerName + "]";
    std::string line2 = std::string(m_state.gamepadMode ? "[GAMEPAD]" : "[PASSTHRU]")
        + (m_state.connected ? " CONNECTED" : " DISCONNECTED");

    std::wstring wl1(line1.begin(), line1.end());
    std::wstring wl2(line2.begin(), line2.end());

    RECT r1 = {5, 5, rc.right - 5, 25};
    RECT r2 = {5, 25, rc.right - 5, 45};
    DrawTextW(hdc, wl1.c_str(), -1, &r1, DT_LEFT | DT_SINGLELINE);
    DrawTextW(hdc, wl2.c_str(), -1, &r2, DT_LEFT | DT_SINGLELINE);

    // Flash message
    if (!m_flashMsg.empty() && GetTickCount() <= m_flashEnd) {
        SetTextColor(hdc, RGB(255, 220, 0));
        std::wstring wflash(m_flashMsg.begin(), m_flashMsg.end());
        RECT r3 = {5, 50, rc.right - 5, 70};
        DrawTextW(hdc, wflash.c_str(), -1, &r3, DT_LEFT | DT_SINGLELINE);
    }

    SelectObject(hdc, oldFont);
}

} // namespace wz
