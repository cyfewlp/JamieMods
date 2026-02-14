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
void FillSurfaceColors(ColorScheme::Colors &colors, const Scheme &scheme)
{
    // clang-format off
    colors.at(static_cast<size_t>(ColorRole::primary))                = ArgbToImVec4(scheme.GetPrimary());
    colors.at(static_cast<size_t>(ColorRole::surfaceTint))            = ArgbToImVec4(scheme.GetSurfaceTint());
    colors.at(static_cast<size_t>(ColorRole::primaryContainer))       = ArgbToImVec4(scheme.GetPrimaryContainer());
    colors.at(static_cast<size_t>(ColorRole::secondary))              = ArgbToImVec4(scheme.GetSecondary());
    colors.at(static_cast<size_t>(ColorRole::secondaryContainer))     = ArgbToImVec4(scheme.GetSecondaryContainer());
    colors.at(static_cast<size_t>(ColorRole::tertiary))               = ArgbToImVec4(scheme.GetTertiary());
    colors.at(static_cast<size_t>(ColorRole::tertiaryContainer))      = ArgbToImVec4(scheme.GetTertiaryContainer());
    colors.at(static_cast<size_t>(ColorRole::error))                  = ArgbToImVec4(scheme.GetError());
    colors.at(static_cast<size_t>(ColorRole::errorContainer))         = ArgbToImVec4(scheme.GetErrorContainer());
    colors.at(static_cast<size_t>(ColorRole::background))             = ArgbToImVec4(scheme.GetBackground());
    colors.at(static_cast<size_t>(ColorRole::surface))                = ArgbToImVec4(scheme.GetSurface());
    colors.at(static_cast<size_t>(ColorRole::surfaceVariant))         = ArgbToImVec4(scheme.GetSurfaceVariant());
    colors.at(static_cast<size_t>(ColorRole::outline))                = ArgbToImVec4(scheme.GetOutline());
    colors.at(static_cast<size_t>(ColorRole::outlineVariant))         = ArgbToImVec4(scheme.GetOutlineVariant());
    colors.at(static_cast<size_t>(ColorRole::shadow))                 = ArgbToImVec4(scheme.GetShadow());
    colors.at(static_cast<size_t>(ColorRole::scrim))                  = ArgbToImVec4(scheme.GetScrim());
    colors.at(static_cast<size_t>(ColorRole::inverseSurface))         = ArgbToImVec4(scheme.GetInverseSurface());
    colors.at(static_cast<size_t>(ColorRole::inversePrimary))         = ArgbToImVec4(scheme.GetInversePrimary());
    colors.at(static_cast<size_t>(ColorRole::primaryFixed))           = ArgbToImVec4(scheme.GetPrimaryFixed());
    colors.at(static_cast<size_t>(ColorRole::primaryFixedDim))        = ArgbToImVec4(scheme.GetPrimaryFixedDim());
    colors.at(static_cast<size_t>(ColorRole::secondaryFixed))         = ArgbToImVec4(scheme.GetSecondaryFixed());
    colors.at(static_cast<size_t>(ColorRole::secondaryFixedDim))      = ArgbToImVec4(scheme.GetSecondaryFixedDim());
    colors.at(static_cast<size_t>(ColorRole::tertiaryFixed))          = ArgbToImVec4(scheme.GetTertiaryFixed());
    colors.at(static_cast<size_t>(ColorRole::tertiaryFixedDim))       = ArgbToImVec4(scheme.GetTertiaryFixedDim());
    colors.at(static_cast<size_t>(ColorRole::surfaceDim))             = ArgbToImVec4(scheme.GetSurfaceDim());
    colors.at(static_cast<size_t>(ColorRole::surfaceBright))          = ArgbToImVec4(scheme.GetSurfaceBright());
    colors.at(static_cast<size_t>(ColorRole::surfaceContainerLowest)) = ArgbToImVec4(scheme.GetSurfaceContainerLowest());
    colors.at(static_cast<size_t>(ColorRole::surfaceContainerLow))    = ArgbToImVec4(scheme.GetSurfaceContainerLow());
    colors.at(static_cast<size_t>(ColorRole::surfaceContainer))       = ArgbToImVec4(scheme.GetSurfaceContainer());
    colors.at(static_cast<size_t>(ColorRole::surfaceContainerHigh))   = ArgbToImVec4(scheme.GetSurfaceContainerHigh());
    colors.at(static_cast<size_t>(ColorRole::surfaceContainerHighest))= ArgbToImVec4(scheme.GetSurfaceContainerHighest());
    // clang-format on
}

void FillContentColors(ColorScheme::Colors &colors, const Scheme &scheme)
{
    // clang-format off
    colors.at(static_cast<size_t>(ColorRole::onPrimary))              = ArgbToImVec4(scheme.GetOnPrimary());
    colors.at(static_cast<size_t>(ColorRole::onPrimaryContainer))     = ArgbToImVec4(scheme.GetOnPrimaryContainer());
    colors.at(static_cast<size_t>(ColorRole::onSecondary))            = ArgbToImVec4(scheme.GetOnSecondary());
    colors.at(static_cast<size_t>(ColorRole::onSecondaryContainer))   = ArgbToImVec4(scheme.GetOnSecondaryContainer());
    colors.at(static_cast<size_t>(ColorRole::onTertiary))             = ArgbToImVec4(scheme.GetOnTertiary());
    colors.at(static_cast<size_t>(ColorRole::onTertiaryContainer))    = ArgbToImVec4(scheme.GetOnTertiaryContainer());
    colors.at(static_cast<size_t>(ColorRole::onError))                = ArgbToImVec4(scheme.GetOnError());
    colors.at(static_cast<size_t>(ColorRole::onErrorContainer))       = ArgbToImVec4(scheme.GetOnErrorContainer());
    colors.at(static_cast<size_t>(ColorRole::onBackground))           = ArgbToImVec4(scheme.GetOnBackground());
    colors.at(static_cast<size_t>(ColorRole::onSurface))              = ArgbToImVec4(scheme.GetOnSurface());
    colors.at(static_cast<size_t>(ColorRole::onSurfaceVariant))       = ArgbToImVec4(scheme.GetOnSurfaceVariant());
    colors.at(static_cast<size_t>(ColorRole::onPrimaryFixed))         = ArgbToImVec4(scheme.GetOnPrimaryFixed());
    colors.at(static_cast<size_t>(ColorRole::onPrimaryFixedVariant))  = ArgbToImVec4(scheme.GetOnPrimaryFixedVariant());
    colors.at(static_cast<size_t>(ColorRole::onSecondaryFixed))       = ArgbToImVec4(scheme.GetOnSecondaryFixed());
    colors.at(static_cast<size_t>(ColorRole::onSecondaryFixedVariant))= ArgbToImVec4(scheme.GetOnSecondaryFixedVariant());
    colors.at(static_cast<size_t>(ColorRole::onTertiaryFixed))        = ArgbToImVec4(scheme.GetOnTertiaryFixed());
    colors.at(static_cast<size_t>(ColorRole::onTertiaryFixedVariant)) = ArgbToImVec4(scheme.GetOnTertiaryFixedVariant());
    colors.at(static_cast<size_t>(ColorRole::inverseOnSurface))       = ArgbToImVec4(scheme.GetInverseOnSurface());
    // clang-format on
}
} // namespace

auto ThemeBuilder::Build(const ColorScheme::SchemeConfig &config) -> ColorScheme
{
    ColorScheme::Colors colors;

    const Scheme scheme(Hct(config.sourceColor), config.darkMode, config.contrastLevel);
    FillSurfaceColors(colors, scheme);
    FillContentColors(colors, scheme);
    return ColorScheme(std::move(colors), config);
}
} // namespace ImGuiEx::M3
