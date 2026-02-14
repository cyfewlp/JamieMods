//
// Created by jamie on 2026/1/27.
//

#include "M3ThemeBuilder.h"

#include "Material3.h"
#include "cpp/scheme/scheme_tonal_spot.h"

namespace ImGuiEx::M3
{
using Scheme = material_color_utilities::SchemeTonalSpot;
using Hct    = material_color_utilities::Hct;

namespace
{
void FillSurfaceColors(ColorScheme::SurfaceColors &surfaceColors, const Scheme &scheme)
{
    // clang-format off
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primary))                = ArgbToImVec4(scheme.GetPrimary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceTint))            = ArgbToImVec4(scheme.GetSurfaceTint());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryContainer))       = ArgbToImVec4(scheme.GetPrimaryContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondary))              = ArgbToImVec4(scheme.GetSecondary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryContainer))     = ArgbToImVec4(scheme.GetSecondaryContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiary))               = ArgbToImVec4(scheme.GetTertiary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryContainer))      = ArgbToImVec4(scheme.GetTertiaryContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::error))                  = ArgbToImVec4(scheme.GetError());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::errorContainer))         = ArgbToImVec4(scheme.GetErrorContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::background))             = ArgbToImVec4(scheme.GetBackground());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surface))                = ArgbToImVec4(scheme.GetSurface());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceVariant))         = ArgbToImVec4(scheme.GetSurfaceVariant());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::outline))                = ArgbToImVec4(scheme.GetOutline());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::outlineVariant))         = ArgbToImVec4(scheme.GetOutlineVariant());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::shadow))                 = ArgbToImVec4(scheme.GetShadow());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::scrim))                  = ArgbToImVec4(scheme.GetScrim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::inverseSurface))         = ArgbToImVec4(scheme.GetInverseSurface());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::inversePrimary))         = ArgbToImVec4(scheme.GetInversePrimary());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryFixed))           = ArgbToImVec4(scheme.GetPrimaryFixed());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::primaryFixedDim))        = ArgbToImVec4(scheme.GetPrimaryFixedDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryFixed))         = ArgbToImVec4(scheme.GetSecondaryFixed());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::secondaryFixedDim))      = ArgbToImVec4(scheme.GetSecondaryFixedDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryFixed))          = ArgbToImVec4(scheme.GetTertiaryFixed());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::tertiaryFixedDim))       = ArgbToImVec4(scheme.GetTertiaryFixedDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceDim))             = ArgbToImVec4(scheme.GetSurfaceDim());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceBright))          = ArgbToImVec4(scheme.GetSurfaceBright());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerLowest)) = ArgbToImVec4(scheme.GetSurfaceContainerLowest());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerLow))    = ArgbToImVec4(scheme.GetSurfaceContainerLow());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainer))       = ArgbToImVec4(scheme.GetSurfaceContainer());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerHigh))   = ArgbToImVec4(scheme.GetSurfaceContainerHigh());
    surfaceColors.at(static_cast<size_t>(SurfaceToken::surfaceContainerHighest))= ArgbToImVec4(scheme.GetSurfaceContainerHighest());
    // clang-format on
}

void FillContentColors(ColorScheme::ContentColors &contentColors, const Scheme &scheme)
{
    // clang-format off
    contentColors.at(static_cast<size_t>(ContentToken::onPrimary))              = ArgbToImVec4(scheme.GetOnPrimary());
    contentColors.at(static_cast<size_t>(ContentToken::onPrimaryContainer))     = ArgbToImVec4(scheme.GetOnPrimaryContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondary))            = ArgbToImVec4(scheme.GetOnSecondary());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondaryContainer))   = ArgbToImVec4(scheme.GetOnSecondaryContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiary))             = ArgbToImVec4(scheme.GetOnTertiary());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiaryContainer))    = ArgbToImVec4(scheme.GetOnTertiaryContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onError))                = ArgbToImVec4(scheme.GetOnError());
    contentColors.at(static_cast<size_t>(ContentToken::onErrorContainer))       = ArgbToImVec4(scheme.GetOnErrorContainer());
    contentColors.at(static_cast<size_t>(ContentToken::onBackground))           = ArgbToImVec4(scheme.GetOnBackground());
    contentColors.at(static_cast<size_t>(ContentToken::onSurface))              = ArgbToImVec4(scheme.GetOnSurface());
    contentColors.at(static_cast<size_t>(ContentToken::onSurfaceVariant))       = ArgbToImVec4(scheme.GetOnSurfaceVariant());
    contentColors.at(static_cast<size_t>(ContentToken::onPrimaryFixed))         = ArgbToImVec4(scheme.GetOnPrimaryFixed());
    contentColors.at(static_cast<size_t>(ContentToken::onPrimaryFixedVariant))  = ArgbToImVec4(scheme.GetOnPrimaryFixedVariant());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondaryFixed))       = ArgbToImVec4(scheme.GetOnSecondaryFixed());
    contentColors.at(static_cast<size_t>(ContentToken::onSecondaryFixedVariant))= ArgbToImVec4(scheme.GetOnSecondaryFixedVariant());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiaryFixed))        = ArgbToImVec4(scheme.GetOnTertiaryFixed());
    contentColors.at(static_cast<size_t>(ContentToken::onTertiaryFixedVariant)) = ArgbToImVec4(scheme.GetOnTertiaryFixedVariant());
    contentColors.at(static_cast<size_t>(ContentToken::inverseOnSurface))       = ArgbToImVec4(scheme.GetInverseOnSurface());
    // clang-format on
}
} // namespace

auto ThemeBuilder::Build(const ColorScheme::SchemeConfig &config) -> ColorScheme
{
    ColorScheme::SurfaceColors surfaceColors;
    ColorScheme::ContentColors contentColors;

    const Scheme scheme(Hct(config.sourceColor), config.darkMode, config.contrastLevel);
    FillSurfaceColors(surfaceColors, scheme);
    FillContentColors(contentColors, scheme);
    return ColorScheme(config, std::move(surfaceColors), std::move(contentColors));
}
} // namespace ImGuiEx::M3
