cmake_minimum_required(VERSION 4.2)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif ()

# [BEGIN set_project_name]
# Set your project name. This will be the name of your SKSE .dll file.
project(JamieMods)

include(${CMAKE_SOURCE_DIR}/cmake/jamiemods.common.cmake)

set(_OLD_BUILD_TESTS ${BUILD_TESTS})
set(BUILD_TESTS OFF) # forbid CommonLibSSE-NG from building its tests, since we only want to build our own tests, and CommonLibSSE-NG's tests are not relevant to us and would just slow down the build
add_subdirectory(extern/CommonLibSSE-NG EXCLUDE_FROM_ALL)
set(BUILD_TESTS ${_OLD_BUILD_TESTS})

if (BUILD_TESTING)
    add_subdirectory(common) # FIXME: common only contains imguiex test. imguiex should move out from common.
endif ()

add_subdirectory(extern) # FIXME: the extern contains a MaterialYou static library, but it also contains some other header-only libraries. It would be better to split the MaterialYou static library into its own directory, and then we can just add_subdirectory that one directory here instead of the entire extern directory.
if (ENABLE_SIMPLE_IME)
    add_subdirectory(SimpleIME)
endif ()
add_subdirectory(SkyrimSETransmogrify EXCLUDE_FROM_ALL)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/private/CMakeLists.txt")
    add_subdirectory(private EXCLUDE_FROM_ALL)
else()
    message(STATUS "Private directory not found, skipping...")
endif()
