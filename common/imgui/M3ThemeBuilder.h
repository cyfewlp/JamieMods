//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "Material3.h"

#include <cstdint>

namespace material_color_utilities
{
struct SchemeTonalSpot;
class Hct;
}

namespace ImGuiEx::M3
{
class Colors;

class ThemeBuilder
{
    using Scheme = material_color_utilities::SchemeTonalSpot;
    using Hct = material_color_utilities::Hct;

public:
    static void BuildThemeFromSeed(uint32_t sourceColor, bool isDark, Colors &colors);

    static auto BuildThemeFromSeed(Argb sourceColor, bool isDark) -> Colors;

private:
    static void FillSurfaceColors(Colors::SurfaceColors &surfaceColors, const Scheme &scheme);

    static void FillContentColors(Colors::ContentColors &contentColors, const Scheme &scheme);
};
}