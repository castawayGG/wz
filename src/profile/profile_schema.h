#pragma once
#include "../core/mapping_engine.h"
#include "../core/axis_processor.h"
#include "../core/macro_engine.h"
#include <string>
#include <vector>

namespace wz {

struct LayerDef {
    std::string   name;
    USHORT        activatorVK{0}; // VK that activates this layer when held
    MappingConfig config;
    float         sensitivityMultiplier{1.0f};
};

struct Profile {
    std::string       name;
    std::string       version{"1.0"};
    MappingConfig     baseMapping;
    MouseStickConfig  mouseConfig;
    WasdStickConfig   wasdConfig;
    std::vector<LayerDef>  layers;
    std::vector<MacroDef>  macros;

    // Hotkeys (VK codes)
    UINT toggleModeVK{VK_F9};
    UINT cycleProfileVK{VK_F10};
    UINT toggleLayerVK{VK_F11};
    UINT emergencyStopVK{VK_F12};
};

} // namespace wz
