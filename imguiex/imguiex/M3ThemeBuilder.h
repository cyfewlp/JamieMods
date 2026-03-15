//
// Created by jamie on 2026/1/27.
//

#pragma once

namespace material_color_utilities
{
struct SchemeTonalSpot;
class Hct;
} // namespace material_color_utilities

namespace ImGuiEx::M3
{
struct SchemeConfig;
class ColorScheme;

namespace ThemeBuilder
{
auto Build(const SchemeConfig &config) -> ColorScheme;
} // namespace ThemeBuilder

} // namespace ImGuiEx::M3
