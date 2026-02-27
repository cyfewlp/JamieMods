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

enum class ButtonColors : std::uint8_t
{
    filled,
    tonal,
    outlined,
    text,
};

enum class ButtonShape : std::uint8_t
{
    Round,
    Square
};

template <States State>
struct ButtonFilled;

template <>
struct ButtonFilled<States::Enabled>
{
    //! Button filled label color - toggle (selected)
    static constexpr auto SelectedLabelTextColor = ColorRole::onPrimary;

    //! Button filled label color - toggle (unselected)
    static constexpr auto UnselectedLabelTextColor = ColorRole::onSurfaceVariant;

    //! Button filled container color
    static constexpr auto ContainerColor = ColorRole::primary;

    //! Button filled container color - toggle (unselected)
    static constexpr auto UnselectedContainerColor = ColorRole::surfaceContainer;

    //! Button filled container color - toggle (selected)
    static constexpr auto SelectedContainerColor = ColorRole::primary;

    //! Button filled shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button filled label color
    static constexpr auto LabelTextColor = ColorRole::onPrimary;

    //! Button filled icon color
    static constexpr auto IconColor = ColorRole::onPrimary;

    //! Button filled icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::onPrimary;

    //! Button filled icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSurfaceVariant;
};

using FilledButtonEnabled = ButtonFilled<States::Enabled>;

template <>
struct ButtonFilled<States::Hovered> : public FilledButtonEnabled
{
    using FilledButtonEnabled::FilledButtonEnabled;
};

template <>
struct ButtonFilled<States::Focused> : public FilledButtonEnabled
{
    using FilledButtonEnabled::FilledButtonEnabled;
};

template <>
struct ButtonFilled<States::Pressed> : public FilledButtonEnabled
{
    using FilledButtonEnabled::FilledButtonEnabled;
};

template <>
struct ButtonFilled<States::Disabled> : public FilledButtonEnabled
{
    using FilledButtonEnabled::FilledButtonEnabled;

    //! Button filled disabled container color
    static constexpr auto DisabledContainerColor = ColorRole::onSurface;

    //! Button filled disabled container opacity
    static constexpr auto DisabledContainerOpacity = 0.1F;

    //! Button filled disabled label color
    static constexpr auto DisabledLabelTextColor = ColorRole::onSurface;

    //! Button filled disabled icon color
    static constexpr auto DisabledIconColor = ColorRole::onSurface;
};

struct ButtonText
{
    //! Button text icon color
    static constexpr auto IconColor = ColorRole::primary;

    //! Button text label color
    static constexpr auto LabelTextColor = ColorRole::primary;

    //! Button text disabled container color
    static constexpr auto DisabledContainerColor = ColorRole::onSurface;

    //! Button text disabled icon color
    static constexpr auto DisabledIconColor = ColorRole::onSurface;

    //! Button text disabled label color
    static constexpr auto DisabledLabelTextColor = ColorRole::onSurface;

    //! Button text disabled container opacity
    static constexpr auto DisabledContainerOpacity = 0.1F;
};

struct ButtonOutlined
{
    //! Button outlined outline color
    static constexpr auto OutlineColor = ColorRole::outlineVariant;

    //! Button outlined icon color
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;

    //! Button outlined icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::inverseOnSurface;

    //! Button outlined icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSurfaceVariant;

    //! Button outlined container color - toggle (selected)
    static constexpr auto SelectedContainerColor = ColorRole::inverseSurface;

    //! Button outlined label color
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;

    //! Button outlined label color - toggle (selected)
    static constexpr auto SelectedLabelTextColor = ColorRole::inverseOnSurface;

    //! Button outlined label color - toggle (unselected)
    static constexpr auto UnselectedLabelTextColor = ColorRole::onSurfaceVariant;

    //! Button outlined disabled outline color
    static constexpr auto DisabledOutlineColor = ColorRole::outlineVariant;

    //! Button outlined disabled outline color (unselected)
    static constexpr auto UnselectedDisabledOutlineColor = ColorRole::outlineVariant;

    //! Button outlined disabled icon color
    static constexpr auto DisabledIconColor = ColorRole::onSurface;

    //! Button outlined disabled container color (selected)
    static constexpr auto SelectedDisabledContainerColor = ColorRole::onSurface;

    //! Button outlined disabled label color
    static constexpr auto DisabledLabelTextColor = ColorRole::onSurface;

    //! Button outlined disabled container opacity
    static constexpr auto DisabledContainerOpacity = 0.1F;
};

struct ButtonTonal
{
    //! Button tonal shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button tonal icon color
    static constexpr auto IconColor = ColorRole::onSecondaryContainer;

    //! Button tonal icon color - toggle (selected)
    static constexpr auto SelectedIconColor = ColorRole::onSecondary;

    //! Button tonal icon color - toggle (unselected)
    static constexpr auto UnselectedIconColor = ColorRole::onSecondaryContainer;

    //! Button tonal container color
    static constexpr auto ContainerColor = ColorRole::secondaryContainer;

    //! Button tonal container color - toggle (selected)
    static constexpr auto SelectedContainerColor = ColorRole::secondary;

    //! Button tonal container color - toggle (unselected)
    static constexpr auto UnselectedContainerColor = ColorRole::secondaryContainer;

    //! Button tonal label color
    static constexpr auto LabelTextColor = ColorRole::onSecondaryContainer;

    //! Button tonal label color - toggle (selected)
    static constexpr auto SelectedLabelTextColor = ColorRole::onSecondary;

    //! Button tonal label color - toggle (unselected)
    static constexpr auto UnselectedLabelTextColor = ColorRole::onSecondaryContainer;

    //! Button tonal disabled icon color
    static constexpr auto DisabledIconColor = ColorRole::onSurface;

    //! Button tonal disabled container color
    static constexpr auto DisabledContainerColor = ColorRole::onSurface;

    //! Button tonal disabled label color
    static constexpr auto DisabledLabelTextColor = ColorRole::onSurface;

    //! Button tonal disabled container opacity
    static constexpr auto DisabledContainerOpacity = 0.1F;
};

template <SizeTips Size>
struct ButtonSizing;

template <>
struct ButtonSizing<SizeTips::XSMALL>
{
    //! Button xsmall container height
    static constexpr auto ContainerHeight = dp<32>();

    //! Button xsmall outline width
    static constexpr auto OutlinedOutlineWidth = 1.0F;

    //! Button xsmall icon size
    static constexpr auto IconSize = dp<20>();

    //! Button xsmall label size
    static constexpr auto LabelText = TypeScaleValue::of<TextRole::LabelLarge>();

    //! Button xsmall shape round
    static constexpr auto ContainerShapeRound = ContainerHeight / 2U; ///< ShapeCorner::full;

    //! Button xsmall leading space
    static constexpr auto LeadingSpace = dp<12>();

    //! Button xsmall between icon label space
    static constexpr auto IconLabelSpace = dp<8>();

    //! Button xsmall trailing space
    static constexpr auto TrailingSpace = dp<12>();

    //! Button xsmall shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::Medium;

    //! Button xsmall shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Small;

    //! Button xsmall selected container shape square
    // static constexpr auto SelectedContainerShapeSquare = ShapeCorner::full;

    //! Button xsmall selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::Medium;
};

using ButtonExtraSmall = ButtonSizing<SizeTips::XSMALL>;

template <>
struct ButtonSizing<SizeTips::SMALL> : public ButtonExtraSmall
{
    using ButtonExtraSmall::ButtonExtraSmall;

    //! Button small container height
    static constexpr auto ContainerHeight = dp<40>();

    //! Button xsmall shape round
    static constexpr auto ContainerShapeRound = ContainerHeight / 2U; ///< ShapeCorner::full;

    //! Button small leading space
    static constexpr auto LeadingSpace = dp<16>();

    //! Button small trailing space
    static constexpr auto TrailingSpace = dp<16>();
};

template <>
struct ButtonSizing<SizeTips::MEDIUM> : public ButtonExtraSmall
{
    //! Button medium container height
    static constexpr auto ContainerHeight = dp<56>();

    //! Button medium label size
    static constexpr auto LabelText = TypeScaleValue::of<TextRole::TitleMedium>();

    //! Button medium icon size
    static constexpr auto IconSize = dp<24>();

    //! Button medium leading space
    static constexpr auto LeadingSpace = dp<24>();

    //! Button medium trailing space
    static constexpr auto TrailingSpace = dp<24>();

    //! Button xsmall shape round
    static constexpr auto ContainerShapeRound = ContainerHeight / 2U; ///< ShapeCorner::full;

    //! Button medium shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::Large;

    //! Button medium shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Medium;

    //! Button medium selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::Large;
};

template <>
struct ButtonSizing<SizeTips::LARGE> : public ButtonExtraSmall
{
    using ButtonExtraSmall::ButtonExtraSmall;

    //! Button large container height
    static constexpr auto ContainerHeight = dp<96>();

    //! Button large outline width
    static constexpr auto OutlinedOutlineWidth = dp<2>();

    //! Button large label size
    static constexpr auto LabelText = TypeScaleValue::of<TextRole::HeadlineSmall>();

    //! Button large icon size
    static constexpr auto IconSize = dp<32>();

    //! Button large leading space
    static constexpr auto LeadingSpace = dp<48>();

    //! Button large between icon label space
    static constexpr auto IconLabelSpace = dp<12>();

    //! Button large trailing space
    static constexpr auto TrailingSpace = dp<48>();

    //! Button xsmall shape round
    static constexpr auto ContainerShapeRound = ContainerHeight / 2U; ///< ShapeCorner::full;

    //! Button large shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::ExtraLarge;

    //! Button large shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Large;

    //! Button large selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::ExtraLarge;
};

template <>
struct ButtonSizing<SizeTips::XLARGE> : public ButtonExtraSmall
{
    using ButtonExtraSmall::ButtonExtraSmall;
    //! Button xlarge container height
    static constexpr auto ContainerHeight = dp<136>();

    //! Button xlarge outline width
    static constexpr auto OutlinedOutlineWidth = 3.0F;

    //! Button xlarge label size
    static constexpr auto LabelText = TypeScaleValue::of<TextRole::HeadlineLarge>();

    //! Button xlarge icon size
    static constexpr auto IconSize = dp<40>();

    //! Button xlarge leading space
    static constexpr auto LeadingSpace = dp<64>();

    //! Button xlarge between icon label space
    static constexpr auto IconLabelSpace = dp<16>();

    //! Button xlarge trailing space
    static constexpr auto TrailingSpace = dp<64>();

    //! Button xsmall shape round
    static constexpr auto ContainerShapeRound = ContainerHeight / 2U; ///< ShapeCorner::full;

    //! Button xlarge shape square
    static constexpr auto ContainerShapeSquare = ShapeCorner::ExtraLarge;

    //! Button xlarge shape pressed morph
    static constexpr auto PressedContainerShape = ShapeCorner::Large;

    //! Button xlarge selected container shape round
    static constexpr auto SelectedContainerShapeRound = ShapeCorner::ExtraLarge;
};

struct ButtonSizingValues
{
    Spec::Unit           containerHeight;
    Spec::Unit           iconSize;
    Spec::Unit           leadingSpace;
    Spec::Unit           iconLabelSpace;
    Spec::Unit           containerShape;
    Spec::Unit           pressedContainerShape;
    Spec::TypeScaleValue labelText;
};

constexpr auto GetButtonSizing(Spec::SizeTips size, Spec::ButtonShape shape) -> ButtonSizingValues
{
    ButtonSizingValues values;
    switch (size)
    {
        case SizeTips::XSMALL: {
            using SizingSpec             = ButtonSizing<SizeTips::XSMALL>;
            values.containerHeight       = SizingSpec::ContainerHeight;
            values.iconSize              = SizingSpec::IconSize;
            values.leadingSpace          = SizingSpec::LeadingSpace;
            values.iconLabelSpace        = SizingSpec::IconLabelSpace;
            values.containerShape        = shape == ButtonShape::Square ? SizingSpec::ContainerShapeSquare : SizingSpec::ContainerShapeRound;
            values.pressedContainerShape = SizingSpec::PressedContainerShape;
            values.labelText             = SizingSpec::LabelText;
            break;
        }
        case SizeTips::SMALL: {
            using SizingSpec             = ButtonSizing<SizeTips::SMALL>;
            values.containerHeight       = SizingSpec::ContainerHeight;
            values.iconSize              = SizingSpec::IconSize;
            values.leadingSpace          = SizingSpec::LeadingSpace;
            values.iconLabelSpace        = SizingSpec::IconLabelSpace;
            values.containerShape        = shape == ButtonShape::Square ? SizingSpec::ContainerShapeSquare : SizingSpec::ContainerShapeRound;
            values.pressedContainerShape = SizingSpec::PressedContainerShape;
            values.labelText             = SizingSpec::LabelText;
            break;
        }
        case SizeTips::MEDIUM: {
            using SizingSpec             = ButtonSizing<SizeTips::MEDIUM>;
            values.containerHeight       = SizingSpec::ContainerHeight;
            values.iconSize              = SizingSpec::IconSize;
            values.leadingSpace          = SizingSpec::LeadingSpace;
            values.iconLabelSpace        = SizingSpec::IconLabelSpace;
            values.containerShape        = shape == ButtonShape::Square ? SizingSpec::ContainerShapeSquare : SizingSpec::ContainerShapeRound;
            values.pressedContainerShape = SizingSpec::PressedContainerShape;
            values.labelText             = SizingSpec::LabelText;
            break;
        }
        case SizeTips::LARGE: {
            using SizingSpec             = ButtonSizing<SizeTips::LARGE>;
            values.containerHeight       = SizingSpec::ContainerHeight;
            values.iconSize              = SizingSpec::IconSize;
            values.leadingSpace          = SizingSpec::LeadingSpace;
            values.iconLabelSpace        = SizingSpec::IconLabelSpace;
            values.containerShape        = shape == ButtonShape::Square ? SizingSpec::ContainerShapeSquare : SizingSpec::ContainerShapeRound;
            values.pressedContainerShape = SizingSpec::PressedContainerShape;
            values.labelText             = SizingSpec::LabelText;
            break;
        }
        case SizeTips::XLARGE: {
            using SizingSpec             = ButtonSizing<SizeTips::XLARGE>;
            values.containerHeight       = SizingSpec::ContainerHeight;
            values.iconSize              = SizingSpec::IconSize;
            values.leadingSpace          = SizingSpec::LeadingSpace;
            values.iconLabelSpace        = SizingSpec::IconLabelSpace;
            values.containerShape        = shape == ButtonShape::Square ? SizingSpec::ContainerShapeSquare : SizingSpec::ContainerShapeRound;
            values.pressedContainerShape = SizingSpec::PressedContainerShape;
            values.labelText             = SizingSpec::LabelText;
            break;
        }
    }
    return values;
}

struct ButtonColorsValues
{
    ColorRole containerColor;
    ColorRole labelTextColor;
    ColorRole iconColor;
    float     containerOpacity;
};

constexpr auto GetButtonColors(Spec::ButtonColors colors, const bool disabled) -> ButtonColorsValues
{
    ButtonColorsValues values{};
    switch (colors)
    {
        case ButtonColors::filled: {
            using ColorSpec       = ButtonFilled<States::Enabled>;
            values.containerColor = disabled ? ButtonFilled<States::Disabled>::ContainerColor : ColorSpec::ContainerColor;
            values.labelTextColor = disabled ? ButtonFilled<States::Disabled>::LabelTextColor : ColorSpec::LabelTextColor;
            values.iconColor      = disabled ? ButtonFilled<States::Disabled>::IconColor : ColorSpec::IconColor;
            break;
        }
        case ButtonColors::tonal: {
            using ColorSpec         = ButtonTonal;
            values.containerColor   = disabled ? ColorSpec::DisabledContainerColor : ColorSpec::ContainerColor;
            values.containerOpacity = disabled ? ColorSpec::DisabledContainerOpacity : 1.0F;
            values.labelTextColor   = disabled ? ColorSpec::DisabledLabelTextColor : ColorSpec::LabelTextColor;
            values.iconColor        = disabled ? ColorSpec::DisabledIconColor : ColorSpec::IconColor;
            break;
        }
        case ButtonColors::outlined: {
            using ColorSpec = ButtonOutlined;
            if (disabled)
            {
                values.containerOpacity = ColorSpec::DisabledContainerOpacity;
            }
            values.labelTextColor = disabled ? ColorSpec::DisabledLabelTextColor : ColorSpec::LabelTextColor;
            values.iconColor      = disabled ? ColorSpec::DisabledIconColor : ColorSpec::IconColor;
            break;
        }
        case ButtonColors::text: {
            using ColorSpec = ButtonText;
            if (disabled)
            {
                values.containerColor   = ColorSpec::DisabledContainerColor;
                values.containerOpacity = ColorSpec::DisabledContainerOpacity;
            }
            values.labelTextColor = disabled ? ColorSpec::DisabledLabelTextColor : ColorSpec::LabelTextColor;
            values.iconColor      = disabled ? ColorSpec::DisabledIconColor : ColorSpec::IconColor;
            break;
        }
    }
    return values;
}
} // namespace ImGuiEx::M3::Spec
