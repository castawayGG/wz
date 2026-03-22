#include "mode_switch.h"
#include "../util/logger.h"

namespace wz {

ModeSwitch::ModeSwitch() = default;

void ModeSwitch::Toggle() {
    int cur = m_mode.load();
    int next = (cur == static_cast<int>(InputMode::GamepadEmulation))
               ? static_cast<int>(InputMode::PassThrough)
               : static_cast<int>(InputMode::GamepadEmulation);
    m_mode.store(next);
    LOG_INFO("Mode switched to " << (next == static_cast<int>(InputMode::GamepadEmulation)
        ? "GamepadEmulation" : "PassThrough"));
    if (m_cb) m_cb(static_cast<InputMode>(next));
}

void ModeSwitch::SetMode(InputMode mode) {
    m_mode.store(static_cast<int>(mode));
    if (m_cb) m_cb(mode);
}

InputMode ModeSwitch::GetMode() const {
    return static_cast<InputMode>(m_mode.load());
}

bool ModeSwitch::IsGamepadMode() const {
    return m_mode.load() == static_cast<int>(InputMode::GamepadEmulation);
}

void ModeSwitch::SetChangeCallback(ModeChangeCallback cb) {
    m_cb = std::move(cb);
}

} // namespace wz
