//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "Material3.h"

namespace material_color_utilities
{
struct SchemeTonalSpot;
class Hct;
} // namespace material_color_utilities

namespace ImGuiEx::M3
{
class Colors;

namespace ThemeBuilder
{
auto Build(const Colors::SchemeConfig &config) -> Colors;
} // namespace ThemeBuilder

} // namespace ImGuiEx::M3
