//
// Created by jamie on 2026/2/10.
//

#pragma once

#include <concepts>
#include <cstdint>
#include <limits>

namespace ImGuiEx::M3::Spec
{

/**
 *  **Material Design 3 Core Unit**
 *
 * Represents the base measurement unit where:
 *
 * @code px=units×BASE_UNIT×currentScale @endcode
 *
 * **Constraints:**
 *
 * @note Range is restricted to `UINT8_MAX` by static assert. Exceeding this limit suggests a
 * layout design flaw; refactoring is recommended over expanding the type.
 *
 * @details Using units facilitates pre-computing common sizes into lookup
 * tables for performance while improving code maintainability.
 *
 */
using Unit = std::uint32_t;

/**
 * Some size not respect the 4dp grid unit, such as ButtonGroup::gap.
 */
static constexpr uint32_t BASE_UNIT = 2U;

template <uint32_t dpValue>
consteval auto dp() -> uint32_t
{
    static_assert(dpValue % BASE_UNIT == 0U, "dp value must be a multiple of BASE_UNIT");
    static_assert(dpValue / BASE_UNIT <= std::numeric_limits<uint8_t>::max(), "Too many units!");
    return dpValue / BASE_UNIT;
}

/**
 * NOTE: some components only have a subset of these sizes.
 */
enum class SizeTips : std::uint8_t
{
    XSMALL,
    SMALL,
    MEDIUM,
    LARGE,
    XLARGE,
};

template <typename T>
concept HasPaddingY = requires { T::paddingY; } && std::same_as<decltype(T::paddingY), const Unit>;

template <typename T>
concept HasPaddingX = requires { T::paddingX; } && std::same_as<decltype(T::paddingX), const Unit>;

template <typename T>
concept HasExactPadding = HasPaddingX<T> && HasPaddingY<T>;

template <typename T>
concept LacksPaddingX = !HasPaddingX<T>;

template <typename T>
concept LacksPaddingY = !HasPaddingY<T>;

template <typename T>
concept HasRounding = requires { T::rounding; } && std::same_as<decltype(T::rounding), const Unit>;

template <typename T>
concept HasGap = requires { T::gap; } && std::same_as<decltype(T::gap), const Unit>;

template <typename T>
concept HasWidth = requires { T::width; } && std::same_as<decltype(T::width), const Unit>;

template <typename T>
concept HasHeight = requires { T::height; } && std::same_as<decltype(T::height), const Unit>;

} // namespace ImGuiEx::M3::Spec
