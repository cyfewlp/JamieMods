# add MaterialYou
set(MATERIAL_YOU_PATH ${CMAKE_CURRENT_SOURCE_DIR}/material-color-utilities)
set(MCU_SOURCES
    "${MATERIAL_YOU_PATH}/cpp/cam/cam.cc"
    "${MATERIAL_YOU_PATH}/cpp/cam/hct.cc"
    "${MATERIAL_YOU_PATH}/cpp/cam/hct_solver.cc"
    "${MATERIAL_YOU_PATH}/cpp/contrast/contrast.cc"
    "${MATERIAL_YOU_PATH}/cpp/dislike/dislike.cc"
    "${MATERIAL_YOU_PATH}/cpp/dynamiccolor/dynamic_color.cc"
    "${MATERIAL_YOU_PATH}/cpp/dynamiccolor/dynamic_scheme.cc"
    "${MATERIAL_YOU_PATH}/cpp/dynamiccolor/material_dynamic_colors.cc"
    "${MATERIAL_YOU_PATH}/cpp/palettes/tones.cc"
    "${MATERIAL_YOU_PATH}/cpp/scheme/scheme_tonal_spot.cc"
    #    "${MATERIAL_YOU_PATH}/cpp/utils/utils.cc"
    "mcu_utils.cpp" # avoid dependency google absl library
)

add_library(MaterialYou STATIC ${MCU_SOURCES})

target_compile_features(MaterialYou PUBLIC cxx_std_23)
target_include_directories(MaterialYou PUBLIC ${MATERIAL_YOU_PATH})
if(MSVC)
  target_compile_definitions(MaterialYou PUBLIC _USE_MATH_DEFINES)
endif()
