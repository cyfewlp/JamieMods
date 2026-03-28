cmake_minimum_required(VERSION 4.2)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTING "Build tests" OFF)

if (BUILD_TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES "test-deps")
endif ()

# [BEGIN set_project_name]
# Set your project name. This will be the name of your SKSE .dll file.
project(JamieMods)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/jamiemods.common.cmake)

add_subdirectory(imguiex)
add_subdirectory(cmake/material-you)
