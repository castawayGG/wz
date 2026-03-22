#include "input_engine.h"
#include "../util/logger.h"
#include <stdexcept>

namespace wz {

static const wchar_t* kWndClassName = L"WzRawInputWindow";

InputEngine::InputEngine() = default;

InputEngine::~InputEngine() {
    Shutdown();
}

void InputEngine::SetKeyCallback(KeyCallback cb)   { m_keyCb   = std::move(cb); }
void InputEngine::SetMouseCallback(MouseCallback cb){ m_mouseCb = std::move(cb); }

bool InputEngine::IsKeyDown(USHORT vkey) const {
    if (vkey >= 256) return false;
    return m_keyState[vkey].load();
}

bool InputEngine::Initialize() {
    m_running.store(true);
    m_thread = std::thread([this]() { MessageLoop(); });
    LOG_INFO("InputEngine started");
    return true;
}

void InputEngine::Shutdown() {
    if (!m_running.load()) return;
    m_running.store(false);
    if (m_hwnd) PostMessage(m_hwnd, WM_QUIT, 0, 0);
    if (m_thread.joinable()) m_thread.join();
    LOG_INFO("InputEngine stopped");
}

void InputEngine::MessageLoop() {
    // Set high priority for this thread
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    // Create invisible message-only window for Raw Input
    HINSTANCE hInst = GetModuleHandle(nullptr);
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = kWndClassName;
    m_wndClass = RegisterClassExW(&wc);
    if (!m_wndClass) {
        LOG_ERR("RegisterClassEx failed: " << GetLastError());
        return;
    }

    m_hwnd = CreateWindowExW(0, kWndClassName, L"WzInput",
        0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInst, this);
    if (!m_hwnd) {
        LOG_ERR("CreateWindowEx failed: " << GetLastError());
        return;
    }

    // Register for Raw Input: keyboard + mouse
    RAWINPUTDEVICE rid[2]{};
    rid[0].usUsagePage = 0x01; rid[0].usUsage = 0x06; // keyboard
    rid[0].dwFlags     = RIDEV_INPUTSINK;
    rid[0].hwndTarget  = m_hwnd;
    rid[1].usUsagePage = 0x01; rid[1].usUsage = 0x02; // mouse
    rid[1].dwFlags     = RIDEV_INPUTSINK;
    rid[1].hwndTarget  = m_hwnd;

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE))) {
        LOG_ERR("RegisterRawInputDevices failed: " << GetLastError());
        return;
    }
    LOG_INFO("Raw Input registered");

    MSG msg{};
    while (m_running.load()) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { m_running.store(false); break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // Small sleep to avoid busy-waiting when there's no input
        WaitMessage();
    }

    // Cleanup
    RAWINPUTDEVICE rid2[2]{};
    rid2[0].usUsagePage = 0x01; rid2[0].usUsage = 0x06;
    rid2[0].dwFlags     = RIDEV_REMOVE; rid2[0].hwndTarget = nullptr;
    rid2[1].usUsagePage = 0x01; rid2[1].usUsage = 0x02;
    rid2[1].dwFlags     = RIDEV_REMOVE; rid2[1].hwndTarget = nullptr;
    RegisterRawInputDevices(rid2, 2, sizeof(RAWINPUTDEVICE));

    DestroyWindow(m_hwnd);
    UnregisterClassW(kWndClassName, hInst);
    m_hwnd = nullptr;
}

LRESULT CALLBACK InputEngine::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_CREATE) {
        auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return 0;
    }

    auto* engine = reinterpret_cast<InputEngine*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_INPUT && engine) {
        engine->ProcessRawInput(reinterpret_cast<HRAWINPUT>(lp));
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void InputEngine::ProcessRawInput(HRAWINPUT hri) {
    UINT size = 0;
    GetRawInputData(hri, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    if (size == 0) return;

    thread_local std::vector<BYTE> buf;
    buf.resize(size);
    if (GetRawInputData(hri, RID_INPUT, buf.data(), &size, sizeof(RAWINPUTHEADER)) != size)
        return;

    auto* raw = reinterpret_cast<RAWINPUT*>(buf.data());

    if (raw->header.dwType == RIM_TYPEKEYBOARD) {
        auto& kb = raw->data.keyboard;
        USHORT vkey = kb.VKey;
        if (vkey == 0 || vkey >= 256) return;

        bool pressed = !(kb.Flags & RI_KEY_BREAK);
        bool extended = !!(kb.Flags & RI_KEY_E0);

        m_keyState[vkey].store(pressed);

        if (m_keyCb) {
            KeyEvent ev{vkey, pressed, extended};
            m_keyCb(ev);
        }
    } else if (raw->header.dwType == RIM_TYPEMOUSE) {
        auto& ms = raw->data.mouse;
        MouseEvent ev{};
        ev.dx = ms.lLastX;
        ev.dy = ms.lLastY;

        if (ms.usButtonFlags & RI_MOUSE_WHEEL)
            ev.dwheel = static_cast<short>(ms.usButtonData);

        ev.lbDown = !!(ms.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN);
        ev.lbUp   = !!(ms.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP);
        ev.rbDown = !!(ms.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN);
        ev.rbUp   = !!(ms.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP);
        ev.mbDown = !!(ms.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN);
        ev.mbUp   = !!(ms.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP);
        ev.x1Down = !!(ms.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN);
        ev.x1Up   = !!(ms.usButtonFlags & RI_MOUSE_BUTTON_4_UP);
        ev.x2Down = !!(ms.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN);
        ev.x2Up   = !!(ms.usButtonFlags & RI_MOUSE_BUTTON_5_UP);

        ev.buttonFlags = ms.usButtonFlags;

        if (m_mouseCb) m_mouseCb(ev);
    }
}

} // namespace wz
