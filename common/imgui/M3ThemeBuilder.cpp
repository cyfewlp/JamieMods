//
// Created by jamie on 2026/1/27.
//

#include "M3ThemeBuilder.h"

#include "Material3.h"
#include "cpp/scheme/scheme_tonal_spot.h"

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{
void ThemeBuilder::BuildThemeFromSeed(uint32_t seedArgb, bool isDark, Colors &colors)
{
    using namespace material_color_utilities;
    SchemeTonalSpot scheme(Hct(seedArgb), isDark, 0.0);

    auto argbToImColor = [](const Argb argb) -> ImU32 {
        uint8_t a = (argb & 0xFF000000) >> 24;
        uint8_t r = (argb & 0xFF0000) >> 16;
        uint8_t g = (argb & 0xFF00) >> 8;
        uint8_t b = argb & 0xFF;
        return ImColor(r, g, b, a);
    };

    colors.primary                    = argbToImColor(scheme.GetPrimary());
    colors.surface_tint               = argbToImColor(scheme.GetSurfaceTint());
    colors.on_primary                 = argbToImColor(scheme.GetOnPrimary());
    colors.primary_container          = argbToImColor(scheme.GetPrimaryContainer());
    colors.on_primary_container       = argbToImColor(scheme.GetOnPrimaryContainer());
    colors.secondary                  = argbToImColor(scheme.GetSecondary());
    colors.on_secondary               = argbToImColor(scheme.GetOnSecondary());
    colors.secondary_container        = argbToImColor(scheme.GetSecondaryContainer());
    colors.on_secondary_container     = argbToImColor(scheme.GetOnSecondaryContainer());
    colors.tertiary                   = argbToImColor(scheme.GetTertiary());
    colors.on_tertiary                = argbToImColor(scheme.GetOnTertiary());
    colors.tertiary_container         = argbToImColor(scheme.GetTertiaryContainer());
    colors.on_tertiary_container      = argbToImColor(scheme.GetOnTertiaryContainer());
    colors.error                      = argbToImColor(scheme.GetError());
    colors.on_error                   = argbToImColor(scheme.GetOnError());
    colors.error_container            = argbToImColor(scheme.GetErrorContainer());
    colors.on_error_container         = argbToImColor(scheme.GetOnErrorContainer());
    colors.background                 = argbToImColor(scheme.GetBackground());
    colors.on_background              = argbToImColor(scheme.GetOnBackground());
    colors.surface                    = argbToImColor(scheme.GetSurface());
    colors.on_surface                 = argbToImColor(scheme.GetOnSurface());
    colors.surface_variant            = argbToImColor(scheme.GetSurfaceVariant());
    colors.on_surface_variant         = argbToImColor(scheme.GetOnSurfaceVariant());
    colors.outline                    = argbToImColor(scheme.GetOutline());
    colors.outline_variant            = argbToImColor(scheme.GetOutlineVariant());
    colors.shadow                     = argbToImColor(scheme.GetShadow());
    colors.scrim                      = argbToImColor(scheme.GetScrim());
    colors.inverse_surface            = argbToImColor(scheme.GetInverseSurface());
    colors.inverse_on_surface         = argbToImColor(scheme.GetInverseOnSurface());
    colors.inverse_primary            = argbToImColor(scheme.GetInversePrimary());
    colors.primary_fixed              = argbToImColor(scheme.GetPrimaryFixed());
    colors.on_primary_fixed           = argbToImColor(scheme.GetOnPrimaryFixed());
    colors.primary_fixed_dim          = argbToImColor(scheme.GetPrimaryFixedDim());
    colors.on_primary_fixed_variant   = argbToImColor(scheme.GetOnPrimaryFixedVariant());
    colors.secondary_fixed            = argbToImColor(scheme.GetSecondaryFixed());
    colors.on_secondary_fixed         = argbToImColor(scheme.GetOnSecondaryFixed());
    colors.secondary_fixed_dim        = argbToImColor(scheme.GetSecondaryFixedDim());
    colors.on_secondary_fixed_variant = argbToImColor(scheme.GetOnSecondaryFixedVariant());
    colors.tertiary_fixed             = argbToImColor(scheme.GetTertiaryFixed());
    colors.on_tertiary_fixed          = argbToImColor(scheme.GetOnTertiaryFixed());
    colors.tertiary_fixed_dim         = argbToImColor(scheme.GetTertiaryFixedDim());
    colors.on_tertiary_fixed_variant  = argbToImColor(scheme.GetOnTertiaryFixedVariant());
    colors.surface_dim                = argbToImColor(scheme.GetSurfaceDim());
    colors.surface_bright             = argbToImColor(scheme.GetSurfaceBright());
    colors.surface_container_lowest   = argbToImColor(scheme.GetSurfaceContainerLowest());
    colors.surface_container_low      = argbToImColor(scheme.GetSurfaceContainerLow());
    colors.surface_container          = argbToImColor(scheme.GetSurfaceContainer());
    colors.surface_container_high     = argbToImColor(scheme.GetSurfaceContainerHigh());
    colors.surface_container_highest  = argbToImColor(scheme.GetSurfaceContainerHighest());
}
}
}