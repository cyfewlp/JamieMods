project(
        SimpleIME
        VERSION 0.0.6
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
##
#  Test Config

enable_testing()

option(INSTALL_GMOCK "Install Googletest's GMock?" OFF)
option(INSTALL_GTEST "Install Googletest's GTest?" OFF)

include(FetchContent)
FetchContent_Declare(
  googletest 
  URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip 
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
add_executable(
    ${TEST_PROJ_NAME}
    src/AppConfig.cpp
    test/AppConfigTest.cpp
    test/WCharUtilsTest.cpp
)
#target_compile_options(${TEST_PROJ_NAME} PRIVATE -fsanitize=address /Zi -D_DISABLE_STRING_ANNOTATION -D_DISABLE_VECTOR_ANNOTATION  -g )

target_link_libraries(
  ${TEST_PROJ_NAME}
  PRIVATE
  GTest::gtest_main
  spdlog::spdlog
)
target_include_directories(
  ${TEST_PROJ_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${SIMPLEINI_INCLUDE_DIRS}
)

include(GoogleTest)
gtest_discover_tests(${TEST_PROJ_NAME})

add_custom_command(
    TARGET "${TEST_PROJ_NAME}"
    PRE_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/test/resources" "$<TARGET_FILE_DIR:${TEST_PROJ_NAME}>/"
    VERBATIM
)
##
# Plugin Config
set(headers
    include/context.h
    include/configs/AppConfig.h
    include/configs/CustomMessage.h
    include/configs/converter.h
    include/ImeApp.h
    include/ImeUI.h
    include/ImeWnd.hpp
    include/FakeDirectInputDevice.h
    include/Utils.h
    include/hooks/Hooks.hpp
    include/hooks/ScaleformHook.h
    include/hooks/UiAddMessageHook.h
    include/ime/TextEditor.h
    include/ime/ITextService.h
    include/ime/ITextServiceFactory.h
    include/tsf/TsfSupport.h
    include/tsf/TextStore.h
    include/tsf/LangProfileUtil.h
	include/tsf/TsfCompartment.h
)

set(sources
    src/context.cpp
    src/plugin.cpp
    src/ImeApp.cpp
    src/ImeUI.cpp
    src/ImeWnd.cpp
    src/AppConfig.cpp
    src/hooks/Hooks.cpp
    src/hooks/ScaleformHook.cpp
    src/hooks/UiAddMessageHook.cpp
    src/ime/TextEditor.cpp
    src/ime/Imm32TextService.cpp
    src/tsf/TsfSupport.cpp
    src/tsf/TextStore.cpp
    src/tsf/TsfCompartment.cpp
    src/tsf/LangProfileUtil.cpp
    src/tsf/TextService.cpp
)

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
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES 
    ${sources}
	${headers}
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    MINIMUM_SKSE_VERSION "1.6.317"
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23) # <--- use C++23 standard
target_precompile_headers(${PROJECT_NAME} PRIVATE include/PCH.h)

# include directories
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${DETOURS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    #${SIMPLEINI_INCLUDE_DIRS}
)

# linkage
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    spdlog::spdlog
    imgui
    ${DETOURS_LIBRARY}
    Microsoft.GSL::GSL
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
install(FILES
        ${CMAKE_SOURCE_DIR}/Data/SKSE/Plugins/${PROJECT_NAME}.ini
        DESTINATION SKSE/Plugins
        COMPONENT configuration
)
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        DESTINATION .
        COMPONENT doc
)
cpack_add_component(dll)
cpack_add_component(configuration)
cpack_add_component(doc)

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

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/Data/SKSE/Plugins$<$<CONFIG:DEBUG>:/debug>/${PROJECT_NAME}.ini" "${DLL_FOLDER}/${PROJECT_NAME}.ini"
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
