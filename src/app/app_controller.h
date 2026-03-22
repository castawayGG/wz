#pragma once
#include "../core/input_engine.h"
#include "../core/virtual_gamepad.h"
#include "../core/mapping_engine.h"
#include "../core/axis_processor.h"
#include "../core/macro_engine.h"
#include "../core/layer_manager.h"
#include "../core/mode_switch.h"
#include "../profile/profile_manager.h"
#include "../profile/auto_switch.h"
#include "../util/process_monitor.h"
#include "../util/hotkey_manager.h"
#include "tray_icon.h"
#include "osd_overlay.h"
#include <Windows.h>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>

namespace wz {

class AppController {
public:
    AppController();
    ~AppController();

    bool Initialize(HINSTANCE hInst);
    void Run();       // Blocks until exit requested
    void RequestExit();

private:
    void SetupHotkeys();
    void OnProfileChanged(const Profile& profile);
    void OnModeChanged(InputMode mode);
    void UpdateOsd();
    void GamepadUpdateLoop(); // Dedicated high-priority thread

    void OnKeyEvent(const KeyEvent& ev);
    void OnMouseEvent(const MouseEvent& ev);

    // Input state (accessed from input thread, protected by m_stateMutex)
    std::mutex                      m_stateMutex;
    std::unordered_map<USHORT, bool> m_keyState;
    bool m_lbDown{false}, m_rbDown{false}, m_mbDown{false};
    bool m_x1Down{false}, m_x2Down{false};
    float m_mouseDx{0.f}, m_mouseDy{0.f};
    int   m_scrollDelta{0};

    // Components
    InputEngine     m_inputEngine;
    VirtualGamepad  m_gamepad;
    MappingEngine   m_mappingEngine;
    AxisProcessor   m_axisProcessor;
    MacroEngine     m_macroEngine;
    LayerManager    m_layerManager;
    ModeSwitch      m_modeSwitch;
    ProfileManager  m_profileMgr;
    ProcessMonitor  m_procMonitor;
    AutoSwitch      m_autoSwitch;
    TrayIcon        m_trayIcon;
    OsdOverlay      m_osd;

    // Gamepad update thread
    std::thread     m_gamepadThread;
    std::atomic<bool> m_running{false};

    HINSTANCE       m_hInst{nullptr};

    // Macro overlay injection
    USHORT m_macroButtons{0};
    BYTE   m_macroLT{0}, m_macroRT{0};
    std::mutex m_macroMutex;
};

} // namespace wz
