#include "mapping_engine.h"
#include "../util/logger.h"

namespace wz {

MappingEngine::MappingEngine() = default;

void MappingEngine::SetConfig(const MappingConfig& cfg) {
    m_cfg = cfg;
}

const MappingConfig& MappingEngine::GetConfig() const {
    return m_cfg;
}

void MappingEngine::ApplyButton(const ButtonBinding& bind, bool pressed,
                                 USHORT& buttons, BYTE& lt, BYTE& rt) const {
    if (!pressed) return;

    if (bind.triggerAxis == 1) {
        lt = 255;
    } else if (bind.triggerAxis == 2) {
        rt = 255;
    } else if (bind.gamepadButton != 0) {
        buttons |= bind.gamepadButton;
    }
}

void MappingEngine::ApplyKeyState(
    const std::unordered_map<USHORT, bool>& keyState,
    USHORT& buttons, BYTE& lt, BYTE& rt) const
{
    for (auto& [vk, bind] : m_cfg.keyBindings) {
        auto it = keyState.find(vk);
        bool pressed = (it != keyState.end()) && it->second;
        ApplyButton(bind, pressed, buttons, lt, rt);
    }
}

void MappingEngine::ApplyMouseButtons(
    bool lb, bool rb, bool mb, bool x1, bool x2,
    USHORT& buttons, BYTE& lt, BYTE& rt) const
{
    auto apply = [&](int idx, bool pressed) {
        auto it = m_cfg.mouseBindings.find(idx);
        if (it != m_cfg.mouseBindings.end())
            ApplyButton(it->second, pressed, buttons, lt, rt);
    };
    apply(0, lb);
    apply(1, rb);
    apply(2, mb);
    apply(3, x1);
    apply(4, x2);
}

USHORT MappingEngine::ProcessScroll(int dwheel) const {
    if (!m_cfg.scrollDpadEnabled || dwheel == 0) return 0;
    return (dwheel > 0) ? XUSB_GAMEPAD_DPAD_UP : XUSB_GAMEPAD_DPAD_DOWN;
}

} // namespace wz
