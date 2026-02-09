import argparse
import re
import sys
from pathlib import Path

IMGUIEX_WRAP_BLOCK_COMMENT_FORMAT = """/////////////////////////////////////////////////
/// ... {} ...
/////////////////////////////////////////////////

"""
IMGUIEX_WRAP_MARCOS = r"""#pragma once

namespace ImGuiEx
{
template <typename Derived, typename EnumType>
class FlagBuilder
{
    EnumType flags_ = static_cast<EnumType>(0);

public:
    constexpr FlagBuilder() = default;

    constexpr explicit FlagBuilder(EnumType f) : flags_(f) {}

    constexpr operator EnumType() const
    {
        return flags_;
    }

    constexpr Derived set(EnumType f) const {
        return Derived(static_cast<EnumType>(flags_ | f));
    }
};
"""
IMGUIEX_WRAP_CLASS_BODY_FORMAT = """class {} : public FlagBuilder<{}, {}> {{
    using FlagBuilder::FlagBuilder;
public:
    // clang-format off
"""
IMGUIEX_WRAP_CLASS_FUNCTION_FORMAT = """    //! {}
    {}constexpr auto {}() const -> {} {{ return set({}); }}
"""
IMGUIEX_WRAP_CLASS_END = """
    // clang-format on
};

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
                wrap_file.write(IMGUIEX_WRAP_CLASS_BODY_FORMAT.format(class_name, class_name, enum_name))
                in_enum_flags_definition = True
            if in_enum_flags_definition and re.match(r"\s*}.*", line):
                wrap_file.write(IMGUIEX_WRAP_CLASS_END)
                in_enum_flags_definition = False
            if not in_enum_flags_definition:
                continue
            if "#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS" in line:
                is_in_obsolete_block = True

            if "#endif" in line and is_in_obsolete_block:
                is_in_obsolete_block = False

            if no_obsolete and is_in_obsolete_block: continue

            current_pattern = r"^\s*{}_(\w+)".format(enum_name)
            if in_enum_flags_definition:
                match = re.search(current_pattern, line)
                if match:
                    flag_name = match.group(1)
                    deprecated_attr = '[[deprecated("Obsolete in ImGui")]] ' if is_in_obsolete_block else ""
                    raw_flag_name = f"{enum_name}_{flag_name}"
                    wrap_file.write(IMGUIEX_WRAP_CLASS_FUNCTION_FORMAT.format(raw_flag_name, deprecated_attr, flag_name, class_name, raw_flag_name))
        wrap_file.write("}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Scripts for Auto scan and wrap all ImGui enum flags.")
    parser.add_argument("--no-obsolete", action ='store_false', help="Should obsolete flags be included?")
    parser.add_argument("--src", default=f"{sys.prefix}/../extern/imgui/imgui.h", help="The input imgui.h file path.")
    parser.add_argument("--out", default=f"{sys.prefix}/../common/imguiex/imguiex_enum_wrap.h",
                        help="The output header file path")

    args = parser.parse_args()
    wrap_enum_flags(Path(args.src), Path(args.out), args.no_obsolete)
    print("Done.")
