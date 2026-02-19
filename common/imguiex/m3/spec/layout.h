//
// Created by jamie on 2026/2/13.
//

#pragma once

#include "base.h"

namespace ImGuiEx::M3::Spec
{
struct Layout
{
    static constexpr float breakpointMedium = 600.0F;
    static constexpr float breakpointLarge  = 840.0F;

    struct Medium
    {
        static constexpr Unit margin = dp<24>();
    };

    struct Compact
    {
        static constexpr Unit margin = dp<16>();
    };
};
} // namespace ImGuiEx::M3::Spec
