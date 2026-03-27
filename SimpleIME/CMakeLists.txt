cmake_minimum_required(VERSION 4.2)

project(
    SimpleIME
    VERSION 2.0.0
    DESCRIPTION "Simple IME support for Skyrim"
    LANGUAGES CXX
)
# Pre-release label appended to the human-readable version string.
# Leave empty for stable releases (e.g. set(PROJECT_VERSION_PRERELEASE "")).
# The numeric FILEVERSION resource field always uses PROJECT_VERSION (2.0.0).
set(PROJECT_VERSION_PRERELEASE "beta")
if(PROJECT_VERSION_PRERELEASE)
    set(PROJECT_VERSION_DISPLAY "${PROJECT_VERSION}-${PROJECT_VERSION_PRERELEASE}")
else()
    set(PROJECT_VERSION_DISPLAY "${PROJECT_VERSION}")
endif()

configure_file(
    ${JAMIE_MODS_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--checks=*,-llvmlibc-*,-fuchsia-*;--config-file=${JAMIE_MODS_DIR}/.clang-tidy;--extra-arg=/EHsc")
set(TEST_PROJ_NAME "${PROJECT_NAME}Test")

file(GLOB_RECURSE headers include/*.h include/*.hpp include/*.cpp)
file(GLOB_RECURSE sources src/*.cpp)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
    ${sources} ${headers}
)

## Set Output folder

if (DEFINED ENV{MO2_MODS_PATH} AND IS_DIRECTORY "$ENV{MO2_MODS_PATH}")
  set(OUTPUT_FOLDER "$ENV{MO2_MODS_PATH}/${PROJECT_NAME}")
endif ()

## Plugin settings
find_package(Freetype REQUIRED) # for imgui_freetype
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(${JAMIE_MODS_DIR}/cmake/imgui-config.cmake)
include(${JAMIE_MODS_EXTERN_DIR}/CommonLibSSE-NG/cmake/CommonLibSSE.cmake)

if (BUILD_TESTING)
  include(${CMAKE_CURRENT_SOURCE_DIR}/test.cmake)
endif ()

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES
    ${sources}
    ${headers}
    ${ImGui_Basic_Sources}
    ${ImGui_Backend_Win32_Dx11_Sources}
    ${ImGui_FreeType_Sources}
    ${JAMIE_MODS_COMMON_DIR}/fonts/FontManager.cpp
    ${JAMIE_MODS_COMMON_DIR}/i18n/translator_manager.cpp
    ${IMGUIEX_DIR}/imguiex/m3.cpp
    ${IMGUIEX_DIR}/imguiex/imguiex_m3.cpp
    ${IMGUIEX_DIR}/imguiex/imguiex_m3_slider.cpp
    ${IMGUIEX_DIR}/imguiex/M3ThemeBuilder.cpp
    ${IMGUIEX_DIR}/imguiex/ErrorNotifier.cpp
    ${IMGUIEX_DIR}/imguiex/ThemesLoader.cpp
    ${IMGUIEX_DIR}/imguiex/imgui_manager.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    MINIMUM_SKSE_VERSION "1.6.317"
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h"
    $<$<CONFIG:Debug>:SIMPLE_IME_TRACE>
)

# include directories
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${JAMIE_MODS_COMMON_DIR}
    ${JAMIE_MODS_EXTERN_DIR}
    ${IMGUIEX_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
    ${DETOURS_INCLUDE_DIRS}
    ${IMGUI_INCLUDE_DIRS}
)

# linkage
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    spdlog::spdlog
    ${DETOURS_LIBRARY}
    Freetype::Freetype
    MaterialYou
)

# close the unused param "/Zc:preprocessor" warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -Wshadow -Wconversion -Wno-unused-command-line-argument)

include(${JAMIE_MODS_DIR}/cmake/build-lucide-icon.cmake)
add_build_lucide_icons(${PROJECT_NAME})

############################################################
# CPACK
set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_DISPLAY})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_BUILD_TYPE}")
set(CPACK_OUTPUT_FILE_PREFIX "${JAMIE_MODS_DIR}/dist")
set(CPACK_PACKAGE_DIRECTORY "${JAMIE_MODS_DIR}/dist")
include(CPack)

############################################################
# Install
set(interface_path interface/${PROJECT_NAME})

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/config/${PROJECT_NAME}.toml"
    "${LUCIDE_ICON_OUT_TTF}"
    DESTINATION ${interface_path}
    COMPONENT mod_interface_files
)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/translate/
    DESTINATION ${interface_path}
    COMPONENT mod_interface_files
)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION SKSE/Plugins
    COMPONENT mod_runtime
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
      DESTINATION SKSE/Plugins
      COMPONENT mod_runtime
  )
endif ()

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${JAMIE_MODS_DIR}/LICENSE
    DESTINATION .
    COMPONENT doc
)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/lucide/Lucide License"
    DESTINATION "third_party"
    COMPONENT 3rd
)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/fomod
    DESTINATION .
    COMPONENT Distribution
)
cpack_add_component(mod_runtime)
cpack_add_component(mod_interface_files)
cpack_add_component(doc)
cpack_add_component(Distribution)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_NAME "Debug")
else ()
  set(BUILD_NAME "Release")
endif ()

file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")

set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")

if (DEFINED OUTPUT_FOLDER)
  # If you specify an <OUTPUT_FOLDER> (including via environment variables)
  # then we'll copy your mod files into Skyrim or a mod manager for you!

  # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
  set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")
  set(INTERFACE_FOLDER "${OUTPUT_FOLDER}/interface/${PROJECT_NAME}")

  message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

  add_custom_command(
      TARGET "${PROJECT_NAME}"
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/contrib/config/${PROJECT_NAME}$<$<CONFIG:DEBUG>:_debug>.toml" "${DLL_FOLDER}/${PROJECT_NAME}.toml"
      VERBATIM
  )

  #Copy translate dir
  add_custom_command(
      TARGET "${PROJECT_NAME}"
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${INTERFACE_FOLDER}"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory_if_different "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/translate" "${INTERFACE_FOLDER}"
      VERBATIM
  )

  # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )
  endif ()
endif ()
