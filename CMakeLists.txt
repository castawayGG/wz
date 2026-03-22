cmake_minimum_required(VERSION 3.20)
project(wz_gamepad VERSION 1.0.0 LANGUAGES CXX)

# ── C++17 ──────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Windows only ──────────────────────────────────────────────────────────
if(NOT WIN32)
    message(FATAL_ERROR "WZ Virtual Gamepad targets Windows only.")
endif()

# ── Options ───────────────────────────────────────────────────────────────
option(BUILD_TESTING "Build unit tests" OFF)
option(WZ_VIGEM_LIB "Path to ViGEmClient.lib (optional; set if not in system paths)" "")

# ── External dependencies ─────────────────────────────────────────────────
add_subdirectory(external)

# ── Source files ──────────────────────────────────────────────────────────
set(WZ_SOURCES
    src/main.cpp
    src/app/app_controller.cpp
    src/app/tray_icon.cpp
    src/app/osd_overlay.cpp
    src/core/input_engine.cpp
    src/core/virtual_gamepad.cpp
    src/core/mapping_engine.cpp
    src/core/axis_processor.cpp
    src/core/macro_engine.cpp
    src/core/layer_manager.cpp
    src/core/mode_switch.cpp
    src/profile/profile_manager.cpp
    src/profile/import_export.cpp
    src/profile/auto_switch.cpp
    src/util/logger.cpp
    src/util/hotkey_manager.cpp
    src/util/timer.cpp
    src/util/process_monitor.cpp
)

# ── Executable ────────────────────────────────────────────────────────────
add_executable(wz_gamepad WIN32 ${WZ_SOURCES})

target_include_directories(wz_gamepad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(wz_gamepad PRIVATE
    nlohmann_json::nlohmann_json
    User32.lib
    Shell32.lib
    Ole32.lib
    Gdi32.lib
    Psapi.lib
    Advapi32.lib
)

# ── Optional: link against ViGEmClient.lib ─────────────────────────────────
# If WZ_VIGEM_LIB is set, link against it; otherwise the user must supply it.
if(WZ_VIGEM_LIB AND EXISTS "${WZ_VIGEM_LIB}")
    target_link_libraries(wz_gamepad PRIVATE "${WZ_VIGEM_LIB}")
    message(STATUS "Linking ViGEmClient: ${WZ_VIGEM_LIB}")
else()
    message(STATUS "ViGEmClient.lib not specified via WZ_VIGEM_LIB.")
    message(STATUS "  Download ViGEmBus SDK and set -DWZ_VIGEM_LIB=<path to ViGEmClient.lib>")
    message(STATUS "  The include header is already bundled in include/vigem/Client.h")
endif()

# ── Compiler warnings ─────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(wz_gamepad PRIVATE /W4 /WX- /MP /utf-8)
    target_compile_definitions(wz_gamepad PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
endif()

# ── Tests ─────────────────────────────────────────────────────────────────
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# ── Install ───────────────────────────────────────────────────────────────
install(TARGETS wz_gamepad DESTINATION bin)
install(DIRECTORY profiles/ DESTINATION bin/profiles)
install(FILES installer/install.bat installer/uninstall.bat DESTINATION bin)
