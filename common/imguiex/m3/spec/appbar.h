//
// Created by jamie on 2026/3/4.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{

enum class AppBarVariant : std::uint8_t
{
    Small,
    MediumFlexible,
    LargeFlexible
};

struct AppBarCommon
{
    //! App bar title text
    static constexpr auto TitleColor = ColorRole::onSurface;

    //! App bar subtitle text
    static constexpr auto SubtitleColor = ColorRole::onSurfaceVariant;

    //! Search label color
    static constexpr auto SearchLabelColor = ColorRole::onSurfaceVariant;

    //! App bar leading icon
    static constexpr auto LeadingIconColor = ColorRole::onSurface;

    //! App bar trailing icon
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! App bar container color
    static constexpr auto ContainerColor = ColorRole::surface;

    //! Search container color
    static constexpr auto SearchContainerColor = ColorRole::surfaceContainer;

    //! App bar container color on scroll
    static constexpr auto OnScrollContainerColor = ColorRole::surfaceContainer;

    //! Search container color on scroll
    static constexpr auto SearchOnScrollContainerColor = ColorRole::surfaceContainerHighest;

    //! App bar left padding
    static constexpr auto LeadingSpace = dp<4>();

    //! Search left padding
    static constexpr auto SearchLeadingSpace = dp<8>();

    //! App bar right padding
    static constexpr auto TrailingSpace = dp<4>();

    //! Search right padding
    static constexpr auto SearchTrailingSpace = dp<8>();

    //! App bar icon spacing
    static constexpr auto IconButtonSpace = dp<0>();

    //! App bar container shape
    static constexpr auto ContainerShape = ShapeCorner::None;

    //! App bar icon size
    static constexpr auto IconSize = dp<24>();

    //! App bar avatar size
    static constexpr auto AvatarSize = dp<32>();
};

template <AppBarVariant Variant>
struct AppBarSizing;

template <>
struct AppBarSizing<AppBarVariant::Small>
{
    //! App bar small container height
    static constexpr auto ContainerHeight = dp<64>();

    //! Search container shape
    static constexpr auto SearchContainerShape = ContainerHeight / 2U; // ShapeCorner::Full;

    //! Search container height
    static constexpr auto SearchContainerHeight = dp<56>();

    //! App bar small title font
    static constexpr auto TitleText = TypeScaleValue::of<TextRole::TitleLarge>();

    //! The title leading space
    static constexpr auto TitleLeadingSpace  = dp<4>();
    //! The title trialing space
    static constexpr auto TitleTrialingSpace = dp<4>();
    static constexpr auto TitleTopSpace      = dp<10>();

    //! App bar small subtitle font
    static constexpr auto SubtitleText = TypeScaleValue::of<TextRole::LabelMedium>();

    //! Search title font
    static constexpr auto SearchLabelText = TypeScaleValue::of<TextRole::BodyLarge>();
};

template <>
struct AppBarSizing<AppBarVariant::MediumFlexible>
{
    //! App bar medium flexible container height with subtitle
    static constexpr auto ContainerHeight    = dp<136>();
    static constexpr auto BottomSpace        = dp<12>();
    static constexpr auto TitleTopSpace      = AppBarSizing<AppBarVariant::Small>::ContainerHeight;
    static constexpr auto TitleBottomSpace   = dp<4>();
    //! The title leading space
    static constexpr auto TitleLeadingSpace  = dp<16>();
    //! The title trialing space
    static constexpr auto TitleTrialingSpace = dp<16>();

    //! App bar medium Title font
    static constexpr auto TitleText = TypeScaleValue::of<TextRole::HeadlineMedium>();

    //! App bar medium Subtitle font
    static constexpr auto SubtitleText = TypeScaleValue::of<TextRole::LabelLarge>();
};

template <>
struct AppBarSizing<AppBarVariant::LargeFlexible>
{
    //! App bar large flexible container height with subtitle
    static constexpr auto ContainerHeight    = dp<152>();
    static constexpr auto BottomSpace        = dp<12>();
    static constexpr auto TitleTopSpace      = AppBarSizing<AppBarVariant::Small>::ContainerHeight;
    static constexpr auto TitleBottomSpace   = dp<8>();
    //! The title leading space
    static constexpr auto TitleLeadingSpace  = dp<16>();
    //! The title trialing space
    static constexpr auto TitleTrialingSpace = dp<16>();

    //! App bar small title font
    static constexpr auto TitleText = TypeScaleValue::of<TextRole::DisplaySmall>();

    //! App bar small subtitle font
    static constexpr auto SubtitleText = TypeScaleValue::of<TextRole::TitleMedium>();
};

} // namespace ImGuiEx::M3::Spec
