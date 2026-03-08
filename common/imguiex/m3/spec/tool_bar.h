//
// Created by jamie on 2026/2/25.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"

namespace ImGuiEx::M3::Spec
{

enum class ToolBarVariant : uint8_t
{
    Floating,
    Docked,
    FloatingFAB,
};

enum class ToolBarColors : uint8_t
{
    Standard,
    Vibrant
};

template <ToolBarVariant variant>
struct ToolBarSizing;

template <>
struct ToolBarSizing<ToolBarVariant::Floating>
{
    //! Floating toolbar leading space
    static constexpr auto ContainerLeadingSpace = dp<8>();

    //! Floating toolbar trailing space
    static constexpr auto ContainerTrailingSpace = dp<8>();

    //! Floating toolbar margin from screen edge - vertical
    static constexpr auto VerticalContainerExternalSpace = dp<24>();

    //! Floating toolbar margin from screen edge - horizontal
    static constexpr auto HorizontalContainerExternalSpace = dp<16>();

    //! Floating toolbar space between actions
    static constexpr auto ContainerBetweenSpace = dp<4>();

    //! Floating toolbar container height - vertical
    static constexpr auto VerticalContainerWidth = dp<64>();

    //! Floating toolbar container height - horizontal
    static constexpr auto HorizontalContainerHeight = dp<64>();

    //! Floating toolbar container shape
    static constexpr auto ContainerShape = HorizontalContainerHeight / 2;
};

template <>
struct ToolBarSizing<ToolBarVariant::Docked>
{
    //! Docked toolbar leading padding
    static constexpr auto ContainerLeadingSpace = dp<16>();

    //! Docked toolbar trailing padding
    static constexpr auto ContainerTrailingSpace = dp<16>();

    //! Docked toolbar container shape
    static constexpr auto ContainerShape = 0.F;

    //! Docked toolbar min space between actions
    static constexpr auto ContainerMinSpacing = dp<4>();

    //! Docked toolbar max space between actions
    static constexpr auto ContainerMaxSpacing = dp<32>();

    //! Docked toolbar container height
    static constexpr auto ContainerHeight = dp<64>();
};

template <>
struct ToolBarSizing<ToolBarVariant::FloatingFAB>
{
    //! Toolbar floating FAB space between toolbar and FAB
    static constexpr auto BetweenSpace = dp<8>();

    //! Toolbar floating FAB container shape
    static constexpr auto ContainerShape = ShapeCorner::Large;

    //! Toolbar floating FAB medium container shape
    static constexpr auto MediumContainerShape = ShapeCorner::LargeIncreased;

    //! Toolbar floating FAB icon size
    static constexpr auto IconSize = dp<24>();

    //! Toolbar floating FAB medium icon size
    static constexpr auto MediumIconSize = dp<28>();

    //! Toolbar floating FAB container width
    static constexpr auto ContainerWidth = dp<56>();

    //! Toolbar floating FAB medium container width
    static constexpr auto MediumContainerWidth = dp<80>();

    //! Toolbar floating FAB container height
    static constexpr auto ContainerHeight = dp<56>();

    //! Toolbar floating FAB medium container height
    static constexpr auto MediumContainerHeight = dp<80>();

    //! Toolbar floating FAB standard icon color
    static constexpr auto StandardIconColor = ColorRole::onSecondaryContainer;

    //! Toolbar floating FAB vibrant icon color
    static constexpr auto VibrantIconColor = ColorRole::onTertiaryContainer;

    //! Toolbar floating FAB standard container color
    static constexpr auto StandardContainerColor = ColorRole::secondaryContainer;

    //! Toolbar floating FAB vibrant container color
    static constexpr auto VibrantContainerColor = ColorRole::tertiaryContainer;
};

template <ToolBarColors Colors>
struct ToolBar;

template <>
struct ToolBar<ToolBarColors::Standard>
{
    //! Toolbar standard icon color
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;

    //! Toolbar standard container color
    static constexpr auto ContainerColor = ColorRole::surfaceContainer;

    //! Toolbar standard button container color
    static constexpr auto ButtonContainerColor = ColorRole::surfaceContainer;

    //! Toolbar standard label color
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;

    //! Toolbar standard selected icon color
    static constexpr auto SelectedIconColor = ColorRole::onSecondaryContainer;

    //!  Toolbar standard selected button container color
    static constexpr auto SelectedButtonContainerColor = ColorRole::secondaryContainer;

    //! Toolbar standard selected label color
    static constexpr auto SelectedLabelTextColor = ColorRole::onSecondaryContainer;

    //! Toolbar standard disabled icon color
    static constexpr auto DisabledIconColor = ColorRole::onSurface;

    //! Toolbar standard disabled label color
    static constexpr auto DisabledLabelTextColor = ColorRole::onSurface;
};

template <>
struct ToolBar<ToolBarColors::Vibrant>
{
    //! Toolbar vibrant icon color
    static constexpr auto IconColor = ColorRole::onPrimaryContainer;

    //! Toolbar vibrant button container color
    static constexpr auto ButtonContainerColor = ColorRole::primaryContainer;

    //! Toolbar standard container color
    static constexpr auto ContainerColor = ColorRole::primaryContainer;

    //! Toolbar vibrant label color
    static constexpr auto LabelTextColor = ColorRole::onPrimaryContainer;

    //! Toolbar vibrant selected icon color
    static constexpr auto SelectedIconColor = ColorRole::onSurface;

    //!  Toolbar vibrant selected button container color
    static constexpr auto SelectedButtonContainerColor = ColorRole::surfaceContainer;

    //! Toolbar vibrant selected label color
    static constexpr auto SelectedLabelTextColor = ColorRole::onSurface;

    //! Toolbar vibrant disabled label color
    static constexpr auto DisabledLabelTextColor = ColorRole::onSurface;

    //! Toolbar vibrant disabled icon color
    static constexpr auto DisabledIconColor = ColorRole::onSurface;
};

} // namespace ImGuiEx::M3::Spec
