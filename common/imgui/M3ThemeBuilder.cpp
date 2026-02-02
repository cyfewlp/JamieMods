//
// Created by jamie on 2026/1/27.
//

#include "M3ThemeBuilder.h"

#include "Material3.h"
#include "cpp/scheme/scheme_tonal_spot.h"

#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{
using Argb = material_color_utilities::Argb;

auto ToColorFloat4 = [](const Argb argb) -> ImVec4 {
    const uint8_t a = (argb & ARGB_A_MASK) >> ARGB_A_SHIFT;
    const uint8_t r = (argb & ARGB_R_MASK) >> ARGB_R_SHIFT;
    const uint8_t g = (argb & ARGB_G_MASK) >> ARGB_G_SHIFT;
    const uint8_t b = argb & ARGB_B_MASK;
    return ImColor(r, g, b, a);
};

void ThemeBuilder::BuildThemeFromSeed(const Argb sourceColor, const bool isDark, Colors &colors)
{
    const Scheme scheme(Hct(sourceColor), isDark, 0.0);
    colors.seedArgb = sourceColor;
    colors.darkMode = isDark;

    FillSurfaceColors(colors.surfaceColors, scheme);
    FillContentColors(colors.contentColors, scheme);
}

auto ThemeBuilder::BuildThemeFromSeed(uint32_t sourceColor, bool isDark) -> Colors
{
    Colors::SurfaceColors surfaceColors;
    Colors::ContentColors contentColors;

    const Scheme scheme(Hct(sourceColor), isDark, 0.0);
    FillSurfaceColors(surfaceColors, scheme);
    FillContentColors(contentColors, scheme);
    return Colors(sourceColor, isDark, std::move(surfaceColors), std::move(contentColors));
}

void ThemeBuilder::FillSurfaceColors(Colors::SurfaceColors &surfaceColors, const Scheme &scheme)
{
    // clang-format off
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primary))                = ToColorFloat4(scheme.GetPrimary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceTint))            = ToColorFloat4(scheme.GetSurfaceTint());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryContainer))       = ToColorFloat4(scheme.GetPrimaryContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondary))              = ToColorFloat4(scheme.GetSecondary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryContainer))     = ToColorFloat4(scheme.GetSecondaryContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiary))               = ToColorFloat4(scheme.GetTertiary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryContainer))      = ToColorFloat4(scheme.GetTertiaryContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::error))                  = ToColorFloat4(scheme.GetError());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::errorContainer))         = ToColorFloat4(scheme.GetErrorContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::background))             = ToColorFloat4(scheme.GetBackground());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surface))                = ToColorFloat4(scheme.GetSurface());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceVariant))         = ToColorFloat4(scheme.GetSurfaceVariant());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::outline))                = ToColorFloat4(scheme.GetOutline());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::outlineVariant))         = ToColorFloat4(scheme.GetOutlineVariant());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::shadow))                 = ToColorFloat4(scheme.GetShadow());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::scrim))                  = ToColorFloat4(scheme.GetScrim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::inverseSurface))         = ToColorFloat4(scheme.GetInverseSurface());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::inversePrimary))         = ToColorFloat4(scheme.GetInversePrimary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryFixed))           = ToColorFloat4(scheme.GetPrimaryFixed());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryFixedDim))        = ToColorFloat4(scheme.GetPrimaryFixedDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryFixed))         = ToColorFloat4(scheme.GetSecondaryFixed());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryFixedDim))      = ToColorFloat4(scheme.GetSecondaryFixedDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryFixed))          = ToColorFloat4(scheme.GetTertiaryFixed());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryFixedDim))       = ToColorFloat4(scheme.GetTertiaryFixedDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceDim))             = ToColorFloat4(scheme.GetSurfaceDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceBright))          = ToColorFloat4(scheme.GetSurfaceBright());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerLowest)) = ToColorFloat4(scheme.GetSurfaceContainerLowest());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerLow))    = ToColorFloat4(scheme.GetSurfaceContainerLow());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainer))       = ToColorFloat4(scheme.GetSurfaceContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerHigh))   = ToColorFloat4(scheme.GetSurfaceContainerHigh());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerHighest))= ToColorFloat4(scheme.GetSurfaceContainerHighest());
    // clang-format on
}

void ThemeBuilder::FillContentColors(Colors::ContentColors &contentColors, const Scheme &scheme)
{
    // clang-format off
    contentColors.at(static_cast<size_t>(ContentToken::onPrimary))              = ToColorFloat4(scheme.GetOnPrimary());
    contentColors.at(static_cast<size_t>(ContentToken::onPrimaryContainer))     = ToColorFloat4(scheme.GetOnPrimaryContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondary))            = ToColorFloat4(scheme.GetOnSecondary());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondaryContainer))   = ToColorFloat4(scheme.GetOnSecondaryContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiary))             = ToColorFloat4(scheme.GetOnTertiary());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiaryContainer))    = ToColorFloat4(scheme.GetOnTertiaryContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onError))                = ToColorFloat4(scheme.GetOnError());
    contentColors.at(static_cast<size_t>(ContentToken::onErrorContainer))       = ToColorFloat4(scheme.GetOnErrorContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onBackground))           = ToColorFloat4(scheme.GetOnBackground());
    contentColors.at(static_cast<size_t>(ContentToken::onSurface))              = ToColorFloat4(scheme.GetOnSurface());
    contentColors.at(static_cast<size_t>(ContentToken::onSurfaceVariant))       = ToColorFloat4(scheme.GetOnSurfaceVariant());
    contentColors.at(static_cast<size_t>(ContentToken::onPrimaryFixed))         = ToColorFloat4(scheme.GetOnPrimaryFixed());
    contentColors.at(static_cast<size_t>(ContentToken::onPrimaryFixedVariant))  = ToColorFloat4(scheme.GetOnPrimaryFixedVariant());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondaryFixed))       = ToColorFloat4(scheme.GetOnSecondaryFixed());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondaryFixedVariant))= ToColorFloat4(scheme.GetOnSecondaryFixedVariant());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiaryFixed))        = ToColorFloat4(scheme.GetOnTertiaryFixed());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiaryFixedVariant)) = ToColorFloat4(scheme.GetOnTertiaryFixedVariant());
    contentColors.at(static_cast<size_t>(ContentToken::inverseOnSurface))       = ToColorFloat4(scheme.GetInverseOnSurface());
    // clang-format on
}
}
}