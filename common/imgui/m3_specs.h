//
// Created by jamie on 2026/2/8.
//
#pragma once
#include <cstdint>

/**
 * This namespace defines size specifications for various Material Design 3 components.
 * All sizes value directly reference from Material3 design guidelines, represented in grid units (**4dp** each).
 * But, some size already optimized to better fit ImGui usage.
 * Such as, the FontSize will not contain the line height part. There will combine to padding or gap when needed.
 *
 * ## Usages
 * For follow the guidelines, you **SHOULD NOT** specify the size in pixels directly.
 *
 * If you want to scale components, it is recommended to call the ImGuiEx::M3::M3Styles::UpdateScaling method to update
 * the scale factor, which will automatically adjust all sizes accordingly.
 */
namespace ImGuiEx::M3::Spec
{

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

/////////////////////////////////////////////////
/// ... FAB ...
/////////////////////////////////////////////////

/**
 * No provide padding preset because padding usually not a multiple of grid unit.
 */
template <SizeTips Size>
struct FAB;

template <>
struct FAB<SizeTips::SMALL>
{
    static constexpr uint8_t iconSize = 6;  ///< 24dp
    static constexpr uint8_t size     = 14; ///< 56dp
    static constexpr uint8_t rounding = 4;  ///< 16dp
};

template <>
struct FAB<SizeTips::MEDIUM>
{
    static constexpr uint8_t iconSize = 7;  ///< 28dp
    static constexpr uint8_t size     = 20; ///< 80dp
    static constexpr uint8_t rounding = 5;  ///< 20dp
};

template <>
struct FAB<SizeTips::LARGE>
{
    static constexpr uint8_t iconSize = 9;  ///< 36dp
    static constexpr uint8_t size     = 24; ///< 96dp
    static constexpr uint8_t rounding = 7;  ///< 28dp
};

/////////////////////////////////////////////////
/// ... IconButton ...
/////////////////////////////////////////////////

//! No provide padding preset because padding usually not a multiple of grid unit.
template <SizeTips Size>
struct IconButton;

template <>
struct IconButton<SizeTips::XSMALL>
{
    static constexpr uint8_t iconSize = 5; ///< 20dp
    static constexpr uint8_t size     = 8; ///< 32dp
    static constexpr uint8_t margin   = 2; ///< 8dp
    static constexpr uint8_t rounding = 2; ///< 8dp
};

template <>
struct IconButton<SizeTips::SMALL>
{
    static constexpr uint8_t iconSize = 6;  ///< 24dp
    static constexpr uint8_t size     = 10; ///< 40dp
    static constexpr uint8_t margin   = 1;  ///< 4dp
    static constexpr uint8_t rounding = 2;  ///< 8dp
};

template <>
struct IconButton<SizeTips::MEDIUM>
{
    static constexpr uint8_t iconSize = 6;  ///< 24dp
    static constexpr uint8_t size     = 14; ///< 56dp
    static constexpr uint8_t margin   = 0;  ///< 0dp
    static constexpr uint8_t rounding = 3;  ///< 12dp
};

template <>
struct IconButton<SizeTips::LARGE>
{
    static constexpr uint8_t iconSize = 8;  ///< 32dp
    static constexpr uint8_t size     = 24; ///< 96dp
    static constexpr uint8_t margin   = 0;  ///< 0dp
    static constexpr uint8_t rounding = 4;  ///< 16dp
};

template <>
struct IconButton<SizeTips::XLARGE>
{
    static constexpr uint8_t iconSize = 10; ///< 40dp
    static constexpr uint8_t size     = 34; ///< 136dp
    static constexpr uint8_t margin   = 0;  ///< 0dp
    static constexpr uint8_t rounding = 4;  ///< 16dp
};

template <SizeTips Size>
struct Slider;

template <>
struct Slider<SizeTips::XSMALL>
{
    static constexpr uint8_t grabOuterWidth = 4;  ///< 16dp
    static constexpr uint8_t grabHeight     = 11; ///< 44dp
    static constexpr uint8_t frameHeight    = 4;  ///< 16dp
};

template <>
struct Slider<SizeTips::SMALL>
{
    static constexpr uint8_t grabOuterWidth = 4;  ///< 16dp
    static constexpr uint8_t grabHeight     = 11; ///< 44dp
    static constexpr uint8_t frameHeight    = 6;  ///< 24dp
};

template <>
struct Slider<SizeTips::MEDIUM>
{
    static constexpr uint8_t grabOuterWidth = 4;  ///< 16dp
    static constexpr uint8_t grabHeight     = 13; ///< 52dp
    static constexpr uint8_t frameHeight    = 10; ///< 40dp
};

template <>
struct Slider<SizeTips::LARGE>
{
    static constexpr uint8_t grabOuterWidth = 4;  ///< 16dp
    static constexpr uint8_t grabHeight     = 17; ///< 68dp
    static constexpr uint8_t frameHeight    = 14; ///< 56dp
};

template <>
struct Slider<SizeTips::XLARGE>
{
    static constexpr uint8_t grabOuterWidth = 4;  ///< 16dp
    static constexpr uint8_t grabHeight     = 27; ///< 108dp
    static constexpr uint8_t frameHeight    = 24; ///< 96dp
};

/////////////////////////////////////////////////
/// ... Other Small Components ...
/////////////////////////////////////////////////

struct Search
{
    static constexpr uint8_t paddingY = 4; ///< 16dp
    static constexpr uint8_t paddingX = 4; ///< 16dp
    static constexpr uint8_t rounding = 7; ///< 28dp
    static constexpr uint8_t gap      = 4; ///< 16dp
};

struct List
{
    static constexpr uint8_t paddingY = 4;  ///< 16dp
    static constexpr uint8_t paddingX = 4;  ///< 16dp
    static constexpr uint8_t height   = 13; ///< 52dp
    //! This size will be computed on every call times because it is greater than 32, but it is not a problem because it
    //! is only used in some specific components, such as the navigation rail.
    static constexpr uint8_t width = 70; ///< 280dp.
};

} // namespace ImGuiEx::M3::Spec
