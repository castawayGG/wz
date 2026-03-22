// test_profile_manager.cpp — Unit tests for ProfileManager

#include <cassert>
#include <cstdio>
#include <string>
#include "profile/profile_manager.h"
#include "profile/import_export.h"

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

static void TestDefaultProfiles() {
    ProfileManager mgr;
    auto& profiles = mgr.GetProfiles();
    EXPECT(!profiles.empty());
    // Should have warzone_default
    bool hasDefault = false;
    for (auto& p : profiles) if (p.name == "warzone_default") hasDefault = true;
    EXPECT(hasDefault);
    printf("  TestDefaultProfiles: done\n");
}

static void TestActiveProfile() {
    ProfileManager mgr;
    EXPECT(mgr.GetActiveProfile() != nullptr);
    EXPECT(mgr.GetActiveProfileName() == "warzone_default");
    printf("  TestActiveProfile: done\n");
}

static void TestSetActiveProfile() {
    ProfileManager mgr;
    bool result = mgr.SetActiveProfile("warzone_sniper");
    EXPECT(result);
    EXPECT(mgr.GetActiveProfileName() == "warzone_sniper");
    printf("  TestSetActiveProfile: done\n");
}

static void TestSetActiveProfileInvalid() {
    ProfileManager mgr;
    bool result = mgr.SetActiveProfile("nonexistent_profile");
    EXPECT(!result);
    EXPECT(mgr.GetActiveProfileName() == "warzone_default");
    printf("  TestSetActiveProfileInvalid: done\n");
}

static void TestCycleNext() {
    ProfileManager mgr;
    std::string first = mgr.GetActiveProfileName();
    mgr.CycleNext();
    EXPECT(mgr.GetActiveProfileName() != first);
    printf("  TestCycleNext: done\n");
}

static void TestAddProfile() {
    ProfileManager mgr;
    Profile p;
    p.name = "my_custom_profile";
    mgr.AddProfile(p);
    EXPECT(mgr.GetProfile("my_custom_profile") != nullptr);
    printf("  TestAddProfile: done\n");
}

static void TestRemoveProfile() {
    ProfileManager mgr;
    Profile p;
    p.name = "remove_me";
    mgr.AddProfile(p);
    bool r = mgr.RemoveProfile("remove_me");
    EXPECT(r);
    EXPECT(mgr.GetProfile("remove_me") == nullptr);
    printf("  TestRemoveProfile: done\n");
}

static void TestRemoveNonexistent() {
    ProfileManager mgr;
    bool r = mgr.RemoveProfile("not_there");
    EXPECT(!r);
    printf("  TestRemoveNonexistent: done\n");
}

static void TestChangeCallback() {
    ProfileManager mgr;
    std::string changed;
    mgr.SetChangeCallback([&](const Profile& p){ changed = p.name; });
    mgr.SetActiveProfile("warzone_sniper");
    EXPECT(changed == "warzone_sniper");
    printf("  TestChangeCallback: done\n");
}

// ── Import/Export tests ─────────────────────────────────────────────────

static void TestJsonRoundTrip() {
    Profile original = ProfileManager::MakeWarzoneDefault();
    std::string json = ImportExport::ToJsonString(original);
    EXPECT(!json.empty());

    Profile loaded;
    bool ok = ImportExport::FromJsonString(json, loaded);
    EXPECT(ok);
    EXPECT(loaded.name == original.name);
    EXPECT(loaded.version == original.version);
    EXPECT(loaded.mouseConfig.sensitivityX == original.mouseConfig.sensitivityX);
    EXPECT(loaded.mouseConfig.sensitivityY == original.mouseConfig.sensitivityY);
    EXPECT(loaded.mouseConfig.smoothingAlpha == original.mouseConfig.smoothingAlpha);
    EXPECT(loaded.wasdConfig.rampUpMs == original.wasdConfig.rampUpMs);
    printf("  TestJsonRoundTrip: done\n");
}

static void TestJsonInvalidInput() {
    Profile p;
    bool ok = ImportExport::FromJsonString("not valid json{{{", p);
    EXPECT(!ok);
    printf("  TestJsonInvalidInput: done\n");
}

static void TestWarzoneDefaultSensitivity() {
    Profile p = ProfileManager::MakeWarzoneDefault();
    EXPECT(p.mouseConfig.sensitivityX > 0.f);
    EXPECT(p.mouseConfig.sensitivityY > 0.f);
    EXPECT(p.mouseConfig.smoothingAlpha >= 0.f && p.mouseConfig.smoothingAlpha <= 1.f);
    EXPECT(p.mouseConfig.deadZone.innerPercent >= 0.f);
    EXPECT(p.mouseConfig.deadZone.outerPercent > p.mouseConfig.deadZone.innerPercent);
    printf("  TestWarzoneDefaultSensitivity: done\n");
}

static void TestWarzoneDefaultHasMacros() {
    Profile p = ProfileManager::MakeWarzoneDefault();
    EXPECT(!p.macros.empty());
    bool hasSlideCancel = false;
    for (auto& m : p.macros) if (m.name == "slide_cancel") hasSlideCancel = true;
    EXPECT(hasSlideCancel);
    printf("  TestWarzoneDefaultHasMacros: done\n");
}

static void TestSniperLowerSensitivity() {
    Profile def    = ProfileManager::MakeWarzoneDefault();
    Profile sniper = ProfileManager::MakeWarzoneSniper();
    EXPECT(sniper.mouseConfig.sensitivityX < def.mouseConfig.sensitivityX);
    EXPECT(sniper.mouseConfig.sensitivityY < def.mouseConfig.sensitivityY);
    printf("  TestSniperLowerSensitivity: done\n");
}

static void TestAggressiveHigherSensitivity() {
    Profile def  = ProfileManager::MakeWarzoneDefault();
    Profile aggr = ProfileManager::MakeWarzoneAggressive();
    EXPECT(aggr.mouseConfig.sensitivityX > def.mouseConfig.sensitivityX);
    printf("  TestAggressiveHigherSensitivity: done\n");
}

int TestProfileManagerMain() {
    printf("=== ProfileManager Tests ===\n");
    TestDefaultProfiles();
    TestActiveProfile();
    TestSetActiveProfile();
    TestSetActiveProfileInvalid();
    TestCycleNext();
    TestAddProfile();
    TestRemoveProfile();
    TestRemoveNonexistent();
    TestChangeCallback();
    TestJsonRoundTrip();
    TestJsonInvalidInput();
    TestWarzoneDefaultSensitivity();
    TestWarzoneDefaultHasMacros();
    TestSniperLowerSensitivity();
    TestAggressiveHigherSensitivity();
    printf("ProfileManager: %d passed, %d failed\n\n", s_passed, s_failed);
    return s_failed;
}
