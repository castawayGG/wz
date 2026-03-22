// test_mapping_engine.cpp — Unit tests for MappingEngine

#include <cassert>
#include <cstdio>
#include <unordered_map>
#include "core/mapping_engine.h"

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

static MappingConfig MakeTestConfig() {
    MappingConfig cfg;
    cfg.scrollDpadEnabled = true;
    cfg.walkModifierVK    = VK_MENU;

    cfg.keyBindings[VK_SPACE] = {XUSB_GAMEPAD_A, 0, HoldBehavior::Normal, 0};
    cfg.keyBindings['C']      = {XUSB_GAMEPAD_B, 0, HoldBehavior::Normal, 0};
    cfg.keyBindings[VK_SHIFT] = {XUSB_GAMEPAD_LEFT_THUMB, 0, HoldBehavior::Normal, 0};

    // LMB -> RT
    cfg.mouseBindings[0] = {0, 2, HoldBehavior::Normal, 0};
    // RMB -> LT
    cfg.mouseBindings[1] = {0, 1, HoldBehavior::Normal, 0};
    return cfg;
}

static void TestKeyToButton() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());

    std::unordered_map<USHORT, bool> state;
    state[VK_SPACE] = true;

    USHORT buttons = 0; BYTE lt = 0, rt = 0;
    eng.ApplyKeyState(state, buttons, lt, rt);
    EXPECT(buttons & XUSB_GAMEPAD_A);
    EXPECT(!(buttons & XUSB_GAMEPAD_B));
    printf("  TestKeyToButton: done\n");
}

static void TestKeyRelease() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());

    std::unordered_map<USHORT, bool> state;
    state[VK_SPACE] = false; // not held

    USHORT buttons = 0; BYTE lt = 0, rt = 0;
    eng.ApplyKeyState(state, buttons, lt, rt);
    EXPECT(!(buttons & XUSB_GAMEPAD_A));
    printf("  TestKeyRelease: done\n");
}

static void TestMouseLmbToRT() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());

    USHORT buttons = 0; BYTE lt = 0, rt = 0;
    eng.ApplyMouseButtons(true, false, false, false, false, buttons, lt, rt);
    EXPECT(rt == 255);
    EXPECT(lt == 0);
    printf("  TestMouseLmbToRT: done\n");
}

static void TestMouseRmbToLT() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());

    USHORT buttons = 0; BYTE lt = 0, rt = 0;
    eng.ApplyMouseButtons(false, true, false, false, false, buttons, lt, rt);
    EXPECT(lt == 255);
    EXPECT(rt == 0);
    printf("  TestMouseRmbToLT: done\n");
}

static void TestScrollDpadUp() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());
    EXPECT(eng.ProcessScroll(120) == XUSB_GAMEPAD_DPAD_UP);
    printf("  TestScrollDpadUp: done\n");
}

static void TestScrollDpadDown() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());
    EXPECT(eng.ProcessScroll(-120) == XUSB_GAMEPAD_DPAD_DOWN);
    printf("  TestScrollDpadDown: done\n");
}

static void TestScrollZero() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());
    EXPECT(eng.ProcessScroll(0) == 0);
    printf("  TestScrollZero: done\n");
}

static void TestScrollDisabled() {
    MappingEngine eng;
    MappingConfig cfg = MakeTestConfig();
    cfg.scrollDpadEnabled = false;
    eng.SetConfig(cfg);
    EXPECT(eng.ProcessScroll(120) == 0);
    printf("  TestScrollDisabled: done\n");
}

static void TestMultipleKeys() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());

    std::unordered_map<USHORT, bool> state;
    state[VK_SPACE] = true;
    state['C']      = true;

    USHORT buttons = 0; BYTE lt = 0, rt = 0;
    eng.ApplyKeyState(state, buttons, lt, rt);
    EXPECT(buttons & XUSB_GAMEPAD_A);
    EXPECT(buttons & XUSB_GAMEPAD_B);
    printf("  TestMultipleKeys: done\n");
}

static void TestUnboundKey() {
    MappingEngine eng;
    eng.SetConfig(MakeTestConfig());

    std::unordered_map<USHORT, bool> state;
    state['Z'] = true; // not bound

    USHORT buttons = 0; BYTE lt = 0, rt = 0;
    eng.ApplyKeyState(state, buttons, lt, rt);
    EXPECT(buttons == 0);
    printf("  TestUnboundKey: done\n");
}

int TestMappingEngineMain() {
    printf("=== MappingEngine Tests ===\n");
    TestKeyToButton();
    TestKeyRelease();
    TestMouseLmbToRT();
    TestMouseRmbToLT();
    TestScrollDpadUp();
    TestScrollDpadDown();
    TestScrollZero();
    TestScrollDisabled();
    TestMultipleKeys();
    TestUnboundKey();
    printf("MappingEngine: %d passed, %d failed\n\n", s_passed, s_failed);
    return s_failed;
}
