//
// Created by jamie on 2026/2/24.
//

#pragma once

#include "../spec/button_groups.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using StandardXSmallButtonGroup = ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::XSMALL>;
using StandardSmallButtonGroup  = ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::SMALL>;
using StandardMediumButtonGroup = ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::MEDIUM>;
using StandardLargeButtonGroup  = ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::LARGE>;
using StandardXLargeButtonGroup = ButtonGroupSizing<ButtonGroupVariant::Standard, SizeTips::XLARGE>;

using ConnectedXSmallButtonGroup = ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::XSMALL>;
using ConnectedSmallButtonGroup  = ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::SMALL>;
using ConnectedMediumButtonGroup = ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::MEDIUM>;
using ConnectedLargeButtonGroup  = ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::LARGE>;
using ConnectedXLargeButtonGroup = ButtonGroupSizing<ButtonGroupVariant::Connected, SizeTips::XLARGE>;
} // namespace ImGuiEx::M3::Spec
