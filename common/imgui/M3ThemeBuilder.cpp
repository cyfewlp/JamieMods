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

    colors.seedArgb                = seedArgb;
    colors.primary                 = argbToImColor(scheme.GetPrimary());
    colors.surfaceTint             = argbToImColor(scheme.GetSurfaceTint());
    colors.onPrimary               = argbToImColor(scheme.GetOnPrimary());
    colors.primaryContainer        = argbToImColor(scheme.GetPrimaryContainer());
    colors.onPrimaryContainer      = argbToImColor(scheme.GetOnPrimaryContainer());
    colors.secondary               = argbToImColor(scheme.GetSecondary());
    colors.onSecondary             = argbToImColor(scheme.GetOnSecondary());
    colors.secondaryContainer      = argbToImColor(scheme.GetSecondaryContainer());
    colors.onSecondaryContainer    = argbToImColor(scheme.GetOnSecondaryContainer());
    colors.tertiary                = argbToImColor(scheme.GetTertiary());
    colors.onTertiary              = argbToImColor(scheme.GetOnTertiary());
    colors.tertiaryContainer       = argbToImColor(scheme.GetTertiaryContainer());
    colors.onTertiaryContainer     = argbToImColor(scheme.GetOnTertiaryContainer());
    colors.error                   = argbToImColor(scheme.GetError());
    colors.onError                 = argbToImColor(scheme.GetOnError());
    colors.errorContainer          = argbToImColor(scheme.GetErrorContainer());
    colors.onErrorContainer        = argbToImColor(scheme.GetOnErrorContainer());
    colors.background              = argbToImColor(scheme.GetBackground());
    colors.onBackground            = argbToImColor(scheme.GetOnBackground());
    colors.surface                 = argbToImColor(scheme.GetSurface());
    colors.onSurface               = argbToImColor(scheme.GetOnSurface());
    colors.surfaceVariant          = argbToImColor(scheme.GetSurfaceVariant());
    colors.onSurfaceVariant        = argbToImColor(scheme.GetOnSurfaceVariant());
    colors.outline                 = argbToImColor(scheme.GetOutline());
    colors.outlineVariant          = argbToImColor(scheme.GetOutlineVariant());
    colors.shadow                  = argbToImColor(scheme.GetShadow());
    colors.scrim                   = argbToImColor(scheme.GetScrim());
    colors.inverseSurface          = argbToImColor(scheme.GetInverseSurface());
    colors.inverseOnSurface        = argbToImColor(scheme.GetInverseOnSurface());
    colors.inversePrimary          = argbToImColor(scheme.GetInversePrimary());
    colors.primaryFixed            = argbToImColor(scheme.GetPrimaryFixed());
    colors.onPrimaryFixed          = argbToImColor(scheme.GetOnPrimaryFixed());
    colors.primaryFixedDim         = argbToImColor(scheme.GetPrimaryFixedDim());
    colors.onPrimaryFixedVariant   = argbToImColor(scheme.GetOnPrimaryFixedVariant());
    colors.secondaryFixed          = argbToImColor(scheme.GetSecondaryFixed());
    colors.onSecondaryFixed        = argbToImColor(scheme.GetOnSecondaryFixed());
    colors.secondaryFixedDim       = argbToImColor(scheme.GetSecondaryFixedDim());
    colors.onSecondaryFixedVariant = argbToImColor(scheme.GetOnSecondaryFixedVariant());
    colors.tertiaryFixed           = argbToImColor(scheme.GetTertiaryFixed());
    colors.onTertiaryFixed         = argbToImColor(scheme.GetOnTertiaryFixed());
    colors.tertiaryFixedDim        = argbToImColor(scheme.GetTertiaryFixedDim());
    colors.onTertiaryFixedVariant  = argbToImColor(scheme.GetOnTertiaryFixedVariant());
    colors.surfaceDim              = argbToImColor(scheme.GetSurfaceDim());
    colors.surfaceBright           = argbToImColor(scheme.GetSurfaceBright());
    colors.surfaceContainerLowest  = argbToImColor(scheme.GetSurfaceContainerLowest());
    colors.surfaceContainerLow     = argbToImColor(scheme.GetSurfaceContainerLow());
    colors.surfaceContainer        = argbToImColor(scheme.GetSurfaceContainer());
    colors.surfaceContainerHigh    = argbToImColor(scheme.GetSurfaceContainerHigh());
    colors.surfaceContainerHighest = argbToImColor(scheme.GetSurfaceContainerHighest());

    colors.primaryPalette        = scheme.primary_palette.get(40.0f);
    colors.secondaryPalette      = scheme.secondary_palette.get(40.0f);
    colors.tertiaryPalette       = scheme.tertiary_palette.get(40.0f);
    colors.neutralPalette        = scheme.neutral_palette.get(40.0f);
    colors.neutralVariantPalette = scheme.neutral_variant_palette.get(40.0f);
    colors.errorPalette          = scheme.error_palette.get(40.0f);
}
}
}