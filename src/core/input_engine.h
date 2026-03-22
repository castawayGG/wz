#pragma once
#include <Windows.h>
#include <functional>
#include <atomic>
#include <thread>
#include <cstdint>

namespace wz {

// Raw keyboard event
struct KeyEvent {
    USHORT vkey;    // Virtual key code
    bool   pressed; // true = key down, false = key up
    bool   extended;// extended key flag
};

// Raw mouse event
struct MouseEvent {
    LONG   dx;          // Relative X movement
    LONG   dy;          // Relative Y movement
    int    dwheel;      // Wheel delta (signed)
    USHORT buttonFlags; // RI_MOUSE_* flags
    bool   lbDown, lbUp;
    bool   rbDown, rbUp;
    bool   mbDown, mbUp;
    bool   x1Down, x1Up;
    bool   x2Down, x2Up;
};

using KeyCallback   = std::function<void(const KeyEvent&)>;
using MouseCallback = std::function<void(const MouseEvent&)>;

class InputEngine {
public:
    InputEngine();
    ~InputEngine();

    // Initialize Raw Input and create message window
    bool Initialize();
    void Shutdown();

    void SetKeyCallback(KeyCallback cb);
    void SetMouseCallback(MouseCallback cb);

    // Check if a key is currently held
    bool IsKeyDown(USHORT vkey) const;

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void MessageLoop();
    void ProcessRawInput(HRAWINPUT hri);

    HWND            m_hwnd{nullptr};
    ATOM            m_wndClass{0};
    std::thread     m_thread;
    std::atomic<bool> m_running{false};

    KeyCallback     m_keyCb;
    MouseCallback   m_mouseCb;

    // Key state array indexed by VK code
    std::atomic<bool> m_keyState[256]{};
};

} // namespace wz
