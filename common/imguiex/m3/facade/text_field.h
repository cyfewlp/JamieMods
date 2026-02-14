//
// Created by jamie on 2026/2/14.
//
#pragma once

#include "../spec/text_field.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using FilledTextFieldEnabled   = TextField<TextFieldStyle::Filled, TextFieldState::Enabled>;
using FilledTextFieldFocused   = TextField<TextFieldStyle::Filled, TextFieldState::Focused>;
using OutlinedTextFieldEnabled = TextField<TextFieldStyle::Outlined, TextFieldState::Enabled>;
using OutlinedTextFieldFocused = TextField<TextFieldStyle::Outlined, TextFieldState::Focused>;
} // namespace ImGuiEx::M3::Spec
