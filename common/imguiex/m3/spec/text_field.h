//
// Created by jamie on 2026/2/20.
//

#pragma once

#include "base.h"
#include "color_roles.h"
#include "others.h"
#include "shapes.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{

enum class TextFieldVariant : uint8_t
{
    Filled,
    Outlined
};

enum class TextFieldState : uint8_t
{
    Enabled,
    Focused,
    Hovered,
    Disabled,
};

struct TextFieldCommon
{
    static constexpr Unit Height   = dp<56>();
    static constexpr Unit PaddingX = dp<16>();
    static constexpr Unit PaddingY = dp<8>();
    static constexpr Unit Gap      = dp<16>();
    static constexpr Unit MinWidth = PaddingX * 2U;

    //! Outlined text field trailing icon size
    static constexpr auto TrailingIconSize = dp<24>();

    //! Outlined text field leading icon size
    static constexpr auto LeadingIconSize = dp<24>();

    //! Outlined text field supporting text size
    static constexpr auto SupportingTextRole = TextRole::BodySmall;

    //! Outlined text field input text size
    static constexpr auto InputTextRole = TextRole::BodyLarge;

    //! Outlined text field label text size
    static constexpr auto LabelTextRole = TextRole::BodyLarge;

    //! Outlined text field label text populated size
    static constexpr auto LabelTextPopulatedRole = TextRole::BodySmall;

    //! Outlined text field caret color
    static constexpr auto CaretColor = ColorRole::primary;

    //! Outlined text field trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Outlined text field input text color
    static constexpr auto InputTextColor = ColorRole::onSurface;

    //! Outlined text field input text placeholder color
    static constexpr auto InputTextPlaceholderColor = ColorRole::onSurfaceVariant;

    //! Outlined text field input text prefix color
    static constexpr auto InputTextPrefixColor = ColorRole::onSurfaceVariant;

    //! Outlined text field input text suffix color
    static constexpr auto InputTextSuffixColor = ColorRole::onSurfaceVariant;

    //! Outlined text field error trailing icon color
    static constexpr auto ErrorTrailingIconColor = ColorRole::error;

    //! Outlined text field error leading icon color
    static constexpr auto ErrorLeadingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field error supporting text color
    static constexpr auto ErrorSupportingTextColor = ColorRole::error;

    //! Outlined text field error input text color
    static constexpr auto ErrorInputTextColor = ColorRole::onSurface;

    //! Outlined text field error label text color
    static constexpr auto ErrorLabelTextColor = ColorRole::error;

    //! Outlined text field label text color
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;

    //! Outlined text field container shape
    static constexpr auto ContainerShape = ShapeCorner::ExtraSmall;
};

template <TextFieldVariant Variant, TextFieldState State>
struct TextField;

template <>
struct TextField<TextFieldVariant::Outlined, TextFieldState::Enabled> : public TextFieldCommon
{
    using TextFieldCommon::TextFieldCommon;

    //! Outlined text field outline width
    static constexpr float OutlineWidth = 1.0F;

    //! Outlined text field error outline color
    static constexpr auto ErrorOutlineColor = ColorRole::error;

    //! Outlined text field outline color
    static constexpr auto OutlineColor = ColorRole::outline;

    //! Filled text field container color
    static constexpr auto ContainerColor = ColorRole::none;

    //! Outlined text field focus outline width
    static constexpr auto LabelTextPopulatedContainerPaddingX = dp<4>();
};

using OutlinedTextField = TextField<TextFieldVariant::Outlined, TextFieldState::Enabled>;

template <>
struct TextField<TextFieldVariant::Outlined, TextFieldState::Hovered> : public OutlinedTextField
{
    using OutlinedTextField::OutlinedTextField;

    //! Outlined text field error hover trailing icon color
    static constexpr auto ErrorTrailingIconColor = ColorRole::onErrorContainer;

    //! Outlined text field error hover leading icon color
    static constexpr auto ErrorLeadingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field error hover supporting text color
    static constexpr auto ErrorSupportingTextColor = ColorRole::error;

    //! Outlined text field error hover input text color
    static constexpr auto ErrorInputTextColor = ColorRole::onSurface;

    //! Outlined text field error hover label text color
    static constexpr auto ErrorLabelTextColor = ColorRole::onErrorContainer;

    //! Outlined text field error hover outline color
    static constexpr auto ErrorOutlineColor = ColorRole::onErrorContainer;

    //! Outlined text field hover trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field hover leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field hover supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Outlined text field hover label text color
    static constexpr auto LabelTextColor = ColorRole::onSurface;

    //! Outlined text field hover outline color
    static constexpr auto OutlineColor = ColorRole::onSurface;
};

template <>
struct TextField<TextFieldVariant::Outlined, TextFieldState::Focused> : public OutlinedTextField
{
    using OutlinedTextField::OutlinedTextField;

    //! Outlined text field error focus caret color
    static constexpr auto ErrorCaretColor = ColorRole::error;

    //! Outlined text field error focus trailing icon color
    static constexpr auto ErrorTrailingIconColor = ColorRole::error;

    //! Outlined text field error focus leading icon color
    static constexpr auto ErrorLeadingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field error focus supporting text color
    static constexpr auto ErrorSupportingTextColor = ColorRole::error;

    //! Outlined text field error focus input text color
    static constexpr auto ErrorInputTextColor = ColorRole::onSurface;

    //! Outlined text field error focus label text color
    static constexpr auto ErrorLabelTextColor = ColorRole::error;

    //! Outlined text field error focus outline color
    static constexpr auto ErrorOutlineColor = ColorRole::error;

    //! Outlined text field focus trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field focus leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurfaceVariant;

    //! Outlined text field focus supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Outlined text field focus label text color
    static constexpr auto LabelTextColor = ColorRole::primary;

    //! Outlined text field focus outline color
    static constexpr auto OutlineColor = ColorRole::primary;

    //! Outlined text field focus outline width
    static constexpr float OutlineWidth = 3.0F;
};

template <>
struct TextField<TextFieldVariant::Outlined, TextFieldState::Disabled> : public OutlinedTextField
{
    using OutlinedTextField::OutlinedTextField;

    //! Outlined text field disabled trailing-icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurface;

    //! Outlined text field disabled leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurface;

    //! Outlined text field disabled supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurface;

    //! Outlined text field disabled label text color
    static constexpr auto LabelTextColor = ColorRole::onSurface;

    //! Outlined text field disabled outline color
    static constexpr auto OutlineColor = ColorRole::onSurface;
};

template <>
struct TextField<TextFieldVariant::Filled, TextFieldState::Enabled> : public TextFieldCommon
{
    using TextFieldCommon::TextFieldCommon;

    //! Filled text field error active indicator color
    static constexpr auto ErrorActiveIndicatorColor = ColorRole::error;

    //! Filled text field active indicator color
    static constexpr auto ActiveIndicatorColor = ColorRole::onSurfaceVariant;

    //! Filled text field active indicator height
    static constexpr float ActiveIndicatorHeight = 1.0F;

    //! Filled text field focus active indicator thickness
    static constexpr float ActiveIndicatorThickness = 1.0F;

    //! Filled text field container color
    static constexpr auto ContainerColor = ColorRole::surfaceContainerHighest;
};

using FilledTextField = TextField<TextFieldVariant::Filled, TextFieldState::Enabled>;

template <>
struct TextField<TextFieldVariant::Filled, TextFieldState::Focused> : public FilledTextField
{
    using FilledTextField::FilledTextField;
    //! Filled text field focus active indicator thickness
    static constexpr float ActiveIndicatorThickness = 3.0F;

    //! Filled text field error focus caret color
    static constexpr auto ErrorCaretColor = ColorRole::error;

    //! Filled text field error focus trailing icon color
    static constexpr auto ErrorTrailingIconColor = ColorRole::error;

    //! Filled text field error focus leading icon color
    static constexpr auto ErrorLeadingIconColor = ColorRole::onSurfaceVariant;

    //! Filled text field error focus supporting text color
    static constexpr auto ErrorSupportingTextColor = ColorRole::error;

    //! Filled text field error focus input text color
    static constexpr auto ErrorInputTextColor = ColorRole::onSurface;

    //! Filled text field error focus label text color
    static constexpr auto ErrorLabelTextColor = ColorRole::error;

    //! Filled text field error focus active indicator color
    static constexpr auto ErrorActiveIndicatorColor = ColorRole::error;

    //! Filled text field focus trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Filled text field focus leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurfaceVariant;

    //! Filled text field focus supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Filled text field focus label text color
    static constexpr auto LabelTextColor = ColorRole::primary;

    //! Filled text field focus active indicator color
    static constexpr auto ActiveIndicatorColor = ColorRole::primary;

    //! Filled text field focus active indicator height
    static constexpr auto ActiveIndicatorHeight = dp<2>();
};

template <>
struct TextField<TextFieldVariant::Filled, TextFieldState::Hovered> : public FilledTextField
{
    using FilledTextField::FilledTextField;

    //! Filled text field error hover state layer color
    static constexpr auto ErrorStateLayerColor = ColorRole::onSurface;

    //! Filled text field error hover trailing icon color
    static constexpr auto ErrorTrailingIconColor = ColorRole::onErrorContainer;

    //! Filled text field error hover leading icon color
    static constexpr auto ErrorLeadingIconColor = ColorRole::onSurfaceVariant;

    //! Filled text field error hover supporting text color
    static constexpr auto ErrorSupportingTextColor = ColorRole::error;

    //! Filled text field error hover input text color
    static constexpr auto ErrorInputTextColor = ColorRole::onSurface;

    //! Filled text field error hover label text color
    static constexpr auto ErrorLabelTextColor = ColorRole::onErrorContainer;

    //! Filled text field error hover active indicator color
    static constexpr auto ErrorActiveIndicatorColor = ColorRole::onErrorContainer;

    //! Filled text field hover state layer color
    static constexpr auto StateLayerColor = ColorRole::onSurface;

    //! Filled text field hover trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurfaceVariant;

    //! Filled text field hover leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurfaceVariant;

    //! Filled text field hover supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurfaceVariant;

    //! Filled text field hover label text color
    static constexpr auto LabelTextColor = ColorRole::onSurfaceVariant;

    //! Filled text field hover active indicator color
    static constexpr auto ActiveIndicatorColor = ColorRole::onSurface;
};

template <>
struct TextField<TextFieldVariant::Filled, TextFieldState::Disabled> : public FilledTextField
{
    using FilledTextField::FilledTextField;

    //! Filled text field disabled trailing icon color
    static constexpr auto TrailingIconColor = ColorRole::onSurface;

    //! Filled text field disabled leading icon color
    static constexpr auto LeadingIconColor = ColorRole::onSurface;

    //! Filled text field disabled supporting text color
    static constexpr auto SupportingTextColor = ColorRole::onSurface;

    //! Filled text field disabled label text color
    static constexpr auto LabelTextColor = ColorRole::onSurface;

    //! Filled text field disabled active indicator color
    static constexpr auto ActiveIndicatorColor = ColorRole::onSurface;

    //! Filled text field disabled active indicator height
    static constexpr float ActiveIndicatorHeight = 1.0F;

    //! Filled text field disabled container color
    static constexpr auto ContainerColor = ColorRole::onSurface;

    //! Filled text field disabled container opacity
    static constexpr auto ContainerOpacity = 0.04F;
};

} // namespace ImGuiEx::M3::Spec
