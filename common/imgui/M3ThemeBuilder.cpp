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

    colors.seedArgb = seedArgb;
    colors.darkMode = isDark;

    // clang-format off
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::primary)]                 = argbToImColor(scheme.GetPrimary());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceTint)]             = argbToImColor(scheme.GetSurfaceTint());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::primaryContainer)]        = argbToImColor(scheme.GetPrimaryContainer());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::secondary)]               = argbToImColor(scheme.GetSecondary());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::secondaryContainer)]      = argbToImColor(scheme.GetSecondaryContainer());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::tertiary)]                = argbToImColor(scheme.GetTertiary());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::tertiaryContainer)]       = argbToImColor(scheme.GetTertiaryContainer());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::error)]                   = argbToImColor(scheme.GetError());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::errorContainer)]          = argbToImColor(scheme.GetErrorContainer());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::background)]              = argbToImColor(scheme.GetBackground());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surface)]                 = argbToImColor(scheme.GetSurface());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceVariant)]          = argbToImColor(scheme.GetSurfaceVariant());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::outline)]                 = argbToImColor(scheme.GetOutline());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::outlineVariant)]          = argbToImColor(scheme.GetOutlineVariant());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::shadow)]                  = argbToImColor(scheme.GetShadow());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::scrim)]                   = argbToImColor(scheme.GetScrim());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::inverseSurface)]          = argbToImColor(scheme.GetInverseSurface());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::inversePrimary)]          = argbToImColor(scheme.GetInversePrimary());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::primaryFixed)]            = argbToImColor(scheme.GetPrimaryFixed());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::primaryFixedDim)]         = argbToImColor(scheme.GetPrimaryFixedDim());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::secondaryFixed)]          = argbToImColor(scheme.GetSecondaryFixed());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::secondaryFixedDim)]       = argbToImColor(scheme.GetSecondaryFixedDim());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::tertiaryFixed)]           = argbToImColor(scheme.GetTertiaryFixed());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::tertiaryFixedDim)]        = argbToImColor(scheme.GetTertiaryFixedDim());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceDim)]              = argbToImColor(scheme.GetSurfaceDim());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceBright)]           = argbToImColor(scheme.GetSurfaceBright());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceContainerLowest)]  = argbToImColor(scheme.GetSurfaceContainerLowest());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceContainerLow)]     = argbToImColor(scheme.GetSurfaceContainerLow());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceContainer)]        = argbToImColor(scheme.GetSurfaceContainer());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceContainerHigh)]    = argbToImColor(scheme.GetSurfaceContainerHigh());
    colors.surfaceColors[static_cast<size_t>(SurfaceToken::surfaceContainerHighest)] = argbToImColor(scheme.GetSurfaceContainerHighest());

    colors.contentColors[static_cast<uint8_t>(ContentToken::onPrimary)]               = argbToImColor(scheme.GetOnPrimary());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onPrimaryContainer)]      = argbToImColor(scheme.GetOnPrimaryContainer());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onSecondary)]             = argbToImColor(scheme.GetOnSecondary());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onSecondaryContainer)]    = argbToImColor(scheme.GetOnSecondaryContainer());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onTertiary)]              = argbToImColor(scheme.GetOnTertiary());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onTertiaryContainer)]     = argbToImColor(scheme.GetOnTertiaryContainer());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onError)]                 = argbToImColor(scheme.GetOnError());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onErrorContainer)]        = argbToImColor(scheme.GetOnErrorContainer());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onBackground)]            = argbToImColor(scheme.GetOnBackground());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onSurface)]               = argbToImColor(scheme.GetOnSurface());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onSurfaceVariant)]        = argbToImColor(scheme.GetOnSurfaceVariant());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onPrimaryFixed)]          = argbToImColor(scheme.GetOnPrimaryFixed());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onPrimaryFixedVariant)]   = argbToImColor(scheme.GetOnPrimaryFixedVariant());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onSecondaryFixed)]        = argbToImColor(scheme.GetOnSecondaryFixed());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onSecondaryFixedVariant)] = argbToImColor(scheme.GetOnSecondaryFixedVariant());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onTertiaryFixed)]         = argbToImColor(scheme.GetOnTertiaryFixed());
    colors.contentColors[static_cast<uint8_t>(ContentToken::onTertiaryFixedVariant)]  = argbToImColor(scheme.GetOnTertiaryFixedVariant());
    colors.contentColors[static_cast<uint8_t>(ContentToken::inverseOnSurface)]        = argbToImColor(scheme.GetInverseOnSurface());
    // clang-format on

    colors.primaryPalette        = scheme.primary_palette.get(40.0f);
    colors.secondaryPalette      = scheme.secondary_palette.get(40.0f);
    colors.tertiaryPalette       = scheme.tertiary_palette.get(40.0f);
    colors.neutralPalette        = scheme.neutral_palette.get(40.0f);
    colors.neutralVariantPalette = scheme.neutral_variant_palette.get(40.0f);
    colors.errorPalette          = scheme.error_palette.get(40.0f);
}
}
}