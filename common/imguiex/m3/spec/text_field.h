//
// Created by jamie on 2026/2/14.
//
#pragma once

#include "base.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{

enum class TextFieldStyle : uint8_t
{
    Filled,
    Outlined
};

enum class TextFieldState : uint8_t
{
    Enabled,
    Focused
};

template <TextFieldStyle Style, TextFieldState State>
struct TextField;

struct TextFieldBase
{
    static constexpr auto textRole        = TextRole::BodyLarge;
    static constexpr auto supportTextRole = TextRole::BodySmall;
    static constexpr Unit height          = dp<56>();
    static constexpr Unit paddingX        = dp<16>();
    static constexpr Unit paddingY        = dp<8>();
    //! extra field
    static constexpr Unit contentHeight   = height - paddingY * 2U;
};

template <>
struct TextField<TextFieldStyle::Filled, TextFieldState::Enabled> : public TextFieldBase
{
    using TextFieldBase::TextFieldBase;
    static constexpr float activeIndicatorHeight = 1.0F;
};

template <>
struct TextField<TextFieldStyle::Filled, TextFieldState::Focused> : public TextFieldBase
{
    using TextFieldBase::TextFieldBase;
    static constexpr float activeIndicatorThickness = 3.0F;
    static constexpr float activeIndicatorHeight    = 2.0F;
};

} // namespace ImGuiEx::M3::Spec
