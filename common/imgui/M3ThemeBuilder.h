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
class Colors;

class ThemeBuilder
{
public:
    static void BuildThemeFromSeed(uint32_t sourceColor, bool isDark, Colors &colors);
};
}
}
