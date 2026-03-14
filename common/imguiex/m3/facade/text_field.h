//
// Created by jamie on 2026/2/14.
//
#pragma once

#include "../spec/text_field.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using FilledTextFieldEnabled    = TextField<TextFieldVariant::Filled, TextFieldState::Enabled>;
using FilledTextFieldHovered    = TextField<TextFieldVariant::Filled, TextFieldState::Hovered>;
using FilledTextFieldFocused    = TextField<TextFieldVariant::Filled, TextFieldState::Focused>;
using FilledTextFieldDisabled   = TextField<TextFieldVariant::Filled, TextFieldState::Disabled>;
using OutlinedTextFieldEnabled  = TextField<TextFieldVariant::Outlined, TextFieldState::Enabled>;
using OutlinedTextFieldHovered  = TextField<TextFieldVariant::Outlined, TextFieldState::Hovered>;
using OutlinedTextFieldFocused  = TextField<TextFieldVariant::Outlined, TextFieldState::Focused>;
using OutlinedTextFieldDisabled = TextField<TextFieldVariant::Outlined, TextFieldState::Disabled>;
} // namespace ImGuiEx::M3::Spec
