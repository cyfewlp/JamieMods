//
// Created by jamie on 2026/2/13.
//

#pragma once

#include "base.h"

namespace ImGuiEx::M3::Spec
{
struct Layout
{
    static constexpr uint32_t breakpointMedium = 600U;
    static constexpr uint32_t breakpointLarge  = 840U;

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
