//
// Created by jamie on 2026/2/24.
//

#pragma once

#include "../spec/buttons.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using ElevatedButtonEnabled  = Button<ButtonColors::elevated, States::Enabled>;
using ElevatedButtonDisabled = Button<ButtonColors::elevated, States::Disabled>;

using FilledButtonEnabled  = Button<ButtonColors::filled, States::Enabled>;
using FilledButtonDisabled = Button<ButtonColors::filled, States::Disabled>;

using OutlinedButtonEnabled  = Button<ButtonColors::outlined, States::Enabled>;
using OutlinedButtonDisabled = Button<ButtonColors::outlined, States::Disabled>;
} // namespace ImGuiEx::M3::Spec
