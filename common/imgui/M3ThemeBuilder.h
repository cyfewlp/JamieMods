//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "../config.h"

#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{

struct Colors;

class ThemeBuilder
{
public:
    static void BuildThemeFromSeed(uint32_t seedArgb, bool isDark, Colors &colors);
};
}
}
