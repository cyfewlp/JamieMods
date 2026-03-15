//
// Created by jamie on 2026/2/28.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

#include <cassert>

namespace ImGuiEx::M3::Spec
{

enum class ChipColors
{
    Assist,
    Filter,
    Input,
    Suggestion
};

enum class ChipVariant
{
    Default,
    Selected,
    Unselected,
};

struct ChipCommon
{
    //! Assist chip disabled outline opacity
    static constexpr auto DisabledOutlineOpacity   = 0.12F;
    static constexpr auto DisabledContainerOpacity = 0.12F;
    //! chip focus indicator color
    static constexpr auto IndicatorColor           = ColorRole::secondary;
    static constexpr auto IconColor                = ColorRole::primary;
    static constexpr auto UnSelectedIconColor      = ColorRole::onSurfaceVariant;

    static constexpr auto LabelText          = TypeScaleValue::of<TextRole::LabelLarge>();
    static constexpr auto IconSize           = dp<18>();
    static constexpr auto AvatarSize         = dp<24>();
    static constexpr auto OutlineWidth       = 1.0F;
    static constexpr auto IndicatorThickness = 3.0F;
    static constexpr auto ContainerHeight    = dp<32>();
    static constexpr auto ContainerShape     = ShapeCorner::Small;
    static constexpr auto LeadingSpace       = dp<16>();
    static constexpr auto IconLeadingSpace   = dp<8>();
    static constexpr auto TrailingSpace      = dp<16>();
    static constexpr auto IconTrialingSpace  = dp<8>();
    static constexpr auto Gap                = dp<8>();
    static constexpr auto Margin             = dp<8>();  ///< the margin between two chips.
    static constexpr auto LineHeigh          = dp<48>(); ///< the min line height for chips line.
};

template <ChipColors Colors, States State = States::Enabled, ChipVariant ChipVariant = ChipVariant::Default>
struct Chip;

template <>
struct Chip<ChipColors::Assist>
{
    //! Assist chip container outline color
    static constexpr auto OutlineColor   = ColorRole::outlineVariant;
    //! Assist chip icon color
    static constexpr auto IconColor      = ColorRole::primary;
    //! Assist chip label text color
    static constexpr auto LabelTextColor = ColorRole::onSurface;
    //! Assist chip focus indicator color
    static constexpr auto IndicatorColor = ColorRole::none;
};

using AssistChip = Chip<ChipColors::Assist>;

template <>
struct Chip<ChipColors::Assist, States::Focused>
{
    //! Assist chip focus indicator color
    static constexpr auto IndicatorColor = ColorRole::secondary;
    //! Assist chip focus outline color
    static constexpr auto OutlineColor   = ColorRole::onSurface;
};

using AssistFocusedChip = Chip<ChipColors::Assist, States::Focused>;

template <>
struct Chip<ChipColors::Filter, States::Enabled, ChipVariant::Selected>
{
    //! Filter chip with leading icon selected leading icon color
    static constexpr auto IconColor = ColorRole::onSecondaryContainer;

    //! Filter chip with trailing icon selected trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSecondaryContainer;

    //! Filter chip (selected) container color
    static constexpr auto ContainerColor = ColorRole::secondaryContainer;

    //! Filter chip (selected) label text color
    static constexpr auto LabelTextColor = ColorRole::onSecondaryContainer;
    static constexpr auto OutlineWidth   = 0.0F;
};

using FilterChip        = Chip<ChipColors::Filter, States::Enabled, ChipVariant::Selected>;
using FilterFocusedChip = Chip<ChipColors::Filter, States::Focused, ChipVariant::Selected>;

template <>
struct Chip<ChipColors::Filter, States::Enabled, ChipVariant::Unselected>
{
    //! Filter chip (unselected) focus outline color
    static constexpr auto FocusOutlineColor = ColorRole::onSurfaceVariant;

    //! Filter chip (unselected) outline color
    static constexpr auto OutlineColor = ColorRole::outlineVariant;

    //! Filter chip with leading icon unselected leading icon color
    static constexpr auto IconColor = ColorRole::primary;

    //! Filter chip with trailing icon unselected trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Filter chip (unselected) label text color
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;
};

using FilterUnSelectedChip = Chip<ChipColors::Filter, States::Enabled, ChipVariant::Unselected>;

template <>
struct Chip<ChipColors::Filter, States::Focused, ChipVariant::Unselected>
{
    //! Filter chip (unselected) focus outline color
    static constexpr auto OutlineColor = ColorRole::onSurfaceVariant;
};

using FilterUnSelectedFocusedChip = Chip<ChipColors::Filter, States::Focused, ChipVariant::Unselected>;

struct ChipsColorsValues
{
    ColorRole containerColor;
    ColorRole outlineColor;
    ColorRole labelTextColor;
    ColorRole iconColor;
    ColorRole indicatorColor;
    float     containerOpacity;
};

constexpr auto GetChipsColorsValues(ChipColors colors, States states, ChipVariant variant) -> ChipsColorsValues
{
    ChipsColorsValues values{};
    values.indicatorColor = states == States::Focused ? ChipCommon::IndicatorColor : ColorRole::none;
    switch (colors)
    {
        case ChipColors::Assist:
            values.outlineColor   = AssistChip::OutlineColor;
            values.labelTextColor = AssistChip::LabelTextColor;
            values.iconColor      = AssistChip::IconColor;
            break;
        case ChipColors::Filter:
            if (variant == ChipVariant::Selected)
            {
                values.containerColor   = FilterChip::ContainerColor;
                values.containerOpacity = states == States::Disabled ? ChipCommon::DisabledContainerOpacity : 1.F;
                values.labelTextColor   = FilterChip::LabelTextColor;
                values.iconColor        = FilterChip::IconColor;
            }
            else
            {
                values.outlineColor   = states == States::Focused ? FilterUnSelectedFocusedChip::OutlineColor : FilterUnSelectedChip::OutlineColor;
                values.labelTextColor = FilterUnSelectedChip::LabelTextColor;
                values.iconColor      = FilterUnSelectedChip::IconColor;
            }
            break;
        case ChipColors::Input:
        case ChipColors::Suggestion:
        default:
            assert(0 && "not implemented yet");
            break;
    }
    return values;
}

} // namespace ImGuiEx::M3::Spec
