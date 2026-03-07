//
// Created by jamie on 2026/2/13.
//

#pragma once

#include "base.h"

namespace ImGuiEx::M3::Spec
{
struct Layout
{

    struct Compact
    {
        static constexpr float Breakpoint = 0.0F;
        static constexpr Unit  Margin     = dp<16>();
    };

    struct Medium
    {
        static constexpr float Breakpoint = 600.0F;
        static constexpr Unit  Margin     = dp<24>();
    };

    struct Expanded
    {
        static constexpr float Breakpoint     = 840.0F;
        static constexpr Unit  Margin         = dp<24>();
        static constexpr auto  FixedPaneWidth = 360.0F;
    };

    struct Large
    {
        static constexpr float Breakpoint     = 1200.0F;
        static constexpr Unit  Margin         = dp<24>();
        static constexpr auto  FixedPaneWidth = 412.0F;
    };

    struct ExtraLarge
    {
        static constexpr float Breakpoint         = 1600.0F;
        static constexpr Unit  Margin             = dp<24>();
        static constexpr auto  FixedPaneWidth     = 412.0F;
        static constexpr auto  SideSheetsMaxWidth = 400.0F;
    };
};
} // namespace ImGuiEx::M3::Spec
