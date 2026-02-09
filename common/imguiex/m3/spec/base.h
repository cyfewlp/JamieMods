//
// Created by jamie on 2026/2/10.
//

#pragma once

#include <cstdint>

namespace ImGuiEx::M3::Spec
{
/**
 * Base unit for all sizes in Material Design 3. 1 unit = 4dp.
 * The range is limit to uint8_t if you need a greate size that may mean your layout is broken, you should consider
 * refactor your layout instead of just increase the size.
 */
using Unit = std::uint8_t;

/**
 * NOTE: some components only have a subset of these sizes.
 */
enum class SizeTips : uint8_t
{
    XSMALL = 0,
    SMALL  = 1,
    MEDIUM = 2,
    LARGE  = 4,
    XLARGE = 8
};
} // namespace ImGuiEx::M3::Spec
