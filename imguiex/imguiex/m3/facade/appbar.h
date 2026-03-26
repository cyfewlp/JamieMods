//
// Created by jamie on 2026/3/4.
//

#pragma once

#include "../spec/appbar.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using SmallAppbar          = AppBarSizing<AppBarVariant::Small>;
using MediumFlexibleAppbar = AppBarSizing<AppBarVariant::MediumFlexible>;
using LargeFlexibleAppbar  = AppBarSizing<AppBarVariant::LargeFlexible>;
} // namespace ImGuiEx::M3::Spec
