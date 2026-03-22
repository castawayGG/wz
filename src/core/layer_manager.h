#pragma once
#include "mapping_engine.h"
#include <array>
#include <mutex>
#include <string>

namespace wz {

static constexpr int kMaxLayers = 4;

struct Layer {
    std::string  name;
    MappingConfig config;   // Overrides for this layer
    bool         active{false};
};

class LayerManager {
public:
    LayerManager();

    // Set base layer (always active)
    void SetBaseLayer(const MappingConfig& cfg);

    // Configure a shift layer (index 1-3)
    void SetLayer(int index, const std::string& name, const MappingConfig& cfg);

    // Activate / deactivate a shift layer
    void ActivateLayer(int index);
    void DeactivateLayer(int index);
    void DeactivateAll();

    // Get the effective (merged) mapping config
    // Higher index layers override lower ones
    MappingConfig GetEffectiveConfig() const;

    // Per-layer sensitivity multiplier
    void SetLayerSensitivity(int index, float multiplier);
    float GetEffectiveSensitivityMultiplier() const;

    int  GetActiveLayerIndex() const;

private:
    std::array<Layer, kMaxLayers>  m_layers;
    std::array<float, kMaxLayers>  m_sensitivityMult;
    mutable std::mutex             m_mutex;
};

} // namespace wz
