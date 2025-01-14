
find_package(Freetype REQUIRED)

add_library(
	imgui STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx11.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/freetype/imgui_freetype.cpp
)
# target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32 IMGUI_ENABLE_FREETYPE)
target_include_directories(
	imgui
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/imgui
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
		${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/freetype
)

target_precompile_headers(
	imgui PUBLIC my_imgui_config.h
)

# linkage
target_link_libraries(
	imgui 
	PRIVATE
		Freetype::Freetype
)