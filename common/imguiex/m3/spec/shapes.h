//
// Created by jamie on 2026/2/19.
//

#pragma once

#include "base.h"

namespace ImGuiEx::M3::Spec
{
struct ShapeCorner
{
    static constexpr auto ExtraSmall = dp<4>();
    static constexpr auto Small      = dp<8>();
    static constexpr auto Medium     = dp<12>();
    static constexpr auto Large      = dp<16>();
    static constexpr auto ExtraLarge = dp<28>();
};
} // namespace ImGuiEx::M3::Spec
