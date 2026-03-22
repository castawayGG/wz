#include "layer_manager.h"

namespace wz {

LayerManager::LayerManager() {
    for (auto& m : m_sensitivityMult) m = 1.0f;
}

void LayerManager::SetBaseLayer(const MappingConfig& cfg) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_layers[0].name   = "base";
    m_layers[0].config = cfg;
    m_layers[0].active = true;
}

void LayerManager::SetLayer(int index, const std::string& name, const MappingConfig& cfg) {
    if (index < 1 || index >= kMaxLayers) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_layers[index].name   = name;
    m_layers[index].config = cfg;
}

void LayerManager::ActivateLayer(int index) {
    if (index < 0 || index >= kMaxLayers) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_layers[index].active = true;
}

void LayerManager::DeactivateLayer(int index) {
    if (index < 0 || index >= kMaxLayers) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_layers[index].active = false;
}

void LayerManager::DeactivateAll() {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (int i = 1; i < kMaxLayers; ++i)
        m_layers[i].active = false;
}

MappingConfig LayerManager::GetEffectiveConfig() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    // Start with base
    MappingConfig result = m_layers[0].config;

    // Merge higher layers (they override)
    for (int i = 1; i < kMaxLayers; ++i) {
        if (!m_layers[i].active) continue;
        for (auto& [vk, bind] : m_layers[i].config.keyBindings)
            result.keyBindings[vk] = bind;
        for (auto& [btn, bind] : m_layers[i].config.mouseBindings)
            result.mouseBindings[btn] = bind;
    }
    return result;
}

void LayerManager::SetLayerSensitivity(int index, float multiplier) {
    if (index < 0 || index >= kMaxLayers) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_sensitivityMult[index] = multiplier;
}

float LayerManager::GetEffectiveSensitivityMultiplier() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    float mult = 1.0f;
    for (int i = 0; i < kMaxLayers; ++i) {
        if (m_layers[i].active) mult *= m_sensitivityMult[i];
    }
    return mult;
}

int LayerManager::GetActiveLayerIndex() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (int i = kMaxLayers - 1; i >= 0; --i)
        if (m_layers[i].active) return i;
    return 0;
}

} // namespace wz
