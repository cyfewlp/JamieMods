import argparse
import re
import sys
from pathlib import Path

IMGUI_FLAG_PATTERN = r"^\s*ImGui{}_(\w+)\s*"

IMGUIEX_WRAP_BLOCK_COMMENT_FORMAT = """
//////////////////////////////////////////////////
/// {}

"""
IMGUIEX_WRAP_MARCOS = r"""#pragma once

namespace ImGuiEx
{
#define FLAGS_CLASS_BODY(className, enumName)                                                                                    \
    class className                                                                                                    \
    {                                                                                                                  \
        enumName flags = 0;                                                                                    \
                                                                                                                       \
    public:                                                                                                            \
        constexpr className() = default;                                                                               \
        constexpr explicit className(enumName f) : flags(f) {}

#define FLAGS_CLASS_FUNCTION(className, enumName, flagName, is_deprecated)                                                                      \
    IMGUI_OPTIONAL_DEPRECATED(is_deprecated)                                                                           \
    constexpr auto flagName() -> className &                                                                           \
    {                                                                                                                  \
        flags |= enumName##_##flagName;                                                                        \
        return *this;                                                                                                  \
    }

#define FLAGS_CLASS_END(enumName)                                                                                     \
    constexpr operator enumName() const                                                                        \
    {                                                                                                                  \
        return flags;                                                                                                  \
    }                                                                                                                  \
    }                                                                                                                  \
    ;

#define IMGUI_OPTIONAL_DEPRECATED(is_deprecated) IMGUI_DEPRECATED_##is_deprecated
#define IMGUI_DEPRECATED_True [[deprecated("This flag is obsolete in ImGui.")]]
#define IMGUI_DEPRECATED_False
"""


def wrap_enum_flags(header_file: Path, wrap_flags_file: Path, no_obsolete: bool):
    if not header_file.name == "imgui.h":
        return
    with open(header_file, 'r', encoding='utf-8') as imgui_header:
        lines = imgui_header.readlines()

    in_enum_flags_definition = False
    is_in_obsolete_block = False
    with open(wrap_flags_file, "w") as wrap_file:
        class_name = ""
        enum_name = ""
        wrap_file.write(IMGUIEX_WRAP_MARCOS)
        for line in lines:
            start_enum_flags_definition_match = re.search(r"^enum (Im(?:Gui)?([A-Z]\w+Flags))_", line)
            if start_enum_flags_definition_match:
                enum_name = start_enum_flags_definition_match[1]
                class_name = start_enum_flags_definition_match[2]
                print(f"start wrap: {enum_name}")
                wrap_file.write(IMGUIEX_WRAP_BLOCK_COMMENT_FORMAT.format(enum_name))
                wrap_file.write(f"FLAGS_CLASS_BODY({class_name}, {enum_name})\n")
                in_enum_flags_definition = True
            if in_enum_flags_definition and re.match(r"\s*}.*", line):
                wrap_file.write(f"FLAGS_CLASS_END({enum_name})\n\n")
                in_enum_flags_definition = False
            if not in_enum_flags_definition:
                continue
            if "#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS" in line:
                is_in_obsolete_block = True

            if "#endif" in line and is_in_obsolete_block:
                is_in_obsolete_block = False

            if not no_obsolete and is_in_obsolete_block: continue

            current_pattern = r"^\s*{}_(\w+)".format(enum_name)
            if in_enum_flags_definition:
                match = re.search(current_pattern, line)
                if match:
                    flag_name = match.group(1)
                    is_deprecated = "True" if (is_in_obsolete_block) else "False"
                    wrap_file.write(f"FLAGS_CLASS_FUNCTION({class_name}, {enum_name}, {flag_name}, {is_deprecated})\n")
        wrap_file.write("}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Scripts for Auto scan and wrap all ImGui enum flags.")
    parser.add_argument("--no-obsolete", default=False, help="Should obsolete flags be included?")
    parser.add_argument("--src", default=f"{sys.prefix}/../extern/imgui/imgui.h", help="The input imgui.h file path.")
    parser.add_argument("--out", default=f"{sys.prefix}/../common/imgui/imguiex_enum_wrap.h", help="The output header file path")

    args = parser.parse_args()
    wrap_enum_flags(Path(args.src), Path(args.out), args.no_obsolete)
    print("Done.")
