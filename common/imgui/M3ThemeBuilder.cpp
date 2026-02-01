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

void ThemeBuilder::BuildThemeFromSeed(const Argb sourceColor, const bool isDark, Colors &colors)
{
    using Scheme = material_color_utilities::SchemeTonalSpot;
    using Hct    = material_color_utilities::Hct;
    const Scheme scheme(Hct(sourceColor), isDark, 0.0);

    auto ToColorFloat4 = [](const Argb argb) -> ImVec4 {
        const uint8_t a = (argb & ARGB_A_MASK) >> ARGB_A_SHIFT;
        const uint8_t r = (argb & ARGB_R_MASK) >> ARGB_R_SHIFT;
        const uint8_t g = (argb & ARGB_G_MASK) >> ARGB_G_SHIFT;
        const uint8_t b = argb & ARGB_B_MASK;
        return ImColor(r, g, b, a);
    };

    colors.seedArgb = sourceColor;
    colors.darkMode = isDark;

    // clang-format off
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::primary))                = ToColorFloat4(scheme.GetPrimary());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceTint))            = ToColorFloat4(scheme.GetSurfaceTint());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryContainer))       = ToColorFloat4(scheme.GetPrimaryContainer());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::secondary))              = ToColorFloat4(scheme.GetSecondary());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryContainer))     = ToColorFloat4(scheme.GetSecondaryContainer());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiary))               = ToColorFloat4(scheme.GetTertiary());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryContainer))      = ToColorFloat4(scheme.GetTertiaryContainer());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::error))                  = ToColorFloat4(scheme.GetError());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::errorContainer))         = ToColorFloat4(scheme.GetErrorContainer());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::background))             = ToColorFloat4(scheme.GetBackground());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surface))                = ToColorFloat4(scheme.GetSurface());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceVariant))         = ToColorFloat4(scheme.GetSurfaceVariant());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::outline))                = ToColorFloat4(scheme.GetOutline());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::outlineVariant))         = ToColorFloat4(scheme.GetOutlineVariant());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::shadow))                 = ToColorFloat4(scheme.GetShadow());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::scrim))                  = ToColorFloat4(scheme.GetScrim());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::inverseSurface))         = ToColorFloat4(scheme.GetInverseSurface());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::inversePrimary))         = ToColorFloat4(scheme.GetInversePrimary());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryFixed))           = ToColorFloat4(scheme.GetPrimaryFixed());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryFixedDim))        = ToColorFloat4(scheme.GetPrimaryFixedDim());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryFixed))         = ToColorFloat4(scheme.GetSecondaryFixed());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryFixedDim))      = ToColorFloat4(scheme.GetSecondaryFixedDim());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryFixed))          = ToColorFloat4(scheme.GetTertiaryFixed());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryFixedDim))       = ToColorFloat4(scheme.GetTertiaryFixedDim());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceDim))             = ToColorFloat4(scheme.GetSurfaceDim());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceBright))          = ToColorFloat4(scheme.GetSurfaceBright());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerLowest)) = ToColorFloat4(scheme.GetSurfaceContainerLowest());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerLow))    = ToColorFloat4(scheme.GetSurfaceContainerLow());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainer))       = ToColorFloat4(scheme.GetSurfaceContainer());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerHigh))   = ToColorFloat4(scheme.GetSurfaceContainerHigh());
    colors.surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerHighest))= ToColorFloat4(scheme.GetSurfaceContainerHighest());

    colors.contentColors.at(static_cast<size_t>(ContentToken::onPrimary))              = ToColorFloat4(scheme.GetOnPrimary());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onPrimaryContainer))     = ToColorFloat4(scheme.GetOnPrimaryContainer());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onSecondary))            = ToColorFloat4(scheme.GetOnSecondary());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onSecondaryContainer))   = ToColorFloat4(scheme.GetOnSecondaryContainer());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onTertiary))             = ToColorFloat4(scheme.GetOnTertiary());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onTertiaryContainer))    = ToColorFloat4(scheme.GetOnTertiaryContainer());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onError))                = ToColorFloat4(scheme.GetOnError());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onErrorContainer))       = ToColorFloat4(scheme.GetOnErrorContainer());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onBackground))           = ToColorFloat4(scheme.GetOnBackground());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onSurface))              = ToColorFloat4(scheme.GetOnSurface());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onSurfaceVariant))       = ToColorFloat4(scheme.GetOnSurfaceVariant());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onPrimaryFixed))         = ToColorFloat4(scheme.GetOnPrimaryFixed());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onPrimaryFixedVariant))  = ToColorFloat4(scheme.GetOnPrimaryFixedVariant());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onSecondaryFixed))       = ToColorFloat4(scheme.GetOnSecondaryFixed());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onSecondaryFixedVariant))= ToColorFloat4(scheme.GetOnSecondaryFixedVariant());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onTertiaryFixed))        = ToColorFloat4(scheme.GetOnTertiaryFixed());
    colors.contentColors.at(static_cast<size_t>(ContentToken::onTertiaryFixedVariant)) = ToColorFloat4(scheme.GetOnTertiaryFixedVariant());
    colors.contentColors.at(static_cast<size_t>(ContentToken::inverseOnSurface))       = ToColorFloat4(scheme.GetInverseOnSurface());
    // clang-format on
}
}
}