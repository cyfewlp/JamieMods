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
    static constexpr Unit grabOuterWidth = dp<16>();
    static constexpr Unit grabHeight     = dp<44>();
    static constexpr Unit frameHeight    = dp<16>();
};

template <>
struct Slider<SizeTips::SMALL>
{
    static constexpr Unit grabOuterWidth = dp<16>();
    static constexpr Unit grabHeight     = dp<44>();
    static constexpr Unit frameHeight    = dp<24>();
};

template <>
struct Slider<SizeTips::MEDIUM>
{
    static constexpr Unit grabOuterWidth = dp<16>();
    static constexpr Unit grabHeight     = dp<52>();
    static constexpr Unit frameHeight    = dp<40>();
};

template <>
struct Slider<SizeTips::LARGE>
{
    static constexpr Unit grabOuterWidth = dp<16>();
    static constexpr Unit grabHeight     = dp<68>();
    static constexpr Unit frameHeight    = dp<56>();
};

template <>
struct Slider<SizeTips::XLARGE>
{
    static constexpr Unit grabOuterWidth = dp<16>();
    static constexpr Unit grabHeight     = dp<108>();
    static constexpr Unit frameHeight    = dp<96>();
};
} // namespace ImGuiEx::M3::Spec
