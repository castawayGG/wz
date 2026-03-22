// test_axis_processor.cpp — Unit tests for AxisProcessor
// Uses simple assertion macros; no external test framework required.

#include <cassert>
#include <cmath>
#include <cstdio>
#include "core/axis_processor.h"

using namespace wz;

static int s_passed = 0;
static int s_failed = 0;

#define EXPECT(cond) \
    do { if (!(cond)) { \
        printf("FAIL [%s:%d]: %s\n", __FILE__, __LINE__, #cond); \
        ++s_failed; \
    } else { \
        ++s_passed; \
    } } while(0)

#define EXPECT_NEAR(a, b, eps) EXPECT(std::fabs((a)-(b)) < (eps))

// ── ApplyCurve tests ──────────────────────────────────────────────────────

static void TestLinearCurve() {
    CurveConfig cfg; cfg.type = CurveType::Linear;
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.5f, cfg), 0.5f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(1.0f, cfg), 1.0f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.0f, cfg), 0.0f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(-0.5f, cfg), -0.5f, 1e-4f);
    printf("  TestLinearCurve: done\n");
}

static void TestQuadraticCurve() {
    CurveConfig cfg; cfg.type = CurveType::Quadratic;
    EXPECT_NEAR(AxisProcessor::ApplyCurve(1.0f, cfg), 1.0f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.0f, cfg), 0.0f, 1e-4f);
    // 0.5^2 = 0.25
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.5f, cfg), 0.25f, 1e-4f);
    // Negative: sign is preserved
    EXPECT_NEAR(AxisProcessor::ApplyCurve(-0.5f, cfg), -0.25f, 1e-4f);
    printf("  TestQuadraticCurve: done\n");
}

static void TestSCurve() {
    CurveConfig cfg; cfg.type = CurveType::SCurve;
    // S-curve: f(1) = 1, f(0) = 0, f(0.5) = 0.5
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.0f, cfg), 0.0f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(1.0f, cfg), 1.0f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.5f, cfg), 0.5f, 1e-4f);
    // Negative symmetry
    EXPECT_NEAR(AxisProcessor::ApplyCurve(-1.0f, cfg), -1.0f, 1e-4f);
    printf("  TestSCurve: done\n");
}

static void TestCurveClamp() {
    CurveConfig cfg; cfg.type = CurveType::Linear;
    // Values > 1.0 should be clamped to 1.0
    EXPECT_NEAR(AxisProcessor::ApplyCurve(2.0f, cfg), 1.0f, 1e-4f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(-2.0f, cfg), -1.0f, 1e-4f);
    printf("  TestCurveClamp: done\n");
}

// ── Dead zone tests ──────────────────────────────────────────────────────

static void TestCircularDeadZoneInner() {
    DeadZoneConfig cfg;
    cfg.type         = DeadZoneType::Circular;
    cfg.innerPercent = 20.0f;
    cfg.outerPercent = 100.0f;

    float x = 0.1f, y = 0.1f;
    // magnitude < inner, should zero out
    AxisProcessor::ApplyDeadZone(x, y, cfg);
    EXPECT_NEAR(x, 0.0f, 1e-4f);
    EXPECT_NEAR(y, 0.0f, 1e-4f);
    printf("  TestCircularDeadZoneInner: done\n");
}

static void TestCircularDeadZoneOuter() {
    DeadZoneConfig cfg;
    cfg.type         = DeadZoneType::Circular;
    cfg.innerPercent = 0.0f;
    cfg.outerPercent = 80.0f;

    float x = 0.9f, y = 0.0f;
    AxisProcessor::ApplyDeadZone(x, y, cfg);
    // magnitude > outer → clamped to full
    EXPECT_NEAR(x, 1.0f, 1e-3f);
    EXPECT_NEAR(y, 0.0f, 1e-3f);
    printf("  TestCircularDeadZoneOuter: done\n");
}

static void TestAxialDeadZone() {
    DeadZoneConfig cfg;
    cfg.type   = DeadZoneType::Axial;
    cfg.axialX = 10.0f;
    cfg.axialY = 10.0f;
    cfg.outerPercent = 100.0f;

    float x = 0.05f, y = 0.5f;
    AxisProcessor::ApplyDeadZone(x, y, cfg);
    // x < axialX → zeroed; y > axialY → scaled
    EXPECT_NEAR(x, 0.0f, 1e-4f);
    EXPECT(y > 0.0f);
    printf("  TestAxialDeadZone: done\n");
}

// ── ToShort tests ──────────────────────────────────────────────────────────

static void TestToShort() {
    EXPECT(AxisProcessor::ToShort(1.0f)  == 32767);
    EXPECT(AxisProcessor::ToShort(-1.0f) == -32767);
    EXPECT(AxisProcessor::ToShort(0.0f)  == 0);
    EXPECT(AxisProcessor::ToShort(2.0f)  == 32767);   // clamped
    EXPECT(AxisProcessor::ToShort(-2.0f) == -32767);  // clamped
    printf("  TestToShort: done\n");
}

// ── WASD ramp tests ───────────────────────────────────────────────────────

static void TestWasdRampUp() {
    AxisProcessor ap;
    WasdStickConfig wCfg;
    wCfg.rampUpMs = 100.0f;
    wCfg.diagonalNorm = true;
    ap.SetWasdConfig(wCfg);

    short lx, ly;
    // After 1 step of 10ms, should not be at full yet
    ap.ProcessWasd(false, false, false, true, false, 10.0, lx, ly);
    EXPECT(lx > 0 && lx < 32767);
    printf("  TestWasdRampUp: done\n");
}

static void TestWasdDiagonalNorm() {
    AxisProcessor ap;
    WasdStickConfig wCfg;
    wCfg.rampUpMs = 0.0f; // instant
    wCfg.diagonalNorm = true;
    ap.SetWasdConfig(wCfg);

    short lx, ly;
    ap.ProcessWasd(true, false, false, true, false, 1000.0, lx, ly);
    // Diagonal should be normalized (not exceed unit circle)
    float fx = lx / 32767.f;
    float fy = ly / 32767.f;
    float mag = std::sqrt(fx * fx + fy * fy);
    EXPECT(mag <= 1.01f); // allow tiny floating-point error
    printf("  TestWasdDiagonalNorm: done\n");
}

// ── BezierCurve test ──────────────────────────────────────────────────────

static void TestBezierCurve() {
    CurveConfig cfg;
    cfg.type = CurveType::CustomBezier;
    cfg.p1   = {0.42f, 0.0f};
    cfg.p2   = {0.58f, 1.0f};

    // Endpoints must map 0→0 and 1→1
    EXPECT_NEAR(AxisProcessor::ApplyCurve(0.0f, cfg), 0.0f, 1e-3f);
    EXPECT_NEAR(AxisProcessor::ApplyCurve(1.0f, cfg), 1.0f, 1e-3f);
    printf("  TestBezierCurve: done\n");
}

int TestAxisProcessorMain() {
    printf("=== AxisProcessor Tests ===\n");
    TestLinearCurve();
    TestQuadraticCurve();
    TestSCurve();
    TestCurveClamp();
    TestCircularDeadZoneInner();
    TestCircularDeadZoneOuter();
    TestAxialDeadZone();
    TestToShort();
    TestWasdRampUp();
    TestWasdDiagonalNorm();
    TestBezierCurve();
    printf("AxisProcessor: %d passed, %d failed\n\n", s_passed, s_failed);
    return s_failed;
}
