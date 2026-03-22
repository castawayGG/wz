cmake_minimum_required(VERSION 3.20)

# Simple unit test executable (no external test framework needed)
set(WZ_TEST_SOURCES
    test_main.cpp
    test_axis_processor.cpp
    test_mapping_engine.cpp
    test_macro_engine.cpp
    test_profile_manager.cpp
)

add_executable(wz_tests ${WZ_TEST_SOURCES}
    ../src/core/axis_processor.cpp
    ../src/core/mapping_engine.cpp
    ../src/core/macro_engine.cpp
    ../src/core/layer_manager.cpp
    ../src/core/mode_switch.cpp
    ../src/profile/profile_manager.cpp
    ../src/profile/import_export.cpp
    ../src/profile/auto_switch.cpp
    ../src/util/logger.cpp
    ../src/util/timer.cpp
    ../src/util/process_monitor.cpp
    ../src/util/hotkey_manager.cpp
)

target_include_directories(wz_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(wz_tests PRIVATE
    nlohmann_json::nlohmann_json
    User32.lib
    Psapi.lib
    Advapi32.lib
)

if(MSVC)
    target_compile_options(wz_tests PRIVATE /W3 /utf-8)
    target_compile_definitions(wz_tests PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
endif()

add_test(NAME wz_unit_tests COMMAND wz_tests)
