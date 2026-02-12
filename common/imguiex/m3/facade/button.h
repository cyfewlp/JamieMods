//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "../spec/button.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using ExtraSmallIconButton = IconButton<SizeTips::XSMALL>;
using SmallIconButton      = IconButton<SizeTips::SMALL>;
using MediumIconButton     = IconButton<SizeTips::MEDIUM>;
using LargeIconButton      = IconButton<SizeTips::LARGE>;
using ExtraIconButton      = IconButton<SizeTips::XLARGE>;

using ExtraSmallButton = Button<SizeTips::XSMALL>;
using SmallButton      = Button<SizeTips::SMALL>;
using MediumButton     = Button<SizeTips::MEDIUM>;
using LargeButton      = Button<SizeTips::LARGE>;
using ExtraButton      = Button<SizeTips::XLARGE>;

using ExtraSmallButtonGroup = ButtonGroup<SizeTips::XSMALL>;
using SmallButtonGroup      = ButtonGroup<SizeTips::SMALL>;
using MediumButtonGroup     = ButtonGroup<SizeTips::MEDIUM>;
using LargeButtonGroup      = ButtonGroup<SizeTips::LARGE>;
using ExtraButtonGroup      = ButtonGroup<SizeTips::XLARGE>;
} // namespace ImGuiEx::M3::Spec
