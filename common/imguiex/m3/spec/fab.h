//
// Created by jamie on 2026/2/24.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

#include <cassert>

namespace ImGuiEx::M3::Spec
{

enum class FabColors : std::uint8_t
{
    TonalPrimary,
    TonalSecondary,
    TonalTertiary,
    TonalPrimaryContainer,
    TonalSecondaryContainer,
    TonalTertiaryContainer,
};

template <FabColors FabColor>
struct Fab;

template <FabColors FabColor>
struct FabContainer;

template <>
struct Fab<FabColors::TonalPrimary>
{
    //! FAB primary container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! FAB primary icon color
    static constexpr auto IconColor = ColorRole::onPrimary;

    //! FAB primary container color
    static constexpr auto ContainerColor = ColorRole::primary;
};

template <>
struct Fab<FabColors::TonalSecondary>
{
    //! FAB secondary container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! FAB secondary icon color
    static constexpr auto IconColor = ColorRole::onSecondary;

    //! FAB secondary container color
    static constexpr auto ContainerColor = ColorRole::secondary;
};

template <>
struct Fab<FabColors::TonalTertiary>
{
    //! FAB tertiary container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! FAB tertiary icon color
    static constexpr auto IconColor = ColorRole::onTertiary;

    //! FAB tertiary container color
    static constexpr auto ContainerColor = ColorRole::tertiary;
};

template <>
struct Fab<FabColors::TonalPrimaryContainer>
{
    //! FAB tonal primary container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! FAB tonal primary icon color
    static constexpr auto IconColor = ColorRole::onPrimaryContainer;

    //! FAB tonal primary container color
    static constexpr auto ContainerColor = ColorRole::primaryContainer;
};

template <>
struct Fab<FabColors::TonalSecondaryContainer>
{
    //! FAB tonal secondary container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! FAB tonal secondary icon color
    static constexpr auto IconColor = ColorRole::onSecondaryContainer;

    //! FAB tonal secondary container color
    static constexpr auto ContainerColor = ColorRole::secondaryContainer;
};

template <>
struct Fab<FabColors::TonalTertiaryContainer>
{
    //! FAB tonal tertiary container shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! FAB tonal tertiary icon color
    static constexpr auto IconColor = ColorRole::onTertiaryContainer;

    //! FAB tonal tertiary container color
    static constexpr auto ContainerColor = ColorRole::tertiaryContainer;
};

template <SizeTips Size>
struct FabSizing;

template <>
struct FabSizing<SizeTips::MEDIUM>
{
    //! FAB medium container shape
    static constexpr auto ContainerShape = ShapeCorner::LargeIncreased;

    //! FAB medium icon size
    static constexpr auto IconSize = dp<28>();

    //! FAB medium container width
    static constexpr auto ContainerWidth = dp<80>();

    //! FAB medium container height
    static constexpr auto ContainerHeight = dp<80>();
};

template <>
struct FabSizing<SizeTips::LARGE>
{
    //! FAB large container shape
    static constexpr auto ContainerShape = ShapeCorner::ExtraLarge;

    //! FAB large icon size
    static constexpr auto IconSize = dp<36>();

    //! FAB large container width
    static constexpr auto ContainerWidth = dp<96>();

    //! FAB large container height
    static constexpr auto ContainerHeight = dp<96>();
};

struct FabSizingValues
{
    Spec::Unit containerShape;
    Spec::Unit iconSize;
    Spec::Unit containerWidth;
    Spec::Unit containerHeight;
};

constexpr auto GetFabSizingValues(SizeTips size) -> FabSizingValues
{
    FabSizingValues values{};
    switch (size)
    {
        case SizeTips::MEDIUM: {
            values.containerShape  = FabSizing<SizeTips::MEDIUM>::ContainerShape;
            values.iconSize        = FabSizing<SizeTips::MEDIUM>::IconSize;
            values.containerWidth  = FabSizing<SizeTips::MEDIUM>::ContainerWidth;
            values.containerHeight = FabSizing<SizeTips::MEDIUM>::ContainerHeight;
            break;
        }
        case SizeTips::LARGE:
            values.containerShape  = FabSizing<SizeTips::LARGE>::ContainerShape;
            values.iconSize        = FabSizing<SizeTips::LARGE>::IconSize;
            values.containerWidth  = FabSizing<SizeTips::LARGE>::ContainerWidth;
            values.containerHeight = FabSizing<SizeTips::LARGE>::ContainerHeight;
            break;
        default:
            assert(0 && "Invalid size for FAB, only medium and large are allowed.");
            break;
    }
    return values;
}

struct FabColorsValues
{
    ColorRole containerColor;
    ColorRole iconColor;
};

constexpr auto GetFabColorsValue(FabColors FabColor) -> FabColorsValues
{
    FabColorsValues values{};
    switch (FabColor)
    {
        using enum FabColors;
        case TonalPrimary:
            values.containerColor = Fab<TonalPrimary>::ContainerColor;
            values.iconColor      = Fab<TonalPrimary>::IconColor;
            break;
        case TonalSecondary:
            values.containerColor = Fab<TonalSecondary>::ContainerColor;
            values.iconColor      = Fab<TonalSecondary>::IconColor;
            break;
        case TonalTertiary:
            values.containerColor = Fab<TonalTertiary>::ContainerColor;
            values.iconColor      = Fab<TonalTertiary>::IconColor;
            break;
        case TonalPrimaryContainer:
            values.containerColor = Fab<TonalPrimaryContainer>::ContainerColor;
            values.iconColor      = Fab<TonalPrimaryContainer>::IconColor;
            break;
        case TonalSecondaryContainer:
            values.containerColor = Fab<TonalSecondaryContainer>::ContainerColor;
            values.iconColor      = Fab<TonalSecondaryContainer>::IconColor;
            break;
        case TonalTertiaryContainer:
            values.containerColor = Fab<TonalTertiaryContainer>::ContainerColor;
            values.iconColor      = Fab<TonalTertiaryContainer>::IconColor;
            break;
    }
    return values;
}

} // namespace ImGuiEx::M3::Spec
