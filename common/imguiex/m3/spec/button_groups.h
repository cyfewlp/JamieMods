//
// Created by jamie on 2026/2/24.
//

#pragma once

#include "base.h"
#include "shapes.h"

namespace ImGuiEx::M3::Spec
{

enum class ButtonGroupVariant : std::uint8_t
{
    Standard,
    Connected
};

template <ButtonGroupVariant Variant, SizeTips Size>
struct ButtonGroupSizing;

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::XSMALL>
{
    //! Button group xsmall between space
    static constexpr auto BetweenSpace = dp<18>();

    //! Button group xsmall pressed width multiplier
    static constexpr auto PressedItemWidthMultiplier = 1.15F;

    //! Button group xsmall container height
    static constexpr auto ContainerHeight = dp<32>();
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::SMALL>
{
    //! Button group small between space
    static constexpr auto BetweenSpace = dp<12>();

    //! Button group small pressed width multiplier
    static constexpr auto PressedItemWidthMultiplier = 1.15F;

    //! Button group small container height
    static constexpr auto ContainerHeight = dp<40>();
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::MEDIUM>
{
    //! Button group medium between space
    static constexpr auto BetweenSpace = dp<8>();

    //! Button group medium pressed width multiplier
    static constexpr auto PressedItemWidthMultiplier = 1.15F;

    //! Button group medium container height
    static constexpr auto ContainerHeight = dp<56>();
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::LARGE>
{
    //! Button group large between space
    static constexpr auto BetweenSpace = dp<8>();

    //! Button group large pressed width multiplier
    static constexpr auto PressedItemWidthMultiplier = 1.15F;

    //! Button group large container height
    static constexpr auto ContainerHeight = dp<96>();
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::XLARGE>
{
    //! Button group xlarge between space
    static constexpr auto BetweenSpace = dp<8>();

    //! Button group xlarge pressed width multiplier
    static constexpr auto PressedItemWidthMultiplier = 1.15F;

    //! Button group xlarge container height
    static constexpr auto ContainerHeight = dp<136>();
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::XLARGE>
{
    //! Button group connected xlarge space between buttons
    static constexpr auto BetweenSpace = dp<2>();

    //! Button group connected xlarge pressed inner corner size
    static constexpr auto PressedInnerCornerCornerSize = ShapeCorner::Large;

    //! Button group connected xlarge selected inner corner size
    static constexpr auto SelectedInnerCornerCornerSize = dp<50>();

    //! Button group connected xlarge inner corner size
    static constexpr auto InnerCornerCornerSize = ShapeCorner::LargeIncreased;

    //! Button group connected xlarge container height
    static constexpr auto ContainerHeight = dp<136>();

    //! Button group connected xlarge container shape
    static constexpr auto ContainerShape = ContainerHeight / 2U; ///< ShapeCorner::Full;
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::LARGE>
{
    //! Button group connected large space between buttons
    static constexpr auto BetweenSpace = dp<2>();

    //! Button group connected large pressed inner corner size
    static constexpr auto PressedInnerCornerCornerSize = ShapeCorner::Medium;

    //! Button group connected large selected inner corner size
    static constexpr auto SelectedInnerCornerCornerSize = dp<50>();

    //! Button group connected large inner corner size
    static constexpr auto InnerCornerCornerSize = ShapeCorner::Large;

    //! Button group connected large container height
    static constexpr auto ContainerHeight = dp<96>();

    //! Button group connected xlarge container shape
    static constexpr auto ContainerShape = ContainerHeight / 2U; ///< ShapeCorner::Full;
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::MEDIUM>
{
    //! Button group connected medium space between buttons
    static constexpr auto BetweenSpace = dp<2>();

    //! Button group connected medium pressed inner corner size
    static constexpr auto PressedInnerCornerCornerSize = ShapeCorner::ExtraSmall;

    //! Button group connected medium selected inner corner size
    static constexpr auto SelectedInnerCornerCornerSize = dp<50>();

    //! Button group connected medium inner corner size
    static constexpr auto InnerCornerCornerSize = ShapeCorner::Small;

    //! Button group connected medium container height
    static constexpr auto ContainerHeight = dp<56>();

    //! Button group connected xlarge container shape
    static constexpr auto ContainerShape = ContainerHeight / 2U; ///< ShapeCorner::Full;
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::SMALL>
{
    //! Button group connected small space between buttons
    static constexpr auto BetweenSpace = dp<2>();

    //! Button group connected small pressed inner corner size
    static constexpr auto PressedInnerCornerCornerSize = ShapeCorner::ExtraSmall;

    //! Button group connected small selected inner corner size
    static constexpr auto SelectedInnerCornerCornerSize = dp<50>();

    //! Button group connected small inner corner size
    static constexpr auto InnerCornerCornerSize = ShapeCorner::Small;

    //! Button group connected small container height
    static constexpr auto ContainerHeight = dp<40>();

    //! Button group connected xlarge container shape
    static constexpr auto ContainerShape = ContainerHeight / 2U; ///< ShapeCorner::Full;
};

template <>
struct ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::XSMALL>
{
    //! Button group connected xsmall space between buttons
    static constexpr auto BetweenSpace = dp<2>();

    //! Button group connected xsmall pressed inner corner size
    static constexpr auto PressedInnerCornerCornerSize = ShapeCorner::ExtraSmall;

    //! Button group connected xsmall selected inner corner size
    static constexpr auto SelectedInnerCornerCornerSize = dp<50>();

    //! Button group connected xsmall inner corner size
    static constexpr auto InnerCornerCornerSize = ShapeCorner::Small;

    //! Button group connected xsmall container height
    static constexpr auto ContainerHeight = dp<32>();

    //! Button group connected xlarge container shape
    static constexpr auto ContainerShape = ContainerHeight / 2U; ///< ShapeCorner::Full;
};

} // namespace ImGuiEx::M3::Spec
