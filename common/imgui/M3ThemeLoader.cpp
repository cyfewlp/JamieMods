//
// Created by jamie on 2026/1/27.
//

#include "M3ThemeLoader.h"

#include "../json/simdjson.h"
#include "../utils.h"
#include "Material3.h"

#include <imgui.h>
#include <string_view>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{

auto LoadTheme(std::string_view path, Colors &colors) -> bool
{
    try
    {
        bool success = true;
        using namespace simdjson;
        ondemand::parser   parser;
        padded_string      json     = padded_string::load(path.data());
        ondemand::document tweets   = parser.iterate(json);
        auto               themeRaw = tweets["schemes"]["light"];

        auto setIfSuccess = [&](std::string_view name, ImColor &color) {
            if (success)
            {
                auto opt = CommonUtils::HexToU32(themeRaw[name].get_string());
                if (opt)
                {
                    color = opt.value();
                }
                else
                {
                    success = false;
                }
            }
        };
        setIfSuccess("primary", colors.primary);
        setIfSuccess("surfaceTint", colors.surface_tint);
        setIfSuccess("onPrimary", colors.on_primary);
        setIfSuccess("primaryContainer", colors.primary_container);
        setIfSuccess("onPrimaryContainer", colors.on_primary_container);
        setIfSuccess("secondary", colors.secondary);
        setIfSuccess("onSecondary", colors.on_secondary);
        setIfSuccess("secondaryContainer", colors.secondary_container);
        setIfSuccess("onSecondaryContainer", colors.on_secondary_container);
        setIfSuccess("tertiary", colors.tertiary);
        setIfSuccess("onTertiary", colors.on_tertiary);
        setIfSuccess("tertiaryContainer", colors.tertiary_container);
        setIfSuccess("onTertiaryContainer", colors.on_tertiary_container);
        setIfSuccess("error", colors.error);
        setIfSuccess("onError", colors.on_error);
        setIfSuccess("errorContainer", colors.error_container);
        setIfSuccess("onErrorContainer", colors.on_error_container);
        setIfSuccess("background", colors.background);
        setIfSuccess("onBackground", colors.on_background);
        setIfSuccess("surface", colors.surface);
        setIfSuccess("onSurface", colors.on_surface);
        setIfSuccess("surfaceVariant", colors.surface_variant);
        setIfSuccess("onSurfaceVariant", colors.on_surface_variant);
        setIfSuccess("outline", colors.outline);
        setIfSuccess("outlineVariant", colors.outline_variant);
        setIfSuccess("shadow", colors.shadow);
        setIfSuccess("scrim", colors.scrim);
        setIfSuccess("inverseSurface", colors.inverse_surface);
        setIfSuccess("inverseOnSurface", colors.inverse_on_surface);
        setIfSuccess("inversePrimary", colors.inverse_primary);
        setIfSuccess("primaryFixed", colors.primary_fixed);
        setIfSuccess("onPrimaryFixed", colors.on_primary_fixed);
        setIfSuccess("primaryFixedDim", colors.primary_fixed_dim);
        setIfSuccess("onPrimaryFixedVariant", colors.on_primary_fixed_variant);
        setIfSuccess("secondaryFixed", colors.secondary_fixed);
        setIfSuccess("onSecondaryFixed", colors.on_secondary_fixed);
        setIfSuccess("secondaryFixedDim", colors.secondary_fixed_dim);
        setIfSuccess("onSecondaryFixedVariant", colors.on_secondary_fixed_variant);
        setIfSuccess("tertiaryFixed", colors.tertiary_fixed);
        setIfSuccess("onTertiaryFixed", colors.on_tertiary_fixed);
        setIfSuccess("tertiaryFixedDim", colors.tertiary_fixed_dim);
        setIfSuccess("onTertiaryFixedVariant", colors.on_tertiary_fixed_variant);
        setIfSuccess("surfaceDim", colors.surface_dim);
        setIfSuccess("surfaceBright", colors.surface_bright);
        setIfSuccess("surfaceContainerLowest", colors.surface_container_lowest);
        setIfSuccess("surfaceContainerLow", colors.surface_container_low);
        setIfSuccess("surfaceContainer", colors.surface_container);
        setIfSuccess("surfaceContainerHigh", colors.surface_container_high);
        setIfSuccess("surfaceContainerHighest", colors.surface_container_highest);
        return success;
    }
    catch (...)
    {
        // parse fail
        return false;
    }
}

void SetDefaultDarkTheme(Colors &colors)
{
    colors.primary                    = ImColor(211, 188, 253);
    colors.surface_tint               = ImColor(211, 188, 253);
    colors.on_primary                 = ImColor(56, 38, 92);
    colors.primary_container          = ImColor(79, 61, 116);
    colors.on_primary_container       = ImColor(235, 221, 255);
    colors.secondary                  = ImColor(205, 194, 219);
    colors.on_secondary               = ImColor(52, 45, 64);
    colors.secondary_container        = ImColor(75, 67, 88);
    colors.on_secondary_container     = ImColor(233, 222, 248);
    colors.tertiary                   = ImColor(240, 183, 197);
    colors.on_tertiary                = ImColor(74, 37, 48);
    colors.tertiary_container         = ImColor(100, 59, 70);
    colors.on_tertiary_container      = ImColor(255, 217, 225);
    colors.error                      = ImColor(255, 180, 171);
    colors.on_error                   = ImColor(105, 0, 5);
    colors.error_container            = ImColor(147, 0, 10);
    colors.on_error_container         = ImColor(255, 218, 214);
    colors.background                 = ImColor(21, 18, 24);
    colors.on_background              = ImColor(231, 224, 232);
    colors.surface                    = ImColor(21, 18, 24);
    colors.on_surface                 = ImColor(231, 224, 232);
    colors.surface_variant            = ImColor(73, 69, 78);
    colors.on_surface_variant         = ImColor(203, 196, 207);
    colors.outline                    = ImColor(148, 143, 153);
    colors.outline_variant            = ImColor(73, 69, 78);
    colors.shadow                     = ImColor(0, 0, 0);
    colors.scrim                      = ImColor(0, 0, 0);
    colors.inverse_surface            = ImColor(231, 224, 232);
    colors.inverse_on_surface         = ImColor(50, 47, 53);
    colors.inverse_primary            = ImColor(104, 84, 142);
    colors.primary_fixed              = ImColor(235, 221, 255);
    colors.on_primary_fixed           = ImColor(35, 15, 70);
    colors.primary_fixed_dim          = ImColor(211, 188, 253);
    colors.on_primary_fixed_variant   = ImColor(79, 61, 116);
    colors.secondary_fixed            = ImColor(233, 222, 248);
    colors.on_secondary_fixed         = ImColor(31, 24, 43);
    colors.secondary_fixed_dim        = ImColor(205, 194, 219);
    colors.on_secondary_fixed_variant = ImColor(75, 67, 88);
    colors.tertiary_fixed             = ImColor(255, 217, 225);
    colors.on_tertiary_fixed          = ImColor(49, 16, 27);
    colors.tertiary_fixed_dim         = ImColor(240, 183, 197);
    colors.on_tertiary_fixed_variant  = ImColor(100, 59, 70);
    colors.surface_dim                = ImColor(21, 18, 24);
    colors.surface_bright             = ImColor(59, 56, 62);
    colors.surface_container_lowest   = ImColor(15, 13, 19);
    colors.surface_container_low      = ImColor(29, 27, 32);
    colors.surface_container          = ImColor(33, 31, 36);
    colors.surface_container_high     = ImColor(44, 41, 47);
    colors.surface_container_highest  = ImColor(54, 52, 58);
}
}
}