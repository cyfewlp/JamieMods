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
    elevated,
    filled,
    tonal,
    outlined,
    text,
};

enum class ButtonShape : std::uint8_t
{
    Round,
    Square,
};

enum class ButtonVariant : std::uint8_t
{
    Default,
    ToggleSelected,
    ToggleUnselected,
};

template <ButtonColors Colors, States State, ButtonVariant Variant = ButtonVariant::Default>
struct Button;

template <>
struct Button<ButtonColors::elevated, States::Enabled>
{
    //! Button elevated shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button elevated icon color
    static constexpr auto IconColor = ColorRole::primary;

    //! Button elevated container color
    static constexpr auto ContainerColor = ColorRole::surfaceContainerLow;

    //! Button elevated label color
    static constexpr auto LabelTextColor = ColorRole::primary;
};

template <>
struct Button<ButtonColors::elevated, States::Enabled, ButtonVariant::ToggleSelected>
{
    //! Button elevated icon color - toggle (selected)
    static constexpr auto IconColor            = ColorRole::onPrimary;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button elevated container color - toggle (selected)
    static constexpr auto ContainerColor = ColorRole::primary;

    //! Button elevated label color - toggle (selected)
    static constexpr auto LabelTextColor = ColorRole::onPrimary;
};

template <>
struct Button<ButtonColors::elevated, States::Enabled, ButtonVariant::ToggleUnselected>
{
    //! Button elevated icon color - toggle (unselected)
    static constexpr auto IconColor = ColorRole::primary;

    //! Button elevated container color - toggle (unselected)
    static constexpr auto ContainerColor       = ColorRole::surfaceContainerLow;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button elevated label color - toggle (unselected)
    static constexpr auto LabelTextColor = ColorRole::primary;
};

template <>
struct Button<ButtonColors::filled, States::Enabled>
{
    //! Button filled container color
    static constexpr auto ContainerColor = ColorRole::primary;

    //! Button filled shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button filled label color
    static constexpr auto LabelTextColor = ColorRole::onPrimary;

    //! Button filled icon color
    static constexpr auto IconColor = ColorRole::onPrimary;
};

template <>
struct Button<ButtonColors::filled, States::Enabled, ButtonVariant::ToggleSelected>
{
    //! Button filled label color - toggle (selected)
    static constexpr auto LabelTextColor = ColorRole::onPrimary;

    //! Button filled container color - toggle (selected)
    static constexpr auto ContainerColor       = ColorRole::primary;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button filled icon color - toggle (selected)
    static constexpr auto IconColor = ColorRole::onPrimary;
};

template <>
struct Button<ButtonColors::filled, States::Enabled, ButtonVariant::ToggleUnselected>
{
    //! Button filled label color - toggle (unselected)
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;

    //! Button filled container color - toggle (unselected)
    static constexpr auto ContainerColor       = ColorRole::surfaceContainer;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button filled icon color - toggle (unselected)
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;
};

template <>
struct Button<ButtonColors::filled, States::Disabled>
{
    //! Button filled disabled container color
    static constexpr auto ContainerColor = ColorRole::onSurface;

    //! Button filled disabled container opacity
    static constexpr auto ContainerOpacity = 0.1F;

    //! Button filled disabled label color
    static constexpr auto LabelTextColor = ColorRole::onSurface;

    //! Button filled disabled icon color
    static constexpr auto IconColor = ColorRole::onSurface;
};

template <>
struct Button<ButtonColors::text, States::Enabled>
{
    //! Button text disabled container color
    static constexpr auto ContainerColor = ColorRole::none;

    //! Button elevated shadow color
    static constexpr auto ContainerShadowColor = ColorRole::none;

    //! Button text icon color
    static constexpr auto IconColor = ColorRole::primary;

    //! Button text label color
    static constexpr auto LabelTextColor = ColorRole::primary;
};

using TextButtonEnabled = Button<ButtonColors::text, States::Enabled>;

template <>
struct Button<ButtonColors::text, States::Enabled, ButtonVariant::ToggleSelected> : public TextButtonEnabled
{
    using TextButtonEnabled::TextButtonEnabled;
};

template <>
struct Button<ButtonColors::text, States::Enabled, ButtonVariant::ToggleUnselected> : public TextButtonEnabled
{
    using TextButtonEnabled::TextButtonEnabled;
};

template <>
struct Button<ButtonColors::outlined, States::Enabled>
{
    //! Button outlined disabled container color
    static constexpr auto ContainerColor       = ColorRole::none;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button outlined outline color
    static constexpr auto OutlineColor = ColorRole::outlineVariant;

    //! Button outlined icon color
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;

    //! Button outlined label color
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;
};

template <>
struct Button<ButtonColors::outlined, States::Enabled, ButtonVariant::ToggleSelected>
{
    //! Button outlined icon color - toggle (selected)
    static constexpr auto IconColor = ColorRole::inverseOnSurface;

    //! Button outlined container color - toggle (selected)
    static constexpr auto ContainerColor       = ColorRole::inverseSurface;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button outlined label color - toggle (selected)
    static constexpr auto LabelTextColor = ColorRole::inverseOnSurface;
};

template <>
struct Button<ButtonColors::outlined, States::Enabled, ButtonVariant::ToggleUnselected>
{
    //! Button outlined icon color - toggle (unselected)
    static constexpr auto IconColor = ColorRole::onSurfaceVariant;

    //! Button outlined container color - toggle (selected)
    static constexpr auto ContainerColor       = ColorRole::none;
    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button outlined label color - toggle (unselected)
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;
};

template <>
struct Button<ButtonColors::outlined, States::Disabled>
{
    //! Button outlined disabled outline color
    static constexpr auto OutlineColor = ColorRole::outlineVariant;
};

template <>
struct Button<ButtonColors::tonal, States::Enabled>
{
    //! Button tonal shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button tonal icon color
    static constexpr auto IconColor = ColorRole::onSecondaryContainer;

    //! Button tonal container color
    static constexpr auto ContainerColor = ColorRole::secondaryContainer;

    //! Button tonal label color
    static constexpr auto LabelTextColor = ColorRole::onSecondaryContainer;
};

template <>
struct Button<ButtonColors::tonal, States::Enabled, ButtonVariant::ToggleSelected>
{
    //! Button tonal icon color - toggle (selected)
    static constexpr auto IconColor = ColorRole::onSecondary;

    //! Button tonal container color - toggle (selected)
    static constexpr auto ContainerColor = ColorRole::secondary;

    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button tonal label color - toggle (selected)
    static constexpr auto LabelTextColor = ColorRole::onSecondary;
};

template <>
struct Button<ButtonColors::tonal, States::Enabled, ButtonVariant::ToggleUnselected>
{
    //! Button tonal icon color - toggle (unselected)
    static constexpr auto IconColor = ColorRole::onSecondaryContainer;

    //! Button tonal container color - toggle (unselected)
    static constexpr auto ContainerColor = ColorRole::secondaryContainer;

    //! Button shadow color
    static constexpr auto ContainerShadowColor = ColorRole::shadow;

    //! Button tonal label color - toggle (unselected)
    static constexpr auto LabelTextColor = ColorRole::onSecondaryContainer;
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
    ButtonSizingValues values{};
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
    ColorRole shadowColor;
    ColorRole labelTextColor;
    ColorRole iconColor;
    float     containerOpacity;
};

template <ButtonColors Colors, States State, ButtonVariant Variant = ButtonVariant::Default>
constexpr auto GetButtonColors() -> ButtonColorsValues
{
    ButtonColorsValues values{};
    values.containerColor = Button<Colors, State, Variant>::ContainerColor;
    values.shadowColor    = Button<Colors, State, Variant>::ContainerShadowColor;
    values.labelTextColor = Button<Colors, State, Variant>::LabelTextColor;
    values.iconColor      = Button<Colors, State, Variant>::IconColor;
    return values;
}

constexpr auto GetButtonColors(ButtonColors colors, ButtonVariant variant) -> ButtonColorsValues
{
    ButtonColorsValues values{};
    switch (colors)
    {
        case ButtonColors::elevated: {
            switch (variant)
            {
                case ButtonVariant::Default:
                    values = GetButtonColors<ButtonColors::elevated, States::Enabled>();
                    break;
                case ButtonVariant::ToggleSelected:
                    values = GetButtonColors<ButtonColors::elevated, States::Enabled, ButtonVariant::ToggleSelected>();
                    break;
                case ButtonVariant::ToggleUnselected:
                    values = GetButtonColors<ButtonColors::elevated, States::Enabled, ButtonVariant::ToggleUnselected>();
                    break;
            }
            break;
        }
        case ButtonColors::filled: {
            switch (variant)
            {
                case ButtonVariant::Default:
                    values = GetButtonColors<ButtonColors::filled, States::Enabled>();
                    break;
                case ButtonVariant::ToggleSelected:
                    values = GetButtonColors<ButtonColors::filled, States::Enabled, ButtonVariant::ToggleSelected>();
                    break;
                case ButtonVariant::ToggleUnselected:
                    values = GetButtonColors<ButtonColors::filled, States::Enabled, ButtonVariant::ToggleUnselected>();
                    break;
            }
            break;
        }
        case ButtonColors::tonal: {
            switch (variant)
            {
                case ButtonVariant::Default:
                    values = GetButtonColors<ButtonColors::tonal, States::Enabled>();
                    break;
                case ButtonVariant::ToggleSelected:
                    values = GetButtonColors<ButtonColors::tonal, States::Enabled, ButtonVariant::ToggleSelected>();
                    break;
                case ButtonVariant::ToggleUnselected:
                    values = GetButtonColors<ButtonColors::tonal, States::Enabled, ButtonVariant::ToggleUnselected>();
                    break;
            }
            break;
        }
        case ButtonColors::outlined: {
            switch (variant)
            {
                case ButtonVariant::Default:
                    values = GetButtonColors<ButtonColors::outlined, States::Enabled>();
                    break;
                case ButtonVariant::ToggleSelected:
                    values = GetButtonColors<ButtonColors::outlined, States::Enabled, ButtonVariant::ToggleSelected>();
                    break;
                case ButtonVariant::ToggleUnselected:
                    values = GetButtonColors<ButtonColors::outlined, States::Enabled, ButtonVariant::ToggleUnselected>();
                    break;
            }
            break;
        }
        case ButtonColors::text: {
            switch (variant)
            {
                case ButtonVariant::Default:
                    values = GetButtonColors<ButtonColors::text, States::Enabled>();
                    break;
                case ButtonVariant::ToggleSelected:
                    values = GetButtonColors<ButtonColors::text, States::Enabled, ButtonVariant::ToggleSelected>();
                    break;
                case ButtonVariant::ToggleUnselected:
                    values = GetButtonColors<ButtonColors::text, States::Enabled, ButtonVariant::ToggleUnselected>();
                    break;
            }
            break;
        }
    }
    return values;
}
} // namespace ImGuiEx::M3::Spec
