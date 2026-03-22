#pragma once
#include <Windows.h>
#include <vigem/Client.h>
#include <functional>
#include <atomic>
#include <string>

namespace wz {

using VibrationCallback = std::function<void(UCHAR largMotor, UCHAR smallMotor)>;

class VirtualGamepad {
public:
    VirtualGamepad();
    ~VirtualGamepad();

    // Connect to ViGEmBus and plug in the virtual Xbox 360 controller
    bool Initialize();
    bool Connect();
    void Disconnect();
    void Shutdown();

    bool IsConnected() const;

    // Send an updated Xbox 360 report to the driver
    bool UpdateState(const XUSB_REPORT& report);

    // Set callback for vibration feedback from the game
    void SetVibrationCallback(VibrationCallback cb);

    // Get last error description
    std::string GetLastError() const;

private:
    static void CALLBACK VibrationNotification(
        PVIGEM_CLIENT client,
        PVIGEM_TARGET target,
        UCHAR largeMotor,
        UCHAR smallMotor,
        UCHAR ledNumber,
        LPVOID userData
    );

    PVIGEM_CLIENT      m_client{nullptr};
    PVIGEM_TARGET      m_target{nullptr};
    std::atomic<bool>  m_connected{false};
    VibrationCallback  m_vibCb;
    std::string        m_lastError;
};

} // namespace wz
