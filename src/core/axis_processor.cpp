#include "axis_processor.h"
#include "../util/timer.h"
#include <cstring>
#include <algorithm>

namespace wz {

AxisProcessor::AxisProcessor(MouseStickConfig mCfg, WasdStickConfig wCfg)
    : m_mCfg(std::move(mCfg)), m_wCfg(std::move(wCfg)) {}

void AxisProcessor::SetMouseConfig(const MouseStickConfig& cfg) { m_mCfg = cfg; }
void AxisProcessor::SetWasdConfig(const WasdStickConfig& cfg)   { m_wCfg = cfg; }

// ── Curve application ──────────────────────────────────────────────────────

float AxisProcessor::ApplyCurve(float v, const CurveConfig& cfg) {
    float sign = (v < 0.f) ? -1.f : 1.f;
    float abs  = std::fabs(v);
    abs = std::min(abs, 1.0f);

    float out = abs;
    switch (cfg.type) {
    case CurveType::Linear:
        out = abs;
        break;
    case CurveType::Quadratic:
        out = abs * abs;
        break;
    case CurveType::Cubic:
        out = abs * abs * abs;
        break;
    case CurveType::SCurve: {
        // Sigmoid mapped to [0,1]: f(x) = x^2 * (3 - 2x)   (smoothstep)
        out = abs * abs * (3.f - 2.f * abs);
        break;
    }
    case CurveType::CustomBezier: {
        // Cubic Bezier interpolation with P0=(0,0), P1, P2, P3=(1,1)
        float t = abs;
        // Newton-Raphson to find t from x
        for (int i = 0; i < 8; ++i) {
            float cx = 3.f * cfg.p1[0];
            float bx = 3.f * (cfg.p2[0] - cfg.p1[0]) - cx;
            float ax = 1.f - cx - bx;
            float x  = ((ax * t + bx) * t + cx) * t;
            float dx = (3.f * ax * t + 2.f * bx) * t + cx;
            if (std::fabs(dx) < 1e-6f) break;
            t -= (x - abs) / dx;
            t = std::max(0.f, std::min(1.f, t));
        }
        float cy = 3.f * cfg.p1[1];
        float by = 3.f * (cfg.p2[1] - cfg.p1[1]) - cy;
        float ay = 1.f - cy - by;
        out = ((ay * t + by) * t + cy) * t;
        break;
    }
    }
    return sign * std::min(out, 1.0f);
}

// ── Dead zone application ──────────────────────────────────────────────────

void AxisProcessor::ApplyDeadZone(float& x, float& y, const DeadZoneConfig& cfg) {
    float inner = cfg.innerPercent / 100.f;
    float outer = cfg.outerPercent / 100.f;

    if (cfg.type == DeadZoneType::Axial) {
        float ix = cfg.axialX / 100.f;
        float iy = cfg.axialY / 100.f;
        auto applyAxis = [](float v, float inner, float outer) -> float {
            float sign = (v < 0.f) ? -1.f : 1.f;
            float abs  = std::fabs(v);
            if (abs < inner) return 0.f;
            if (abs > outer) return sign;
            return sign * (abs - inner) / (outer - inner);
        };
        x = applyAxis(x, ix, outer);
        y = applyAxis(y, iy, outer);
        return;
    }

    // Circular
    float len = std::sqrt(x * x + y * y);
    if (len < 1e-6f) { x = y = 0.f; return; }

    if (len <= inner) { x = y = 0.f; return; }

    float scaled = (len - inner) / (outer - inner);
    scaled = std::max(0.f, std::min(1.f, scaled));
    float nx = (x / len) * scaled;
    float ny = (y / len) * scaled;
    x = nx;
    y = ny;
}

short AxisProcessor::ToShort(float v) {
    v = std::max(-1.f, std::min(1.f, v));
    return static_cast<short>(v * 32767.f);
}

// ── Mouse → Right Stick ───────────────────────────────────────────────────

void AxisProcessor::ProcessMouseDelta(float dx, float dy, double elapsedMs,
                                       short& outRX, short& outRY) {
    // Jitter filter
    if (std::fabs(dx) < m_mCfg.jitterThreshold) dx = 0.f;
    if (std::fabs(dy) < m_mCfg.jitterThreshold) dy = 0.f;

    bool hasInput = (dx != 0.f || dy != 0.f);

    // Apply sensitivity
    float nx = dx * m_mCfg.sensitivityX / 1000.f;
    float ny = dy * m_mCfg.sensitivityY / 1000.f;

    // EMA smoothing
    float alpha = m_mCfg.smoothingAlpha;
    m_smoothX = alpha * nx + (1.f - alpha) * m_smoothX;
    m_smoothY = alpha * ny + (1.f - alpha) * m_smoothY;

    if (!hasInput) {
        // Auto-center: decay toward zero
        double now = Timer::NowMs();
        double decayFrac = (now - m_lastMouseMs) / m_mCfg.autoCenterDecayMs;
        if (decayFrac >= 1.0) {
            m_smoothX = m_smoothY = 0.f;
        } else {
            float factor = 1.f - static_cast<float>(decayFrac);
            m_smoothX *= factor;
            m_smoothY *= factor;
        }
    } else {
        m_lastMouseMs = Timer::NowMs();
    }

    float rx = m_smoothX;
    float ry = m_smoothY;

    // Clamp to unit circle
    float len = std::sqrt(rx * rx + ry * ry);
    if (len > 1.f) { rx /= len; ry /= len; }

    // Apply curve
    rx = ApplyCurve(rx, m_mCfg.curve);
    ry = ApplyCurve(ry, m_mCfg.curve);

    // Dead zone
    ApplyDeadZone(rx, ry, m_mCfg.deadZone);

    outRX = ToShort(rx);
    outRY = ToShort(-ry); // Y is inverted on gamepad
}

// ── WASD → Left Stick ─────────────────────────────────────────────────────

void AxisProcessor::ProcessWasd(bool up, bool down, bool left, bool right,
                                  bool walkModifier, double elapsedMs,
                                  short& outLX, short& outLY) {
    // Target values
    float tx = 0.f, ty = 0.f;
    if (left)  tx -= 1.f;
    if (right) tx += 1.f;
    if (up)    ty += 1.f;
    if (down)  ty -= 1.f;

    // Diagonal normalization
    if (m_wCfg.diagonalNorm) {
        float len = std::sqrt(tx * tx + ty * ty);
        if (len > 1.f) { tx /= len; ty /= len; }
    }

    // Walk modifier
    if (walkModifier) { tx *= m_wCfg.walkMultiplier; ty *= m_wCfg.walkMultiplier; }

    // Ramp up
    float rampStep = static_cast<float>(elapsedMs) / m_wCfg.rampUpMs;
    auto ramp = [&](float& cur, float target) {
        if (target > cur) cur = std::min(cur + rampStep, target);
        else              cur = std::max(cur - rampStep, target);
    };
    ramp(m_rampX, tx);
    ramp(m_rampY, ty);

    outLX = ToShort(m_rampX);
    outLY = ToShort(m_rampY);
}

} // namespace wz
