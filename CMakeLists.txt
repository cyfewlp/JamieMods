# This file configures the "build" of this SKSE plugin
# - how to compile the C++ plugin (in addition to settings found in CMakePresets.json)
# - where to output the resulting .dll file

# It's recommended to set a minimum CMake version.
# If you use CMake features from higher versions, update this to match.
cmake_minimum_required(VERSION 3.21)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# [BEGIN set_project_name]
# Set your project name. This will be the name of your SKSE .dll file.
project(HelloWorld)

add_subdirectory(extern)
add_subdirectory(dev)
add_subdirectory(SkyrimSETransmogrify)