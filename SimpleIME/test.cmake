enable_testing()

file(GLOB_RECURSE TEST_SOURCES test/*.cpp)

find_package(GTest CONFIG REQUIRED)

add_executable(
    ${TEST_PROJ_NAME}
    ${CMAKE_SOURCE_DIR}/common/imgui/ErrorNotifier.cpp
    src/AppConfig.cpp
    src/ImGuiThemeLoader.cpp
    src/ime/ImeManagerComposer.cpp
    src/ime/ImeManager.cpp
    ${TEST_SOURCES}
    ${IMGUI_SOURCES}
)
#target_compile_options(${TEST_PROJ_NAME} PRIVATE -fsanitize=address /Zi -D_DISABLE_STRING_ANNOTATION -D_DISABLE_VECTOR_ANNOTATION  -g )

target_link_libraries(
    ${TEST_PROJ_NAME}
    PRIVATE
    GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
    spdlog::spdlog
    ${DETOURS_LIBRARY}
    Freetype::Freetype
)
target_include_directories(
    ${TEST_PROJ_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${IMGUI_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SIMPLEINI_INCLUDE_DIRS}
)
target_compile_features(${TEST_PROJ_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard

include(GoogleTest)
gtest_discover_tests(${TEST_PROJ_NAME})

add_custom_command(
    TARGET "${TEST_PROJ_NAME}"
    PRE_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/test/resources" "$<TARGET_FILE_DIR:${TEST_PROJ_NAME}>/"
    VERBATIM
)