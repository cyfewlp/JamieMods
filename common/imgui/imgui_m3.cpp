//
// Created by jamie on 2026/2/2.
//
#include "M3ThemeBuilder.h"
#include "Material3.h"

namespace ImGuiEx::M3
{
void M3Styles::RebuildColors(const Argb sourceColor, const bool isDark)
{
    ThemeBuilder::BuildThemeFromSeed(sourceColor, isDark, colors);
}
} // namespace ImGuiEx::M3
