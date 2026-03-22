#pragma once
#include <cmath>
#include <algorithm>
#include <array>
#include <vector>

namespace wz {

enum class CurveType {
    Linear,
    Quadratic,
    Cubic,
    SCurve,     // Sigmoid-based
    CustomBezier
};

enum class DeadZoneType {
    Circular,   // Standard circular inner dead zone
    Axial,      // Per-axis dead zones
    Outer       // Anti-overextension
};

struct DeadZoneConfig {
    float innerPercent{5.0f};  // 0-100: values inside are zeroed
    float outerPercent{95.0f}; // 0-100: values outside are clamped to full
    DeadZoneType type{DeadZoneType::Circular};
    float axialX{5.0f};        // used for Axial mode
    float axialY{5.0f};
};

struct CurveConfig {
    CurveType type{CurveType::SCurve};
    float exponent{2.0f};          // used for Quadratic/Cubic
    // Bezier control points (normalized 0-1)
    std::array<float, 2> p1{0.25f, 0.1f};
    std::array<float, 2> p2{0.75f, 0.9f};
};

struct MouseStickConfig {
    float sensitivityX{15.0f};
    float sensitivityY{13.0f};
    float adsSensMultiplier{0.6f};
    float smoothingAlpha{0.3f};    // EMA alpha: 0=no smoothing, 1=instant
    float jitterThreshold{2.0f};   // pixels below this are ignored
    float autoCenterDecayMs{50.0f};// ms until stick returns to center
    CurveConfig curve;
    DeadZoneConfig deadZone;
};

struct WasdStickConfig {
    float rampUpMs{30.0f};     // ms to reach full deflection
    float walkMultiplier{0.5f};// deflection when walk modifier held
    bool  diagonalNorm{true};  // normalize diagonal to unit circle
    DeadZoneConfig deadZone;
};

// AxisProcessor: converts raw input values to normalized stick values [-1,1]
class AxisProcessor {
public:
    explicit AxisProcessor(MouseStickConfig mCfg = {}, WasdStickConfig wCfg = {});

    void SetMouseConfig(const MouseStickConfig& cfg);
    void SetWasdConfig(const WasdStickConfig& cfg);

    // Process mouse delta -> right stick output in [-32767, 32767]
    // Call each input frame with raw dx/dy from mouse
    void ProcessMouseDelta(float dx, float dy, double elapsedMs,
                           short& outRX, short& outRY);

    // Process WASD keys -> left stick output in [-32767, 32767]
    // kUp/kDown/kLeft/kRight: 1.0 if key held, 0.0 otherwise
    // walkModifier: true if walk key is held
    void ProcessWasd(bool up, bool down, bool left, bool right,
                     bool walkModifier, double elapsedMs,
                     short& outLX, short& outLY);

    // Apply sensitivity curve to normalized value [-1,1]
    static float ApplyCurve(float v, const CurveConfig& cfg);

    // Apply dead zones; input/output: normalized [-1,1] pair
    static void ApplyDeadZone(float& x, float& y, const DeadZoneConfig& cfg);

    // Clamp to short range
    static short ToShort(float v);

private:
    MouseStickConfig m_mCfg;
    WasdStickConfig  m_wCfg;

    // EMA smoothed mouse values
    float m_smoothX{0.f}, m_smoothY{0.f};
    double m_lastMouseMs{0.0};

    // WASD ramp state
    float m_rampX{0.f}, m_rampY{0.f};
};

} // namespace wz
