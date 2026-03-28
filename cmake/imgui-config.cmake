if (NOT ${EXTERN_DIR})
    message(FATAL_ERROR "Please set the EXTERN_DIR that contains the imgui source code before including this file.")
    return()
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/jamiemods.common.cmake)

set(IMGUI_INCLUDE_DIRS
   ${EXTERN_DIR}/imgui
   ${EXTERN_DIR}/imgui/backends
)
set(ImGui_Basic_Sources
   ${EXTERN_DIR}/imgui/imgui.h
   ${EXTERN_DIR}/imgui/imgui.cpp
   ${EXTERN_DIR}/imgui/imgui_widgets.cpp
   ${EXTERN_DIR}/imgui/imgui_tables.cpp
   ${EXTERN_DIR}/imgui/imgui_draw.cpp
)
set(ImGui_Backend_Win32_Dx11_Sources
   ${EXTERN_DIR}/imgui/backends/imgui_impl_win32.h
   ${EXTERN_DIR}/imgui/backends/imgui_impl_win32.cpp
   ${EXTERN_DIR}/imgui/backends/imgui_impl_dx11.h
   ${EXTERN_DIR}/imgui/backends/imgui_impl_dx11.cpp
)
set(ImGui_FreeType_Sources
   ${EXTERN_DIR}/imgui/misc/freetype/imgui_freetype.h
   ${EXTERN_DIR}/imgui/misc/freetype/imgui_freetype.cpp
)
set(ImGui_Demo_Sources
   ${EXTERN_DIR}/imgui/imgui_demo.cpp
)
