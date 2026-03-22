#include "import_export.h"
#include "../util/logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace wz {

// ── Serialization helpers ─────────────────────────────────────────────────

static json SerializeCurve(const CurveConfig& c) {
    json j;
    j["type"]     = static_cast<int>(c.type);
    j["exponent"] = c.exponent;
    j["p1"]       = {c.p1[0], c.p1[1]};
    j["p2"]       = {c.p2[0], c.p2[1]};
    return j;
}

static CurveConfig DeserializeCurve(const json& j) {
    CurveConfig c;
    c.type     = static_cast<CurveType>(j.value("type", 0));
    c.exponent = j.value("exponent", 2.0f);
    if (j.contains("p1") && j["p1"].is_array()) {
        c.p1 = {j["p1"][0].get<float>(), j["p1"][1].get<float>()};
    }
    if (j.contains("p2") && j["p2"].is_array()) {
        c.p2 = {j["p2"][0].get<float>(), j["p2"][1].get<float>()};
    }
    return c;
}

static json SerializeDeadZone(const DeadZoneConfig& dz) {
    json j;
    j["innerPercent"] = dz.innerPercent;
    j["outerPercent"] = dz.outerPercent;
    j["type"]         = static_cast<int>(dz.type);
    j["axialX"]       = dz.axialX;
    j["axialY"]       = dz.axialY;
    return j;
}

static DeadZoneConfig DeserializeDeadZone(const json& j) {
    DeadZoneConfig dz;
    dz.innerPercent = j.value("innerPercent", 5.0f);
    dz.outerPercent = j.value("outerPercent", 95.0f);
    dz.type         = static_cast<DeadZoneType>(j.value("type", 0));
    dz.axialX       = j.value("axialX", 5.0f);
    dz.axialY       = j.value("axialY", 5.0f);
    return dz;
}

static json SerializeMouseConfig(const MouseStickConfig& m) {
    json j;
    j["sensitivityX"]       = m.sensitivityX;
    j["sensitivityY"]       = m.sensitivityY;
    j["adsSensMultiplier"]  = m.adsSensMultiplier;
    j["smoothingAlpha"]     = m.smoothingAlpha;
    j["jitterThreshold"]    = m.jitterThreshold;
    j["autoCenterDecayMs"]  = m.autoCenterDecayMs;
    j["curve"]              = SerializeCurve(m.curve);
    j["deadZone"]           = SerializeDeadZone(m.deadZone);
    return j;
}

static MouseStickConfig DeserializeMouseConfig(const json& j) {
    MouseStickConfig m;
    m.sensitivityX      = j.value("sensitivityX",      15.0f);
    m.sensitivityY      = j.value("sensitivityY",      13.0f);
    m.adsSensMultiplier = j.value("adsSensMultiplier", 0.6f);
    m.smoothingAlpha    = j.value("smoothingAlpha",    0.3f);
    m.jitterThreshold   = j.value("jitterThreshold",   2.0f);
    m.autoCenterDecayMs = j.value("autoCenterDecayMs", 50.0f);
    if (j.contains("curve"))    m.curve    = DeserializeCurve(j["curve"]);
    if (j.contains("deadZone")) m.deadZone = DeserializeDeadZone(j["deadZone"]);
    return m;
}

static json SerializeWasdConfig(const WasdStickConfig& w) {
    json j;
    j["rampUpMs"]       = w.rampUpMs;
    j["walkMultiplier"] = w.walkMultiplier;
    j["diagonalNorm"]   = w.diagonalNorm;
    j["deadZone"]       = SerializeDeadZone(w.deadZone);
    return j;
}

static WasdStickConfig DeserializeWasdConfig(const json& j) {
    WasdStickConfig w;
    w.rampUpMs       = j.value("rampUpMs",       30.0f);
    w.walkMultiplier = j.value("walkMultiplier",  0.5f);
    w.diagonalNorm   = j.value("diagonalNorm",    true);
    if (j.contains("deadZone")) w.deadZone = DeserializeDeadZone(j["deadZone"]);
    return w;
}

static json SerializeButtonBinding(const ButtonBinding& b) {
    json j;
    j["gamepadButton"] = b.gamepadButton;
    j["triggerAxis"]   = b.triggerAxis;
    j["holdMode"]      = static_cast<int>(b.holdMode);
    j["turboIntervalMs"] = b.turboIntervalMs;
    return j;
}

static ButtonBinding DeserializeButtonBinding(const json& j) {
    ButtonBinding b;
    b.gamepadButton   = j.value("gamepadButton",   (unsigned short)0);
    b.triggerAxis     = j.value("triggerAxis",     (unsigned char)0);
    b.holdMode        = static_cast<HoldBehavior>(j.value("holdMode", 0));
    b.turboIntervalMs = j.value("turboIntervalMs", 50u);
    return b;
}

static json SerializeMappingConfig(const MappingConfig& cfg) {
    json j;
    j["scrollDpadEnabled"] = cfg.scrollDpadEnabled;
    j["walkModifierVK"]    = cfg.walkModifierVK;
    for (auto& [vk, bind] : cfg.keyBindings)
        j["keyBindings"][std::to_string(vk)] = SerializeButtonBinding(bind);
    for (auto& [btn, bind] : cfg.mouseBindings)
        j["mouseBindings"][std::to_string(btn)] = SerializeButtonBinding(bind);
    return j;
}

static MappingConfig DeserializeMappingConfig(const json& j) {
    MappingConfig cfg;
    cfg.scrollDpadEnabled = j.value("scrollDpadEnabled", true);
    cfg.walkModifierVK    = j.value("walkModifierVK",    (unsigned short)VK_MENU);
    if (j.contains("keyBindings")) {
        for (auto& [k, v] : j["keyBindings"].items())
            cfg.keyBindings[static_cast<USHORT>(std::stoi(k))] = DeserializeButtonBinding(v);
    }
    if (j.contains("mouseBindings")) {
        for (auto& [k, v] : j["mouseBindings"].items())
            cfg.mouseBindings[std::stoi(k)] = DeserializeButtonBinding(v);
    }
    return cfg;
}

static json SerializeMacroStep(const MacroStep& s) {
    return json{
        {"button", s.button}, {"triggerAxis", s.triggerAxis},
        {"triggerValue", s.triggerValue}, {"press", s.press}, {"delayMs", s.delayMs}
    };
}

static MacroStep DeserializeMacroStep(const json& j) {
    MacroStep s;
    s.button       = j.value("button",       (unsigned short)0);
    s.triggerAxis  = j.value("triggerAxis",  (unsigned char)0);
    s.triggerValue = j.value("triggerValue", (unsigned char)255);
    s.press        = j.value("press",        true);
    s.delayMs      = j.value("delayMs",      50u);
    return s;
}

static json SerializeMacroDef(const MacroDef& m) {
    json j;
    j["name"] = m.name;
    j["loop"] = m.loop;
    json steps = json::array();
    for (auto& s : m.steps) steps.push_back(SerializeMacroStep(s));
    j["steps"] = steps;
    return j;
}

static MacroDef DeserializeMacroDef(const json& j) {
    MacroDef m;
    m.name = j.value("name", "");
    m.loop = j.value("loop", false);
    if (j.contains("steps")) {
        for (auto& sj : j["steps"]) m.steps.push_back(DeserializeMacroStep(sj));
    }
    return m;
}

// ── Public API ────────────────────────────────────────────────────────────

std::string ImportExport::ToJsonString(const Profile& p) {
    json j;
    j["name"]        = p.name;
    j["version"]     = p.version;
    j["baseMapping"] = SerializeMappingConfig(p.baseMapping);
    j["mouseConfig"] = SerializeMouseConfig(p.mouseConfig);
    j["wasdConfig"]  = SerializeWasdConfig(p.wasdConfig);
    j["toggleModeVK"]    = p.toggleModeVK;
    j["cycleProfileVK"]  = p.cycleProfileVK;
    j["toggleLayerVK"]   = p.toggleLayerVK;
    j["emergencyStopVK"] = p.emergencyStopVK;

    json macros = json::array();
    for (auto& m : p.macros) macros.push_back(SerializeMacroDef(m));
    j["macros"] = macros;

    json layers = json::array();
    for (auto& l : p.layers) {
        json lj;
        lj["name"]                  = l.name;
        lj["activatorVK"]           = l.activatorVK;
        lj["sensitivityMultiplier"] = l.sensitivityMultiplier;
        lj["config"]                = SerializeMappingConfig(l.config);
        layers.push_back(lj);
    }
    j["layers"] = layers;

    return j.dump(4);
}

bool ImportExport::FromJsonString(const std::string& jsonStr, Profile& out) {
    try {
        json j = json::parse(jsonStr);
        out.name    = j.value("name", "unknown");
        out.version = j.value("version", "1.0");
        if (j.contains("baseMapping")) out.baseMapping = DeserializeMappingConfig(j["baseMapping"]);
        if (j.contains("mouseConfig")) out.mouseConfig = DeserializeMouseConfig(j["mouseConfig"]);
        if (j.contains("wasdConfig"))  out.wasdConfig  = DeserializeWasdConfig(j["wasdConfig"]);
        out.toggleModeVK    = j.value("toggleModeVK",    VK_F9);
        out.cycleProfileVK  = j.value("cycleProfileVK",  VK_F10);
        out.toggleLayerVK   = j.value("toggleLayerVK",   VK_F11);
        out.emergencyStopVK = j.value("emergencyStopVK", VK_F12);

        if (j.contains("macros")) {
            for (auto& mj : j["macros"]) out.macros.push_back(DeserializeMacroDef(mj));
        }
        if (j.contains("layers")) {
            for (auto& lj : j["layers"]) {
                LayerDef l;
                l.name                  = lj.value("name", "");
                l.activatorVK           = lj.value("activatorVK", (unsigned short)0);
                l.sensitivityMultiplier = lj.value("sensitivityMultiplier", 1.0f);
                if (lj.contains("config")) l.config = DeserializeMappingConfig(lj["config"]);
                out.layers.push_back(l);
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_ERR("JSON parse error: " << e.what());
        return false;
    }
}

bool ImportExport::SaveToFile(const Profile& profile, const std::string& path) {
    std::ofstream f(path);
    if (!f) {
        LOG_ERR("Cannot open file for writing: " << path);
        return false;
    }
    f << ToJsonString(profile);
    return true;
}

bool ImportExport::LoadFromFile(const std::string& path, Profile& out) {
    std::ifstream f(path);
    if (!f) {
        LOG_ERR("Cannot open profile: " << path);
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    return FromJsonString(content, out);
}

} // namespace wz
