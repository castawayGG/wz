#pragma once
#include <Windows.h>
#include <string>
#include <atomic>
#include <thread>

namespace wz {

struct OsdState {
    std::string profileName;
    std::string layerName;
    bool        gamepadMode{true};
    bool        connected{true};
};

// Lightweight OSD overlay rendered as a topmost layered window
class OsdOverlay {
public:
    OsdOverlay();
    ~OsdOverlay();

    bool Create(HINSTANCE hInst);
    void Destroy();

    void UpdateState(const OsdState& state);
    void Flash(const std::string& message, DWORD durationMs = 2000);

    void SetVisible(bool visible);
    bool IsVisible() const;

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void PaintOsd(HDC hdc);
    void MessageLoop(HINSTANCE hInst);

    HWND          m_hwnd{nullptr};
    std::thread   m_thread;
    std::atomic<bool> m_running{false};

    OsdState      m_state;
    std::string   m_flashMsg;
    DWORD         m_flashEnd{0};
    bool          m_visible{true};

    HFONT         m_font{nullptr};
};

} // namespace wz
