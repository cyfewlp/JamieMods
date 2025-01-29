project(
    SimpleIME
    VERSION 0.0.1 
    DESCRIPTION "Simple IME support for Skyrim"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY
)

set(headers
    include/Hooks.hpp
    include/SystemImeLoader.h
    include/Configs.h
    include/Device.h
    include/ImeApp.h
    include/ImeUI.h
    include/ImeWnd.hpp
)

set(sources
    src/plugin.cpp
	src/ImeApp.cpp
    src/ImeUI.cpp
    src/ImeWnd.cpp
    src/Hooks.cpp
    src/SystemImeLoader.cpp
)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${sources} ${headers})

## Set Output folder

if(DEFINED ENV{MO2_MODS_PATH} AND IS_DIRECTORY "$ENV{MO2_MODS_PATH}")
	set(OUTPUT_FOLDER "$ENV{MO2_MODS_PATH}/${PROJECT_NAME}")
endif()

## Plugin settings
find_package(spdlog CONFIG REQUIRED)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
find_library(DETOURS_LIBRARY detours REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)
find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")

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
		${UTILS_INCLUDE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${SIMPLEINI_INCLUDE_DIRS}
)

# linkage
target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
		spdlog::spdlog
		imgui
        ${DETOURS_LIBRARY}
)

if(DEFINED OUTPUT_FOLDER)
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
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/Data/SKSE/Plugins/${PROJECT_NAME}.ini" "${DLL_FOLDER}/${PROJECT_NAME}.ini"
			VERBATIM
	)

	# If you perform a "Debug" build, also copy .pdb file (for debug symbols)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(
				TARGET "${PROJECT_NAME}"
				POST_BUILD
				COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
				VERBATIM
		)
	endif()
endif()
