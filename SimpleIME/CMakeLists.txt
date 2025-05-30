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

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
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
#set(OUTPUT_FOLDER "D:/GAME/Mod-Organizer-2/mods/SimpleIME")

## Plugin settings
find_package(Freetype REQUIRED) # for imgui_freetype
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")

include(${CMAKE_SOURCE_DIR}/cmake/imgui-config.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/test.cmake)
include(${CMAKE_SOURCE_DIR}/extern/CommonLibSSE-NG/cmake/CommonLibSSE.cmake)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES
    ${sources}
    ${headers}
    ${IMGUI_SOURCES}
    ${CMAKE_SOURCE_DIR}/common/imgui/ErrorNotifier.cpp
    ${CMAKE_SOURCE_DIR}/common/imgui/ThemesLoader.cpp
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
    ${CMAKE_SOURCE_DIR}
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
)

## Install config
set(CPACK_GENERATOR "7Z")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_BUILD_TYPE}")
include(CPack)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION SKSE/Plugins
    COMPONENT dll
)
# If you perform a "Debug" build, also copy .pdb file (for debug symbols)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  install(FILES "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
      DESTINATION SKSE/Plugins
      COMPONENT pdb
  )
endif ()
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/config/${PROJECT_NAME}.ini
    DESTINATION config
    COMPONENT configuration
)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION .
    COMPONENT doc
)
install(FILES
    ${CMAKE_SOURCE_DIR}/common/themes.toml
    DESTINATION interface/${PROJECT_NAME}
    COMPONENT doc
)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/fomod
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/translate
    DESTINATION .
    COMPONENT Distribution
)
cpack_add_component(dll)
cpack_add_component(configuration)
cpack_add_component(doc)
cpack_add_component(Distribution)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_NAME "Debug")
else ()
  set(BUILD_NAME "Release")
endif ()

file(GLOB_RECURSE OUTPUT_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.dll")
file(GLOB_RECURSE OUTPUT_PDBS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pdb")
#file(GLOB_RECURSE OUTPUT_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/Distribution/**/*.pex")

set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_DLLS}")
set_property(TARGET ${PROJECT_NAME}
    APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_PDBS}")
#set_property(TARGET ${PROJECT_NAME}
#        APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${OUTPUT_SCRIPTS}")

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
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/contrib/config/${PROJECT_NAME}$<$<CONFIG:DEBUG>:_debug>.ini" "${DLL_FOLDER}/${PROJECT_NAME}.ini"
      VERBATIM
  )

  #Copy themes file
  add_custom_command(
      TARGET "${PROJECT_NAME}"
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${INTERFACE_FOLDER}"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/common/themes.toml" "${INTERFACE_FOLDER}"
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
