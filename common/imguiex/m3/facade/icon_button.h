//
// Created by jamie on 2026/2/23.
//

#pragma once

#include "../spec/icon_button.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using FilledIconButtonEnabled  = IconButtonFilled<States::Enabled>;
using FilledIconButtonDisabled = IconButtonFilled<States::Disabled>;
using FilledIconButtonHovered  = IconButtonFilled<States::Hovered>;
using FilledIconButtonFocused  = IconButtonFilled<States::Focused>;
using FilledIconButtonPressed  = IconButtonFilled<States::Pressed>;

using TonalIconButtonEnabled  = IconButtonTonal<States::Enabled>;
using TonalIconButtonDisabled = IconButtonTonal<States::Disabled>;

using OutlinedIconButtonEnabled  = IconButtonOutlined<States::Enabled>;
using OutlinedIconButtonDisabled = IconButtonOutlined<States::Disabled>;

using StandardIconButtonEnabled  = IconButtonStandard<States::Enabled>;
using StandardIconButtonDisabled = IconButtonStandard<States::Disabled>;

using ExtraSmallIconButton = IconButtonSizing<SizeTips::XSMALL>;
using SmallIconButton      = IconButtonSizing<SizeTips::SMALL>;
using MediumIconButton     = IconButtonSizing<SizeTips::MEDIUM>;
using LargeIconButton      = IconButtonSizing<SizeTips::LARGE>;
using ExtraLargeIconButton = IconButtonSizing<SizeTips::XLARGE>;

} // namespace ImGuiEx::M3::Spec
