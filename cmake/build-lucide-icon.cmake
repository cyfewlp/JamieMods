# Fetches the Lucide icons, builds a TTF font containing the icons.
# 1. Add a "lucide-icons.txt" in assets/lucide/ with the list of icons you want to include (one per line, without .svg extension)
# 2. Call add_build_lucide_icons(<target>) in your CMakeLists.txt after the target is defined.
# This fucntion will oouput a LUCIDE_ICON_OUT_TTF variable with the path to the generated TTF file.
function(add_build_lucide_icons TARGET)
    find_program(FONTFORGE_EXECUTABLE fontforge REQUIRED)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(ICON_LIST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/assets/lucide/lucide-icons.txt)
    set(ICON_BUILD_SCRIPT ${JAMIE_MODS_DIR}/scripts/build_icons_font.py)
    if (NOT EXISTS ${ICON_LIST_FILE})
        message(FATAL_ERROR "Missing lucide icons file ${ICON_LIST_FILE}.")
        return()
    endif ()
    if (NOT EXISTS ${ICON_BUILD_SCRIPT})
        message(FATAL_ERROR "Missing python script: ${ICON_BUILD_SCRIPT}.")
        return()
    endif ()

    include(FetchContent)
    FetchContent_Declare(
        lucide
        URL https://github.com/lucide-icons/lucide/releases/download/1.7.0/lucide-icons-1.7.0.zip
        URL_HASH SHA256=f1ea79d5aba7add3e0d282604e546c84de7ff14b024e213377acf5faaf694b11
    )

    FetchContent_MakeAvailable(lucide)

    FetchContent_GetProperties(lucide)
    set(ICONS_DIR ${lucide_SOURCE_DIR})
    set(ICON_FILE "lucide-icons.ttf")

    set(LUCIDE_ICON_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/icons)
    set(LUCIDE_ICON_OUT_HEADER ${LUCIDE_ICON_GEN_DIR}/icons.h)
    set(LUCIDE_ICON_OUT_TTF ${LUCIDE_ICON_GEN_DIR}/${ICON_FILE} PARENT_SCOPE)

    # Read icon names and build a list of SVG dependencies
    file(STRINGS "${ICON_LIST_FILE}" ICON_NAMES)
    set(ICON_SVG_DEPS "")
    foreach (name IN LISTS ICON_NAMES)
        string(STRIP "${name}" name)
        if (name STREQUAL "")
            continue()
        endif ()
        # allow comments
        string(SUBSTRING "${name}" 0 1 first_char)
        if (first_char STREQUAL "#")
            continue()
        endif ()
        list(APPEND ICON_SVG_DEPS "${ICONS_DIR}/${name}.svg")
    endforeach ()

    add_custom_command(
        OUTPUT ${LUCIDE_ICON_OUT_TTF} ${LUCIDE_ICON_OUT_HEADER}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${LUCIDE_ICON_GEN_DIR}

        COMMAND ${FONTFORGE_EXECUTABLE} -script ${ICON_BUILD_SCRIPT} --icons_dir ${ICONS_DIR} --list ${ICON_LIST_FILE} --out ${LUCIDE_ICON_GEN_DIR}

        DEPENDS ${ICON_LIST_FILE} ${ICON_BUILD_SCRIPT} ${ICON_SVG_DEPS}
        COMMENT "Generating icon font from Lucide (fetched via FetchContent)"
        VERBATIM
    )

    add_custom_target(icons DEPENDS ${LUCIDE_ICON_OUT_TTF} ${LUCIDE_ICON_OUT_HEADER})
    add_dependencies(${TARGET} icons)
endfunction()
