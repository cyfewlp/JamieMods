//
// Created by jamie on 2026/2/15.
//

#pragma once

#include "../colors.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{

struct NavRailItem
{
    //! Nav rail item icon size
    static constexpr auto IconSize = dp<24>();

    //! Nav rail item active indicator shape
    // static constexpr auto  ActiveIndicatorShape = md.sys.shape.corner.full;

    //! Nav rail item active indicator leading space
    static constexpr auto ActiveIndicatorLeadingSpace = dp<16>();

    //! Nav rail item active indicator icon label space
    static constexpr auto ActiveIndicatorIconLabelSpace = dp<8>();

    //! Nav rail item active indicator trailing space
    static constexpr auto ActiveIndicatorTrailingSpace = dp<16>();

    //! Nav rail item container height
    static constexpr auto ContainerHeight = dp<64>();

    //! Nav rail item container vertical space
    static constexpr auto ContainerVerticalSpace = dp<6>();

    //! Nav rail item header space minimum
    static constexpr auto HeaderSpaceMinimum = dp<40>();

    //! Nav rail item container shape
    // static constexpr auto  ContainerShape = md.sys.shape.corner.none;

    //! Nav rail item short container height
    static constexpr auto ShortContainerHeight = dp<56>();

    //! Extent field
    static constexpr auto ActiveIndicatorOffsetXEx = dp<20>();
};

struct NavRailItemVertical : public NavRailItem
{
    using NavRailItem::NavRailItem;

    //! Nav rail item vertical active indicator height
    static constexpr auto ActiveIndicatorHeight = dp<32>();

    //! Nav rail item vertical active indicator width
    static constexpr auto ActiveIndicatorWidth = dp<56>();

    //! Nav rail item vertical icon label space
    static constexpr auto IconLabelSpace = dp<4>();

    //! Nav rail item vertical trailing space
    static constexpr auto TrailingSpace = dp<16>();

    //! Nav rail item vertical leading-space
    static constexpr auto LeadingSpace = dp<16>();

    //! Nav rail item vertical label text
    // static constexpr auto LabelTextFont = md.sys.typescale.label - medium;
    static constexpr auto LabelTextRole = TextRole::LabelMedium;
};

struct NavRailItemHorizontal : public NavRailItem
{
    using NavRailItem::NavRailItem;

    //! Nav rail item horizontal label text
    static constexpr auto LabelTextRole = TextRole::LabelLarge;

    //! Nav rail item horizontal active indicator height
    static constexpr auto ActiveIndicatorHeight = dp<56>();

    //! Nav rail item horizontal full width leading space
    static constexpr auto FullWidthLeadingSpace = dp<16>();

    //! Nav rail item horizontal full width trailing space
    static constexpr auto FullWidthTrailingSpace = dp<16>();

    //! Nav rail item horizontal icon-label-space
    static constexpr auto IconLabelSpace = dp<8>();

    //! Extend field. Not contain label size, only contain icon size and related space.
    static constexpr auto ActiveIndicatorMinWidthEx = NavRailItem::ActiveIndicatorLeadingSpace +
                                                      NavRailItem::ActiveIndicatorTrailingSpace +
                                                      NavRailItem::IconSize + IconLabelSpace;
};

struct NavRailCommon
{
    //! Nav rail item active indicator color
    static constexpr auto ActiveIndicatorColor = ColorRole::secondaryContainer;

    //! Nav rail item active label text color
    static constexpr auto ActiveLabelTextColor = ColorRole::secondary;

    //! Nav rail item inactive label text color
    static constexpr auto InactiveLabelTextColor = ColorRole::onSurfaceVariant;

    //! Nav rail item active icon color
    static constexpr auto ActiveIconColor = ColorRole::onSecondaryContainer;

    //! Nav rail item inactive icon color
    static constexpr auto InactiveIconColor = ColorRole::onSurfaceVariant;

    //! Nav rail item active hovered state layer color
    static constexpr auto ActiveHoveredStateLayerColor = ColorRole::onSecondaryContainer;

    //! Nav rail item inactive hovered state layer color
    static constexpr auto InactiveHoveredStateLayerColor = ColorRole::onSecondaryContainer;

    //! Nav rail item active focused state layer color
    static constexpr auto ActiveFocusedStateLayerColor = ColorRole::onSecondaryContainer;

    //! Nav rail item inactive focused state layer color
    static constexpr auto InactiveFocusedStateLayerColor = ColorRole::onSecondaryContainer;

    //! Nav rail item active pressed state layer color
    static constexpr auto ActivePressedStateLayerColor = ColorRole::onSecondaryContainer;

    //! Nav rail item inactive pressed state layer color
    static constexpr auto InactivePressedStateLayerColor = ColorRole::onSecondaryContainer;

    //! Nav rail item active hovered state layer opacity
    // static constexpr auto  ActiveHoveredStateLayerOpacity = md.sys.state.hover.state-layer-opacity;

    //! Nav rail item active focused state layer opacity
    // static constexpr auto  ActiveFocusedStateLayerOpacity = md.sys.state.focus.state-layer-opacity;

    //! Nav rail item active pressed state layer opacity
    // static constexpr auto  ActivePressedStateLayerOpacity = md.sys.state.pressed.state-layer-opacity;
};

enum class NavRailState : std::uint8_t
{
    Collapsed,
    Expanded,
};

template <NavRailState State>
struct NavRail;

template <>
struct NavRail<NavRailState::Collapsed> : public NavRailCommon
{
    using NavRailCommon::NavRailCommon;

    //! Nav rail collapsed container color
    static constexpr auto ContainerColor = ColorRole::surface;

    //! Nav rail collapsed item vertical space
    static constexpr auto ItemVerticalSpace = dp<4>();

    //! Nav rail collapsed container width
    static constexpr auto ContainerWidth = dp<96>();

    //! Nav rail collapsed container elevation
    // static constexpr auto ContainerElevation = md.sys.elevation.level0;

    //! Nav rail collapsed container shape
    // static constexpr auto ContainerShape = md.sys.shape.corner.none;

    //! Nav rail collapsed item top space
    static constexpr auto TopSpace = dp<44>();

    //! Nav rail collapsed narrow container width
    static constexpr auto NarrowContainerWidth = dp<80>();
};

template <>
struct NavRail<NavRailState::Expanded> : public NavRailCommon
{
    using NavRailCommon::NavRailCommon;

    //! Nav rail expanded modal container color
    static constexpr auto ModalContainerColor = ColorRole::surfaceContainer;

    //! Nav rail expanded container color
    static constexpr auto ContainerColor = ColorRole::surface;

    //! Nav rail expanded modal container shape
    // static constexpr auto ModalContainerShape = md.sys.shape.corner.large;

    //! Nav rail expanded container shape
    // static constexpr auto ContainerShape = md.sys.shape.corner.none;

    //! Nav rail expanded between item space
    static constexpr auto BetweenItemSpace = dp<0>();

    //! Nav rail expanded container width minimum
    static constexpr auto ContainerWidthMinimum = dp<220>();

    //! Nav rail expanded container width maximum
    static constexpr auto ContainerWidthMaximum = dp<360>();

    //! Nav rail expanded top space
    static constexpr auto TopSpace = dp<44>();

    //! Nav rail expanded container elevation
    // static constexpr auto ContainerElevation = md.sys.elevation.level0;

    //! Nav rail expanded modal container elevation
    // static constexpr auto ModalContainerElevation = md.sys.elevation.level2;
};

} // namespace ImGuiEx::M3::Spec
