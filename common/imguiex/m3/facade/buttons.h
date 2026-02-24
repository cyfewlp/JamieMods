//
// Created by jamie on 2026/2/24.
//

#pragma once

#include "../spec/buttons.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using FilledButtonHovered  = ButtonFilled<States::Hovered>;
using FilledButtonDisabled = ButtonFilled<States::Disabled>;
using FilledButtonPressed  = ButtonFilled<States::Pressed>;
using FilledButtonFocused  = ButtonFilled<States::Focused>;
} // namespace ImGuiEx::M3::Spec
