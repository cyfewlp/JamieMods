//
// Created by jamie on 2026/2/2.
//
#include "M3ThemeBuilder.h"
#include "Material3.h"

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{
void M3Styles::RebuildColors(const uint32_t sourceColor, const bool isDark)
{
    ThemeBuilder::BuildThemeFromSeed(sourceColor, isDark, colors);
}
}
}