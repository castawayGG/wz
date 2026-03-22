// test_macro_engine.cpp — Unit tests for MacroEngine

#include <cassert>
#include <cstdio>
#include <vector>
#include <thread>
#include <chrono>
#include "core/macro_engine.h"

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

static void TestPrebuiltSlideCancel() {
    MacroDef def = MacroEngine::MakeSlideCancel();
    EXPECT(def.name == "slide_cancel");
    EXPECT(!def.loop);
    EXPECT(!def.steps.empty());
    // First step should press B
    EXPECT(def.steps[0].button == XUSB_GAMEPAD_B);
    EXPECT(def.steps[0].press == true);
    printf("  TestPrebuiltSlideCancel: done\n");
}

static void TestPrebuiltTacticalSprint() {
    MacroDef def = MacroEngine::MakeTacticalSprint();
    EXPECT(def.name == "tactical_sprint");
    EXPECT(!def.loop);
    // Should involve LEFT_THUMB double-tap
    bool hasLS = false;
    for (auto& s : def.steps) if (s.button == XUSB_GAMEPAD_LEFT_THUMB) hasLS = true;
    EXPECT(hasLS);
    printf("  TestPrebuiltTacticalSprint: done\n");
}

static void TestPrebuiltBunnyHop() {
    MacroDef def = MacroEngine::MakeBunnyHop();
    EXPECT(def.name == "bunny_hop");
    EXPECT(def.loop); // bunny hop repeats
    bool hasA = false;
    for (auto& s : def.steps) if (s.button == XUSB_GAMEPAD_A) hasA = true;
    EXPECT(hasA);
    printf("  TestPrebuiltBunnyHop: done\n");
}

static void TestAddAndPlay() {
    std::vector<std::pair<USHORT, BYTE>> outputs;

    MacroEngine eng([&](USHORT btn, BYTE lt, BYTE rt) {
        outputs.push_back({btn, rt});
    });

    MacroDef def;
    def.name = "test_macro";
    def.loop = false;
    def.steps.push_back({XUSB_GAMEPAD_A, 0, 255, true,  10});
    def.steps.push_back({XUSB_GAMEPAD_A, 0, 255, false, 10});

    eng.AddMacro(def);
    eng.Play("test_macro");

    // Wait for playback to finish
    int tries = 0;
    while (eng.IsPlaying() && tries++ < 50)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT(!eng.IsPlaying());
    // Should have received at least one output with A pressed
    bool gotA = false;
    for (auto& [btn, rt] : outputs) if (btn == XUSB_GAMEPAD_A) gotA = true;
    EXPECT(gotA);
    printf("  TestAddAndPlay: done\n");
}

static void TestPlayNonexistent() {
    MacroEngine eng([](USHORT, BYTE, BYTE){});
    bool result = eng.Play("doesnt_exist");
    EXPECT(!result);
    printf("  TestPlayNonexistent: done\n");
}

static void TestRemoveMacro() {
    MacroEngine eng([](USHORT, BYTE, BYTE){});
    MacroDef def; def.name = "remove_me"; def.loop = false;
    eng.AddMacro(def);
    eng.RemoveMacro("remove_me");
    bool result = eng.Play("remove_me");
    EXPECT(!result);
    printf("  TestRemoveMacro: done\n");
}

static void TestStopMacro() {
    MacroEngine eng([](USHORT, BYTE, BYTE){});

    MacroDef def = MacroEngine::MakeBunnyHop(); // looping
    eng.AddMacro(def);
    eng.Play(def.name);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    eng.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT(!eng.IsPlaying());
    printf("  TestStopMacro: done\n");
}

int TestMacroEngineMain() {
    printf("=== MacroEngine Tests ===\n");
    TestPrebuiltSlideCancel();
    TestPrebuiltTacticalSprint();
    TestPrebuiltBunnyHop();
    TestAddAndPlay();
    TestPlayNonexistent();
    TestRemoveMacro();
    TestStopMacro();
    printf("MacroEngine: %d passed, %d failed\n\n", s_passed, s_failed);
    return s_failed;
}
