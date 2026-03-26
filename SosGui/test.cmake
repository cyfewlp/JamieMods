
set(TEST_PROJ_NAME ${PROJECT_NAME}Test)
enable_testing()
find_package(GTest CONFIG REQUIRED)
add_executable(
    ${TEST_PROJ_NAME}
    test/ClipperTest.cpp
)
add_test(AllTestsInMain ${TEST_PROJ_NAME})
target_link_libraries(${TEST_PROJ_NAME} PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
