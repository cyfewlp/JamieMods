//
// Created by jamie on 2026/2/18.
//
#pragma once

#include "../spec/nav_rail.h"
#include "base.h"

namespace ImGuiEx::M3::Spec
{
using CollapsedNavRail = NavRail<NavRailState::Collapsed>;
using ExpandedNavRail  = NavRail<NavRailState::Expanded>;
} // namespace ImGuiEx::M3::Spec
