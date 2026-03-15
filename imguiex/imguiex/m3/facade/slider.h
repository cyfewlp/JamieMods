//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "../spec/slider.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using ExtraSmallSlider = Slider<SizeTips::XSMALL>;
using SmallSlider      = Slider<SizeTips::SMALL>;
using MediumSlider     = Slider<SizeTips::MEDIUM>;
using LargeSlider      = Slider<SizeTips::LARGE>;
using ExtraSlider      = Slider<SizeTips::XLARGE>;
} // namespace ImGuiEx::M3::Spec
