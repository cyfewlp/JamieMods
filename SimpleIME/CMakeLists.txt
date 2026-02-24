project(
    SimpleIME
    VERSION 1.3.0
    DESCRIPTION "Simple IME support for Skyrim"
    LANGUAGES CXX
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

#set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--checks=*,-llvmlibc-*,-fuchsia-*;--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy;--extra-arg=/EHsc")
set(TEST_PROJ_NAME "${PROJECT_NAME}Test")

file(GLOB_RECURSE headers *.h include/*.hpp)
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

include(${CMAKE_SOURCE_DIR}/cmake/imgui-config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/test.cmake)
include(${CMAKE_SOURCE_DIR}/extern/CommonLibSSE-NG/cmake/CommonLibSSE.cmake)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES
    ${sources}
    ${headers}
    ${ImGui_Basic_Sources}
    ${ImGui_Backend_Win32_Dx11_Sources}
    ${ImGui_FreeType_Sources}
#    ${IMGUI_DEMO_SOURCES}
    ${CMAKE_SOURCE_DIR}/common/json/simdjson.cpp
    ${CMAKE_SOURCE_DIR}/common/imguiex/m3.cpp
    ${CMAKE_SOURCE_DIR}/common/imguiex/imguiex_m3.cpp
    ${CMAKE_SOURCE_DIR}/common/imguiex/imguiex_m3_slider.cpp
    ${CMAKE_SOURCE_DIR}/common/imguiex/M3ThemeBuilder.cpp
    ${CMAKE_SOURCE_DIR}/common/imguiex/ErrorNotifier.cpp
    ${CMAKE_SOURCE_DIR}/common/imguiex/ThemesLoader.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    MINIMUM_SKSE_VERSION "1.6.317"
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE SIMPLE_IME_DEBUG)
endif ()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    SIMPLE_EXPORTS
    IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h"
)

# include directories
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${DETOURS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/common
    ${IMGUI_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
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

include(FetchContent)

############################################################
# Auto pack icons
FetchContent_Declare(
        lucide
        URL https://github.com/lucide-icons/lucide/releases/download/0.575.0/lucide-icons-0.575.0.zip
        URL_HASH SHA256=254cca92a5686b03756b077cb931b935d1550982d1c6d64893eeb7ba047d5caa
)

FetchContent_MakeAvailable(lucide)

set(ICON_FILE "simple-ime-icons.ttf")

FetchContent_GetProperties(lucide)
set(ICONS_DIR ${lucide_SOURCE_DIR})

find_program(FONTFORGE_EXECUTABLE fontforge REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(ICON_LIST_FILE ${CMAKE_SOURCE_DIR}/assets/lucide/lucide-icons.txt)
set(ICON_BUILD_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/build_icons_font.py)

set(ICON_GEN_DIR ${CMAKE_BINARY_DIR}/generated/icons)
set(ICON_OUT_TTF ${ICON_GEN_DIR}/${ICON_FILE})
set(ICON_OUT_HEADER ${ICON_GEN_DIR}/icons.h)

# Read icon names and build a list of SVG dependencies
file(STRINGS "${ICON_LIST_FILE}" ICON_NAMES)
set(ICON_SVG_DEPS "")
foreach(name IN LISTS ICON_NAMES)
  string(STRIP "${name}" name)
  if(name STREQUAL "")
    continue()
  endif()
  # allow comments
  string(SUBSTRING "${name}" 0 1 first_char)
  if(first_char STREQUAL "#")
    continue()
  endif()
  list(APPEND ICON_SVG_DEPS "${ICONS_DIR}/${name}.svg")
endforeach()

add_custom_command(
        OUTPUT ${ICON_OUT_TTF} ${ICON_OUT_HEADER}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ICON_GEN_DIR}

        COMMAND ${FONTFORGE_EXECUTABLE} -script ${ICON_BUILD_SCRIPT}  --icons_dir ${ICONS_DIR} --list ${ICON_LIST_FILE} --out ${ICON_GEN_DIR}

        DEPENDS ${ICON_LIST_FILE} ${ICON_BUILD_SCRIPT} ${ICON_SVG_DEPS}
        COMMENT "Generating icon font from Lucide (fetched via FetchContent)"
        VERBATIM
)

add_custom_target(icons DEPENDS ${ICON_OUT_TTF} ${ICON_OUT_HEADER})
add_dependencies(${PROJECT_NAME} icons)

############################################################
# CPACK
set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_BUILD_TYPE}")
include(CPack)

############################################################
# Install
set(interface_path interface/${PROJECT_NAME})

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/contrib/config/${PROJECT_NAME}.toml"
        "${ICON_OUT_TTF}"
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
    ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION .
    COMPONENT doc
)
install(FILES
        "${CMAKE_SOURCE_DIR}/assets/lucide/Lucide License"
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
