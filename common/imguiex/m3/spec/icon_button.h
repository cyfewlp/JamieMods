//
// Created by jamie on 2026/2/23.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{

enum class IconButtonColors : std::uint8_t
{
    Filled,   ///< Default, with container and icon color
    Tonal,    ///< Container color is a tonal variation of the icon color
    Outlined, ///< No container, only outline and icon color
    Standard  ///< No container, only icon color
};

template <States State>
struct IconButtonFilled;

template <>
struct IconButtonFilled<States::Enabled>
{
    //! Icon button filled container color
    static constexpr auto ContainerColor = ColorRole::primary;

    //! Icon button filled icon color
    static constexpr auto IconColor = ColorRole::onPrimary;

    //! Icon button filled container color - toggle (selected)
    static constexpr auto SelectedContainerColor = ColorRole::primary;

    //! Icon button filled icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::onPrimary;

    //! Icon button filled container color - toggle (unselected)
    static constexpr auto UnselectedContainerColor = ColorRole::surfaceContainer;

    //! Icon button filled icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSurfaceVariant;
};

using IconButtonEnabled = IconButtonFilled<States::Enabled>;

template <>
struct IconButtonFilled<States::Disabled>
{
    //! Icon button disabled container color
    static constexpr auto ContainerColor = ColorRole::onSurface;

    //! Icon button disabled container opacity
    static constexpr auto ContainerOpacity = 0.1F;

    //! Icon button disabled icon color
    static constexpr auto IconColor = ColorRole::onSurface;
};

template <>
struct IconButtonFilled<States::Pressed>
{
    //! Icon button shape pressed morph
    static constexpr auto ContainerShape = ShapeCorner::Small;
};

//-------------------------
// Tonal
//-------------------------

template <States State>
struct IconButtonTonal;

template <>
struct IconButtonTonal<States::Enabled>
{
    //! Icon button tonal icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::onSecondary;

    //! Icon button tonal icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSecondaryContainer;

    //! Icon button tonal icon color
    static constexpr auto IconColor = ColorRole::onSecondaryContainer;

    //! Icon button tonal container color - toggle (selected)
    static constexpr auto SelectedContainerColor = ColorRole::secondary;

    //! Icon button tonal container color - toggle (unselected)
    static constexpr auto UnselectedContainerColor = ColorRole::secondaryContainer;

    //! Icon button tonal container color
    static constexpr auto ContainerColor = ColorRole::secondaryContainer;
};

template <>
struct IconButtonTonal<States::Disabled>
{
    //! Icon button tonal disabled icon color
    static constexpr auto IconColor = ColorRole::onSurface;

    //! Icon button tonal disabled container color
    static constexpr auto ContainerColor = ColorRole::onSurface;

    //! Icon button tonal disabled container opacity
    static constexpr auto ContainerOpacity = 0.1F;
};

//-------------------------
// Outlined
//-------------------------

template <States State>
struct IconButtonOutlined;

template <>
struct IconButtonOutlined<States::Enabled>
{
    //! Icon button outlined color
    static constexpr auto OutlineColor = ColorRole::outlineVariant;

    //! Icon button outlined color - toggle (unselected)
    static constexpr auto UnselectedOutlineColor = ColorRole::outlineVariant;

    //! Icon button outlined icon color
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;

    //! Icon button outlined icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::inverseOnSurface;

    //! Icon button outlined icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSurfaceVariant;

    //! Icon button outlined container color - toggle (selected)
    static constexpr auto SelectedContainerColor = ColorRole::inverseSurface;
};

template <>
struct IconButtonOutlined<States::Disabled>
{
    //! Icon button outlined color
    static constexpr auto OutlineColor = ColorRole::outlineVariant;

    //! Icon button outlined disabled icon color
    static constexpr auto IconColor = ColorRole::onSurface;

    //! Icon button outlined disabled container color (selected)
    static constexpr auto SelectedContainerColor = ColorRole::onSurface;

    //! Icon button outlined disabled icon opacity
    static constexpr auto IconOpacity = 1.0F;
};

//-------------------------
// Standard
//-------------------------
template <States State>
struct IconButtonStandard;

template <>
struct IconButtonStandard<States::Enabled>
{
    //! Icon button icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::primary;

    //! Icon button icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSurfaceVariant;

    //! Icon button icon color
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;
};

template <>
struct IconButtonStandard<States::Disabled>
{
    //! Icon button disabled icon color
    static constexpr auto IconColor = ColorRole::onSurface;

    //! Icon button disabled opacity
    static constexpr auto IconOpacity = 1.0F;
};

template <SizeTips Size>
struct IconButtonSizing;

template <>
struct IconButtonSizing<SizeTips::XSMALL>
{
    //! Icon button xsmall container height
    static constexpr auto ContainerHeight = dp<32>();

    //! Icon button xsmall container shape round
    // static constexpr auto ContainerShapeRound = ShapeCorner::Full;

    //! Icon button xsmall container shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::Medium;

    //! Icon button xsmall default leading space
    static constexpr auto DefaultLeadingSpace = dp<6>();

    //! Icon button xsmall default trailing space
    static constexpr auto DefaultTrailingSpace = dp<6>();

    //! Icon button xsmall icon size
    static constexpr auto IconSize = dp<20>();

    //! Icon button xsmall narrow leading space
    static constexpr auto NarrowLeadingSpace = dp<4>();

    //! Icon button xsmall narrow trailing space
    static constexpr auto NarrowTrailingSpace = dp<4>();

    //! Icon button xsmall outline width
    static constexpr auto OutlinedOutlineWidth = 1.0F;

    //! Icon button xsmall shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Small;

    //! Icon button xsmall selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::Medium;

    //! Icon button xsmall selected container shape square
    // static constexpr auto SelectedContainerShapeSquare = ShapeCorner::Full;

    //! Icon button xsmall wide leading space
    static constexpr auto WideLeadingSpace = dp<10>();

    //! Icon button xsmall wide trailing space
    static constexpr auto WideTrailingSpace = dp<10>();
};

template <>
struct IconButtonSizing<SizeTips::SMALL>
{
    //! Icon button small container height
    static constexpr auto ContainerHeight = dp<40>();

    //! Icon button small container shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::Medium;

    //! Icon button small default leading space
    static constexpr auto DefaultLeadingSpace = dp<8>();

    //! Icon button small default trailing space
    static constexpr auto DefaultTrailingSpace = dp<8>();

    //! Icon button small icon size
    static constexpr auto IconSize = dp<24>();

    //! Icon button small narrow leading space
    static constexpr auto NarrowLeadingSpace = dp<4>();

    //! Icon button small narrow trailing space
    static constexpr auto NarrowTrailingSpace = dp<4>();

    //! Icon button small outline width
    static constexpr auto OutlinedOutlineWidth = 1.0F;

    //! Icon button small shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Small;

    //! Icon button small selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::Medium;

    //! Icon button small wide leading space
    static constexpr auto WideLeadingSpace = dp<14>();

    //! Icon button small wide trailing space
    static constexpr auto WideTrailingSpace = dp<14>();
};

template <>
struct IconButtonSizing<SizeTips::MEDIUM>
{
    //! Icon button medium container height
    static constexpr auto ContainerHeight = dp<56>();

    //! Icon button medium container shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::Large;

    //! Icon button medium default leading space
    static constexpr auto DefaultLeadingSpace = dp<16>();

    //! Icon button medium default trailing space
    static constexpr auto DefaultTrailingSpace = dp<16>();

    //! Icon button medium icon size
    static constexpr auto IconSize = dp<24>();

    //! Icon button medium narrow leading space
    static constexpr auto NarrowLeadingSpace = dp<12>();

    //! Icon button medium narrow trailing space
    static constexpr auto NarrowTrailingSpace = dp<12>();

    //! Icon button medium outline width
    static constexpr auto OutlinedOutlineWidth = 1.0F;

    //! Icon button medium shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Medium;

    //! Icon button medium selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::Large;

    //! Icon button medium wide leading space
    static constexpr auto WideLeadingSpace = dp<24>();

    //! Icon button medium wide trailing space
    static constexpr auto WideTrailingSpace = dp<24>();
};

template <>
struct IconButtonSizing<SizeTips::LARGE>
{
    //! Icon button large container height
    static constexpr auto ContainerHeight = dp<96>();

    //! Icon button large container shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::ExtraLarge;

    //! Icon button large default leading space
    static constexpr auto DefaultLeadingSpace = dp<32>();

    //! Icon button large default trailing space
    static constexpr auto DefaultTrailingSpace = dp<32>();

    //! Icon button large icon size
    static constexpr auto IconSize = dp<32>();

    //! Icon button large narrow leading space
    static constexpr auto NarrowLeadingSpace = dp<16>();

    //! Icon button large narrow trailing space
    static constexpr auto NarrowTrailingSpace = dp<16>();

    //! Icon button large outline width
    static constexpr auto OutlinedOutlineWidth = 2.0F;

    //! Icon button large shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Large;

    //! Icon button large selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::ExtraLarge;

    //! Icon button large wide leading space
    static constexpr auto WideLeadingSpace = dp<48>();

    //! Icon button large wide trailing space
    static constexpr auto WideTrailingSpace = dp<48>();
};

template <>
struct IconButtonSizing<SizeTips::XLARGE>
{
    //! Icon button xlarge container height
    static constexpr auto ContainerHeight = dp<136>();

    //! Icon button xlarge container shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::ExtraLarge;

    //! Icon button xlarge default leading space
    static constexpr auto DefaultLeadingSpace = dp<48>();

    //! Icon button xlarge default trailing space
    static constexpr auto DefaultTrailingSpace = dp<48>();

    //! Icon button xlarge icon size
    static constexpr auto IconSize = dp<40>();

    //! Icon button xlarge narrow leading space
    static constexpr auto NarrowLeadingSpace = dp<32>();

    //! Icon button xlarge narrow trailing space
    static constexpr auto NarrowTrailingSpace = dp<32>();

    //! Icon button xlarge outline width
    static constexpr auto OutlinedOutlineWidth = 3.0F;

    //! Icon button xlarge shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Large;

    //! Icon button xlarge selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::ExtraLarge;

    //! Icon button xlarge wide leading space
    static constexpr auto WideLeadingSpace = dp<72>();

    //! Icon button xlarge wide trailing space
    static constexpr auto WideTrailingSpace = dp<72>();
};

struct IconButtonSizingValues
{
    Spec::Unit containerHeight;
    Spec::Unit iconSize;
    Spec::Unit defaultLeadingSpace;
    Spec::Unit containerShapeSquare;
    Spec::Unit pressedContainerShape;
    float      outlinedOutlineWidth;
};

constexpr auto GetIconButtonSizing(Spec::SizeTips size) -> IconButtonSizingValues
{
    switch (size)
    {
        using enum Spec::SizeTips;
        case XSMALL:
            return {
                .containerHeight       = IconButtonSizing<XSMALL>::ContainerHeight,
                .iconSize              = IconButtonSizing<XSMALL>::IconSize,
                .defaultLeadingSpace   = IconButtonSizing<XSMALL>::DefaultLeadingSpace,
                .containerShapeSquare  = IconButtonSizing<XSMALL>::ContainerShapeSquare,
                .pressedContainerShape = IconButtonSizing<XSMALL>::PressedContainerShape,
                .outlinedOutlineWidth  = IconButtonSizing<XSMALL>::OutlinedOutlineWidth
            };
        case SMALL:
            return {
                .containerHeight       = IconButtonSizing<SMALL>::ContainerHeight,
                .iconSize              = IconButtonSizing<SMALL>::IconSize,
                .defaultLeadingSpace   = IconButtonSizing<SMALL>::DefaultLeadingSpace,
                .containerShapeSquare  = IconButtonSizing<SMALL>::ContainerShapeSquare,
                .pressedContainerShape = IconButtonSizing<SMALL>::PressedContainerShape,
                .outlinedOutlineWidth  = IconButtonSizing<SMALL>::OutlinedOutlineWidth
            };
        case MEDIUM:
            return {
                .containerHeight       = IconButtonSizing<MEDIUM>::ContainerHeight,
                .iconSize              = IconButtonSizing<MEDIUM>::IconSize,
                .defaultLeadingSpace   = IconButtonSizing<MEDIUM>::DefaultLeadingSpace,
                .containerShapeSquare  = IconButtonSizing<MEDIUM>::ContainerShapeSquare,
                .pressedContainerShape = IconButtonSizing<MEDIUM>::PressedContainerShape,
                .outlinedOutlineWidth  = IconButtonSizing<MEDIUM>::OutlinedOutlineWidth
            };
        case LARGE:
            return {
                .containerHeight       = IconButtonSizing<LARGE>::ContainerHeight,
                .iconSize              = IconButtonSizing<LARGE>::IconSize,
                .defaultLeadingSpace   = IconButtonSizing<LARGE>::DefaultLeadingSpace,
                .containerShapeSquare  = IconButtonSizing<LARGE>::ContainerShapeSquare,
                .pressedContainerShape = IconButtonSizing<LARGE>::PressedContainerShape,
                .outlinedOutlineWidth  = IconButtonSizing<LARGE>::OutlinedOutlineWidth
            };
        case XLARGE:
            return {
                .containerHeight       = IconButtonSizing<XLARGE>::ContainerHeight,
                .iconSize              = IconButtonSizing<XLARGE>::IconSize,
                .defaultLeadingSpace   = IconButtonSizing<XLARGE>::DefaultLeadingSpace,
                .containerShapeSquare  = IconButtonSizing<XLARGE>::ContainerShapeSquare,
                .pressedContainerShape = IconButtonSizing<XLARGE>::PressedContainerShape,
                .outlinedOutlineWidth  = IconButtonSizing<XLARGE>::OutlinedOutlineWidth
            };
    }
    return {};
}

struct IconButtonColorsValues
{
    ColorRole containerColor;
    ColorRole iconColor;
    ColorRole outlineColor;
    float     containerOpacity;
};

constexpr auto GetIconButtonColorsValues(IconButtonColors colors, bool disabled) -> IconButtonColorsValues
{
    IconButtonColorsValues values{};
    switch (colors)
    {
        case IconButtonColors::Filled: {
            values.iconColor      = disabled ? IconButtonFilled<States::Disabled>::IconColor : IconButtonFilled<States::Enabled>::IconColor;
            values.containerColor = disabled ? IconButtonFilled<States::Disabled>::ContainerColor : IconButtonFilled<States::Enabled>::ContainerColor;
            values.containerOpacity = disabled ? IconButtonFilled<States::Disabled>::ContainerOpacity : 1.0F;
            break;
        }
        case IconButtonColors::Tonal: {
            values.iconColor        = disabled ? IconButtonTonal<States::Disabled>::IconColor : IconButtonTonal<States::Enabled>::IconColor;
            values.containerColor   = disabled ? IconButtonTonal<States::Disabled>::ContainerColor : IconButtonTonal<States::Enabled>::ContainerColor;
            values.containerOpacity = disabled ? IconButtonTonal<States::Disabled>::ContainerOpacity : 1.0F;
            break;
        }
        case IconButtonColors::Outlined: {
            values.iconColor    = disabled ? IconButtonOutlined<States::Disabled>::IconColor : IconButtonOutlined<States::Enabled>::IconColor;
            values.outlineColor = disabled ? IconButtonOutlined<States::Disabled>::OutlineColor : IconButtonOutlined<States::Enabled>::OutlineColor;
            break;
        }
        case IconButtonColors::Standard: {
            values.iconColor = disabled ? IconButtonStandard<States::Disabled>::IconColor : IconButtonStandard<States::Enabled>::IconColor;
            break;
        }
    }
    return values;
}

} // namespace ImGuiEx::M3::Spec
