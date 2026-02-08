enable_testing()

file(GLOB_RECURSE TEST_SOURCES test/*.cpp)

find_package(GTest CONFIG REQUIRED)
set(TEST_PROJ_NAME "CommonTest")
add_executable(
    ${TEST_PROJ_NAME}
    ${TEST_SOURCES}
        imgui/m3.cpp
)

target_link_libraries(
    ${TEST_PROJ_NAME}
    PRIVATE
    GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
)
target_include_directories(
    ${TEST_PROJ_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(${TEST_PROJ_NAME} PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJ_NAME})
