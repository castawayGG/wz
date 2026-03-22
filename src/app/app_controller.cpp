#include "app_controller.h"
#include "../util/logger.h"
#include "../util/timer.h"
#include <sstream>
#include <algorithm>

namespace wz {

AppController::AppController()
    : m_macroEngine([this](USHORT btn, BYTE lt, BYTE rt) {
          std::lock_guard<std::mutex> lk(m_macroMutex);
          m_macroButtons = btn;
          m_macroLT = lt;
          m_macroRT = rt;
      })
    , m_autoSwitch(m_profileMgr, m_procMonitor)
{}

AppController::~AppController() {
    RequestExit();
}

bool AppController::Initialize(HINSTANCE hInst) {
    m_hInst = hInst;

    // Setup logging
    Logger::Instance().SetFile("wz_gamepad.log");
    Logger::Instance().SetLevel(LogLevel::INFO);
    LOG_INFO("WZ Virtual Gamepad starting...");

    // Initialize virtual gamepad
    if (!m_gamepad.Initialize()) {
        MessageBoxA(nullptr,
            ("ViGEmBus error: " + m_gamepad.GetLastError() +
             "\n\nPlease install ViGEmBus from:\nhttps://github.com/nefarius/ViGEmBus/releases").c_str(),
            "WZ Virtual Gamepad - Driver Error", MB_ICONERROR | MB_OK);
        return false;
    }
    if (!m_gamepad.Connect()) {
        LOG_ERR("Failed to connect virtual gamepad");
        return false;
    }

    m_gamepad.SetVibrationCallback([](UCHAR large, UCHAR small) {
        LOG_DEBUG("Vibration: large=" << (int)large << " small=" << (int)small);
    });

    // Load profiles from ./profiles directory
    m_profileMgr.LoadFromDirectory("profiles");

    // Apply default profile
    auto* profile = m_profileMgr.GetActiveProfile();
    if (profile) OnProfileChanged(*profile);

    m_profileMgr.SetChangeCallback([this](const Profile& p) { OnProfileChanged(p); });

    // Mode change callback
    m_modeSwitch.SetChangeCallback([this](InputMode m) { OnModeChanged(m); });

    // Input callbacks
    m_inputEngine.SetKeyCallback([this](const KeyEvent& ev) { OnKeyEvent(ev); });
    m_inputEngine.SetMouseCallback([this](const MouseEvent& ev) { OnMouseEvent(ev); });

    if (!m_inputEngine.Initialize()) {
        LOG_ERR("InputEngine init failed");
        return false;
    }

    // Tray icon
    m_trayIcon.Create(hInst, "WZ Virtual Gamepad");
    {
        std::vector<std::string> names;
        for (auto& p : m_profileMgr.GetProfiles()) names.push_back(p.name);
        m_trayIcon.SetProfiles(names, 0);
    }
    m_trayIcon.SetActionCallback([this](int id) {
        if (id == static_cast<int>(TrayMenuId::ToggleMode))
            m_modeSwitch.Toggle();
        else if (id == static_cast<int>(TrayMenuId::Exit))
            RequestExit();
        else if (id >= static_cast<int>(TrayMenuId::Profile0) &&
                 id < static_cast<int>(TrayMenuId::ProfileMax)) {
            int idx = id - static_cast<int>(TrayMenuId::Profile0);
            auto& profiles = m_profileMgr.GetProfiles();
            if (idx < static_cast<int>(profiles.size()))
                m_profileMgr.SetActiveProfile(profiles[idx].name);
        }
    });

    // OSD
    m_osd.Create(hInst);
    UpdateOsd();

    // AutoSwitch
    m_autoSwitch.Start();

    // Hotkeys
    SetupHotkeys();

    // Start gamepad update thread
    m_running.store(true);
    m_gamepadThread = std::thread(&AppController::GamepadUpdateLoop, this);

    LOG_INFO("AppController initialized");
    return true;
}

void AppController::Run() {
    MSG msg{};
    while (m_running.load()) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { m_running.store(false); break; }
            if (msg.message == WM_HOTKEY) {
                HotkeyManager::Instance().OnHotkey(static_cast<int>(msg.wParam));
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        WaitMessage();
    }
}

void AppController::RequestExit() {
    m_running.store(false);
    m_autoSwitch.Stop();
    m_inputEngine.Shutdown();
    HotkeyManager::Instance().UnregisterAll();
    m_osd.Destroy();
    m_trayIcon.Destroy();
    m_gamepad.Shutdown();
    if (m_gamepadThread.joinable()) m_gamepadThread.join();
    PostQuitMessage(0);
}

void AppController::SetupHotkeys() {
    auto* p = m_profileMgr.GetActiveProfile();
    UINT toggleVK    = p ? p->toggleModeVK    : VK_F9;
    UINT cycleVK     = p ? p->cycleProfileVK  : VK_F10;
    UINT layerVK     = p ? p->toggleLayerVK   : VK_F11;
    UINT stopVK      = p ? p->emergencyStopVK : VK_F12;

    HotkeyManager::Instance().Register(0, toggleVK, [this]() {
        m_modeSwitch.Toggle();
        m_osd.Flash(m_modeSwitch.IsGamepadMode() ? "Gamepad ON" : "Gamepad OFF");
    });
    HotkeyManager::Instance().Register(0, cycleVK, [this]() {
        m_profileMgr.CycleNext();
        m_osd.Flash("Profile: " + m_profileMgr.GetActiveProfileName());
    });
    HotkeyManager::Instance().Register(0, layerVK, [this]() {
        int cur = m_layerManager.GetActiveLayerIndex();
        if (cur == 0) m_layerManager.ActivateLayer(1);
        else m_layerManager.DeactivateAll();
    });
    HotkeyManager::Instance().Register(0, stopVK, [this]() {
        m_gamepad.Disconnect();
        m_osd.Flash("EMERGENCY STOP - Gamepad disconnected");
        LOG_WARN("Emergency stop triggered");
    });
}

void AppController::OnProfileChanged(const Profile& profile) {
    // Update mapping engine
    m_layerManager.SetBaseLayer(profile.baseMapping);
    m_mappingEngine.SetConfig(m_layerManager.GetEffectiveConfig());

    // Update axis processor
    MouseStickConfig mCfg = profile.mouseConfig;
    float sensMulti = m_layerManager.GetEffectiveSensitivityMultiplier();
    mCfg.sensitivityX *= sensMulti;
    mCfg.sensitivityY *= sensMulti;
    m_axisProcessor.SetMouseConfig(mCfg);
    m_axisProcessor.SetWasdConfig(profile.wasdConfig);

    // Update layers
    for (int i = 0; i < static_cast<int>(profile.layers.size()) && i < 3; ++i) {
        auto& l = profile.layers[i];
        m_layerManager.SetLayer(i + 1, l.name, l.config);
        m_layerManager.SetLayerSensitivity(i + 1, l.sensitivityMultiplier);
    }

    // Load macros
    m_macroEngine.ClearMacros();
    for (auto& m : profile.macros) m_macroEngine.AddMacro(m);

    // Update tray
    std::vector<std::string> names;
    for (auto& p : m_profileMgr.GetProfiles()) names.push_back(p.name);
    int idx = 0;
    for (int i = 0; i < static_cast<int>(names.size()); ++i)
        if (names[i] == profile.name) { idx = i; break; }
    m_trayIcon.SetProfiles(names, idx);

    UpdateOsd();
    LOG_INFO("Profile applied: " << profile.name);
}

void AppController::OnModeChanged(InputMode mode) {
    m_trayIcon.SetActive(mode == InputMode::GamepadEmulation);
    UpdateOsd();
}

void AppController::UpdateOsd() {
    OsdState st;
    st.profileName = m_profileMgr.GetActiveProfileName();
    st.layerName   = "L" + std::to_string(m_layerManager.GetActiveLayerIndex());
    st.gamepadMode = m_modeSwitch.IsGamepadMode();
    st.connected   = m_gamepad.IsConnected();
    m_osd.UpdateState(st);
}

void AppController::OnKeyEvent(const KeyEvent& ev) {
    {
        std::lock_guard<std::mutex> lk(m_stateMutex);
        m_keyState[ev.vkey] = ev.pressed;
    }

    // Check layer activators
    auto* profile = m_profileMgr.GetActiveProfile();
    if (profile) {
        for (int i = 0; i < static_cast<int>(profile->layers.size()) && i < 3; ++i) {
            USHORT activator = profile->layers[i].activatorVK;
            if (activator && ev.vkey == activator) {
                if (ev.pressed) m_layerManager.ActivateLayer(i + 1);
                else            m_layerManager.DeactivateLayer(i + 1);
                m_mappingEngine.SetConfig(m_layerManager.GetEffectiveConfig());
            }
        }
    }
}

void AppController::OnMouseEvent(const MouseEvent& ev) {
    std::lock_guard<std::mutex> lk(m_stateMutex);
    m_mouseDx += static_cast<float>(ev.dx);
    m_mouseDy += static_cast<float>(ev.dy);
    m_scrollDelta += ev.dwheel;

    if (ev.lbDown) m_lbDown = true;
    if (ev.lbUp)   m_lbDown = false;
    if (ev.rbDown) m_rbDown = true;
    if (ev.rbUp)   m_rbDown = false;
    if (ev.mbDown) m_mbDown = true;
    if (ev.mbUp)   m_mbDown = false;
    if (ev.x1Down) m_x1Down = true;
    if (ev.x1Up)   m_x1Down = false;
    if (ev.x2Down) m_x2Down = true;
    if (ev.x2Up)   m_x2Down = false;
}

void AppController::GamepadUpdateLoop() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    const double kTargetIntervalMs = 2.0; // 500 Hz
    Timer frameTimer;

    while (m_running.load()) {
        frameTimer.Reset();

        if (!m_modeSwitch.IsGamepadMode()) {
            Timer::SleepPreciseMs(kTargetIntervalMs);
            continue;
        }

        // Snapshot input state
        float  dx, dy;
        int    scroll;
        bool   lb, rb, mb, x1, x2;
        std::unordered_map<USHORT, bool> keySnap;
        {
            std::lock_guard<std::mutex> lk(m_stateMutex);
            dx = m_mouseDx; dy = m_mouseDy;
            m_mouseDx = m_mouseDy = 0.f;
            scroll = m_scrollDelta; m_scrollDelta = 0;
            lb = m_lbDown; rb = m_rbDown; mb = m_mbDown;
            x1 = m_x1Down; x2 = m_x2Down;
            keySnap = m_keyState;
        }

        XUSB_REPORT report{};

        // Buttons from keys
        m_mappingEngine.ApplyKeyState(keySnap, report.wButtons, report.bLeftTrigger, report.bRightTrigger);

        // Buttons from mouse
        m_mappingEngine.ApplyMouseButtons(lb, rb, mb, x1, x2,
            report.wButtons, report.bLeftTrigger, report.bRightTrigger);

        // Scroll -> D-pad
        report.wButtons |= m_mappingEngine.ProcessScroll(scroll);

        // WASD -> Left stick
        bool up    = keySnap['W'] || keySnap[VK_UP];
        bool down  = keySnap['S'] || keySnap[VK_DOWN];
        bool left  = keySnap['A'] || keySnap[VK_LEFT];
        bool right = keySnap['D'] || keySnap[VK_RIGHT];
        bool walk  = keySnap[VK_MENU]; // Alt
        m_axisProcessor.ProcessWasd(up, down, left, right, walk,
            kTargetIntervalMs, report.sThumbLX, report.sThumbLY);

        // Mouse -> Right stick
        m_axisProcessor.ProcessMouseDelta(dx, dy, kTargetIntervalMs,
            report.sThumbRX, report.sThumbRY);

        // Macro overlay
        {
            std::lock_guard<std::mutex> lk(m_macroMutex);
            report.wButtons     |= m_macroButtons;
            report.bLeftTrigger  = std::max(report.bLeftTrigger,  m_macroLT);
            report.bRightTrigger = std::max(report.bRightTrigger, m_macroRT);
            m_macroButtons = 0; m_macroLT = 0; m_macroRT = 0;
        }

        m_gamepad.UpdateState(report);

        frameTimer.WaitUntilMs(kTargetIntervalMs);
    }
}

} // namespace wz
