#include "virtual_gamepad.h"
#include "../util/logger.h"
#include <sstream>

namespace wz {

VirtualGamepad::VirtualGamepad() = default;

VirtualGamepad::~VirtualGamepad() {
    Shutdown();
}

bool VirtualGamepad::Initialize() {
    m_client = vigem_alloc();
    if (!m_client) {
        m_lastError = "vigem_alloc failed — out of memory";
        LOG_ERR(m_lastError);
        return false;
    }

    VIGEM_ERROR err = vigem_connect(m_client);
    if (!VIGEM_SUCCESS(err)) {
        std::ostringstream oss;
        oss << "vigem_connect failed: 0x" << std::hex << err
            << ". Ensure ViGEmBus driver is installed.";
        m_lastError = oss.str();
        LOG_ERR(m_lastError);
        vigem_free(m_client);
        m_client = nullptr;
        return false;
    }

    LOG_INFO("ViGEmBus connected");
    return true;
}

bool VirtualGamepad::Connect() {
    if (!m_client) return false;

    m_target = vigem_target_x360_alloc();
    if (!m_target) {
        m_lastError = "vigem_target_x360_alloc failed";
        LOG_ERR(m_lastError);
        return false;
    }

    VIGEM_ERROR err = vigem_target_add(m_client, m_target);
    if (!VIGEM_SUCCESS(err)) {
        std::ostringstream oss;
        oss << "vigem_target_add failed: 0x" << std::hex << err;
        m_lastError = oss.str();
        LOG_ERR(m_lastError);
        vigem_target_free(m_target);
        m_target = nullptr;
        return false;
    }

    // Register vibration callback
    err = vigem_target_x360_register_notification(
        m_client, m_target, VibrationNotification, this);
    if (!VIGEM_SUCCESS(err)) {
        LOG_WARN("Could not register vibration callback: 0x" << std::hex << err);
    }

    m_connected.store(true);
    LOG_INFO("Virtual Xbox 360 controller connected (index "
        << vigem_target_get_index(m_target) << ")");
    return true;
}

void VirtualGamepad::Disconnect() {
    if (!m_connected.load()) return;
    m_connected.store(false);

    if (m_target) {
        vigem_target_x360_unregister_notification(m_target);
        vigem_target_remove(m_client, m_target);
        vigem_target_free(m_target);
        m_target = nullptr;
    }
    LOG_INFO("Virtual Xbox 360 controller disconnected");
}

void VirtualGamepad::Shutdown() {
    Disconnect();
    if (m_client) {
        vigem_disconnect(m_client);
        vigem_free(m_client);
        m_client = nullptr;
    }
}

bool VirtualGamepad::IsConnected() const {
    return m_connected.load();
}

bool VirtualGamepad::UpdateState(const XUSB_REPORT& report) {
    if (!m_connected.load() || !m_client || !m_target) return false;

    VIGEM_ERROR err = vigem_target_x360_update(m_client, m_target, report);
    if (!VIGEM_SUCCESS(err)) {
        LOG_WARN("vigem_target_x360_update failed: 0x" << std::hex << err);
        return false;
    }
    return true;
}

void VirtualGamepad::SetVibrationCallback(VibrationCallback cb) {
    m_vibCb = std::move(cb);
}

std::string VirtualGamepad::GetLastError() const {
    return m_lastError;
}

// static
void CALLBACK VirtualGamepad::VibrationNotification(
    PVIGEM_CLIENT /*client*/,
    PVIGEM_TARGET /*target*/,
    UCHAR largeMotor,
    UCHAR smallMotor,
    UCHAR /*ledNumber*/,
    LPVOID userData)
{
    auto* self = reinterpret_cast<VirtualGamepad*>(userData);
    if (self && self->m_vibCb)
        self->m_vibCb(largeMotor, smallMotor);
}

} // namespace wz
