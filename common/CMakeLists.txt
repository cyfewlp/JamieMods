enable_testing()

file(GLOB_RECURSE TEST_SOURCES test/*.cpp)

include(${CMAKE_SOURCE_DIR}/cmake/imgui-config.cmake)

find_package(GTest CONFIG REQUIRED)
set(TEST_PROJ_NAME "CommonTest")
add_executable(
    ${TEST_PROJ_NAME}
    ${TEST_SOURCES}
        imguiex/m3.cpp
        imguiex/imguiex_m3.cpp
        imguiex/M3ThemeBuilder.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_capture_tool.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_context.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_coroutine.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_engine.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_exporters.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_perftool.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_ui.cpp
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine/imgui_te_utils.cpp
        ${ImGui_Basic_Sources}
        ${ImGui_Backend_Win32_Dx11_Sources}
        ${ImGui_Demo_Sources}
        test/imgui_test_config.h
        test/m3_test_fixture.cpp
        test/m3_test_fixture.h
        test/m3_list_test.cpp
)
target_include_directories(
    ${TEST_PROJ_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/extern/imgui
        ${CMAKE_SOURCE_DIR}/extern/imgui/backends
        ${CMAKE_SOURCE_DIR}/extern/imgui_test_engine/imgui_test_engine
)

target_link_libraries(
        ${TEST_PROJ_NAME}
        PRIVATE
        MaterialYou
        GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
)
target_compile_definitions(${TEST_PROJ_NAME}
        PRIVATE
        SIMPLE_EXPORTS
        IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/test/imgui_test_config.h"
)
target_compile_features(${TEST_PROJ_NAME} PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJ_NAME})
