#pragma once
#include <atomic>
#include <functional>
#include <string>

namespace wz {

enum class InputMode { PassThrough, GamepadEmulation };

class ModeSwitch {
public:
    using ModeChangeCallback = std::function<void(InputMode newMode)>;

    ModeSwitch();

    void Toggle();
    void SetMode(InputMode mode);
    InputMode GetMode() const;
    bool IsGamepadMode() const;

    void SetChangeCallback(ModeChangeCallback cb);

private:
    std::atomic<int>   m_mode{static_cast<int>(InputMode::GamepadEmulation)};
    ModeChangeCallback m_cb;
};

} // namespace wz
