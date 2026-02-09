//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "base.h"

namespace ImGuiEx::M3::Spec
{
template <SizeTips Size>
struct Slider;

template <>
struct Slider<SizeTips::XSMALL>
{
    static constexpr Unit grabOuterWidth = 4;  ///< 16dp
    static constexpr Unit grabHeight     = 11; ///< 44dp
    static constexpr Unit frameHeight    = 4;  ///< 16dp
};

template <>
struct Slider<SizeTips::SMALL>
{
    static constexpr Unit grabOuterWidth = 4;  ///< 16dp
    static constexpr Unit grabHeight     = 11; ///< 44dp
    static constexpr Unit frameHeight    = 6;  ///< 24dp
};

template <>
struct Slider<SizeTips::MEDIUM>
{
    static constexpr Unit grabOuterWidth = 4;  ///< 16dp
    static constexpr Unit grabHeight     = 13; ///< 52dp
    static constexpr Unit frameHeight    = 10; ///< 40dp
};

template <>
struct Slider<SizeTips::LARGE>
{
    static constexpr Unit grabOuterWidth = 4;  ///< 16dp
    static constexpr Unit grabHeight     = 17; ///< 68dp
    static constexpr Unit frameHeight    = 14; ///< 56dp
};

template <>
struct Slider<SizeTips::XLARGE>
{
    static constexpr Unit grabOuterWidth = 4;  ///< 16dp
    static constexpr Unit grabHeight     = 27; ///< 108dp
    static constexpr Unit frameHeight    = 24; ///< 96dp
};
} // namespace ImGuiEx::M3::Spec
