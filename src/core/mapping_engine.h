#pragma once
#include <Windows.h>
#include <vigem/Client.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace wz {

// How a button behaves when held
enum class HoldBehavior { Normal, Toggle, Turbo };

struct ButtonBinding {
    USHORT gamepadButton{0};     // XUSB_GAMEPAD_* bitmask value, or 0 for axis
    BYTE   triggerAxis{0};       // 1=LT, 2=RT; 0=none
    HoldBehavior holdMode{HoldBehavior::Normal};
    DWORD turboIntervalMs{50};
};

struct AxisBinding {
    // Which axis and direction
    enum class Axis { LeftX, LeftY, RightX, RightY } axis{Axis::LeftX};
    float value{1.0f}; // -1.0 to 1.0
};

// Maps VK codes to gamepad buttons/axes
using KeyMap    = std::unordered_map<USHORT, ButtonBinding>;
using MouseBtnMap = std::unordered_map<int, ButtonBinding>; // 0=L,1=R,2=M,3=X1,4=X2

struct MappingConfig {
    KeyMap      keyBindings;
    MouseBtnMap mouseBindings;

    // Modifier key for walk (partial left stick)
    USHORT      walkModifierVK{VK_MENU}; // Left Alt

    // Scroll wheel → D-pad
    bool        scrollDpadEnabled{true};
};

class MappingEngine {
public:
    MappingEngine();

    void SetConfig(const MappingConfig& cfg);
    const MappingConfig& GetConfig() const;

    // Called from input thread each frame.
    // Produces button bitmask and trigger values from current key states.
    void ApplyKeyState(const std::unordered_map<USHORT, bool>& keyState,
                       USHORT& buttons, BYTE& lt, BYTE& rt) const;

    // Apply mouse button state to buttons/triggers
    void ApplyMouseButtons(bool lb, bool rb, bool mb, bool x1, bool x2,
                           USHORT& buttons, BYTE& lt, BYTE& rt) const;

    // Process scroll wheel: returns d-pad bits to set for one frame
    USHORT ProcessScroll(int dwheel) const;

private:
    void ApplyButton(const ButtonBinding& bind, bool pressed,
                     USHORT& buttons, BYTE& lt, BYTE& rt) const;

    MappingConfig m_cfg;
};

} // namespace wz
