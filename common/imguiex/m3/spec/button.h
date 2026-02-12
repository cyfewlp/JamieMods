//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "base.h"
#include "typogrphy.h"

namespace ImGuiEx::M3::Spec
{
//! No provide padding preset because padding usually not a multiple of grid unit.
template <SizeTips Size>
struct IconButton;

template <>
struct IconButton<SizeTips::XSMALL>
{
    static constexpr Unit iconSize = dp<20>();
    static constexpr Unit size     = dp<32>();
    static constexpr Unit margin   = dp<8>();
    static constexpr Unit rounding = dp<8>();
};

template <>
struct IconButton<SizeTips::SMALL>
{
    static constexpr Unit iconSize = dp<24>();
    static constexpr Unit size     = dp<40>();
    static constexpr Unit margin   = dp<4>();
    static constexpr Unit rounding = dp<8>();
};

template <>
struct IconButton<SizeTips::MEDIUM>
{
    static constexpr Unit iconSize = dp<24>();
    static constexpr Unit size     = dp<56>();
    static constexpr Unit margin   = dp<0>();
    static constexpr Unit rounding = dp<12>();
};

template <>
struct IconButton<SizeTips::LARGE>
{
    static constexpr Unit iconSize = dp<32>();
    static constexpr Unit size     = dp<96>();
    static constexpr Unit margin   = dp<0>();
    static constexpr Unit rounding = dp<16>();
};

template <>
struct IconButton<SizeTips::XLARGE>
{
    static constexpr Unit iconSize = dp<40>();
    static constexpr Unit size     = dp<136>();
    static constexpr Unit margin   = dp<0>();
    static constexpr Unit rounding = dp<16>();
};

template <SizeTips Size>
struct Button;

template <>
struct Button<SizeTips::XSMALL>
{
    static constexpr auto textRole = TextRole::LabelLarge;
    static constexpr Unit iconSize = dp<20>();
    static constexpr Unit height   = dp<32>();
    static constexpr Unit paddingX = dp<12>();
    static constexpr Unit marginY  = dp<8>();
    static constexpr Unit rounding = dp<100>();
};

template <>
struct Button<SizeTips::SMALL>
{
    static constexpr auto textRole = TextRole::LabelLarge;
    static constexpr Unit iconSize = dp<20>();
    static constexpr Unit height   = dp<40>();
    static constexpr Unit paddingX = dp<16>();
    static constexpr Unit marginY  = dp<4>();
    static constexpr Unit rounding = dp<100>();
};

template <>
struct Button<SizeTips::MEDIUM>
{
    static constexpr auto textRole = TextRole::TitleMedium;
    static constexpr Unit iconSize = dp<24>();
    static constexpr Unit height   = dp<56>();
    static constexpr Unit paddingX = dp<24>();
    static constexpr Unit margin   = dp<0>();
    static constexpr Unit rounding = dp<100>();
};

template <>
struct Button<SizeTips::LARGE>
{
    static constexpr auto textRole = TextRole::HeadlineSmall;
    static constexpr Unit iconSize = dp<32>();
    static constexpr Unit height   = dp<96>();
    static constexpr Unit paddingX = dp<48>();
    static constexpr Unit margin   = dp<0>();
    static constexpr Unit rounding = dp<100>();
};

template <>
struct Button<SizeTips::XLARGE>
{
    static constexpr auto textRole = TextRole::HeadlineLarge;
    static constexpr Unit iconSize = dp<40>();
    static constexpr Unit height   = dp<136>();
    static constexpr Unit paddingX = dp<64>();
    static constexpr Unit margin   = dp<0>();
    static constexpr Unit rounding = dp<100>();
};

template <SizeTips Size>
struct ButtonGroup;

template <>
struct ButtonGroup<SizeTips::XSMALL>
{
    static constexpr Unit gap = dp<18>();
};

template <>
struct ButtonGroup<SizeTips::SMALL>
{
    static constexpr Unit gap = dp<12>();
};

template <>
struct ButtonGroup<SizeTips::MEDIUM>
{
    static constexpr Unit gap = dp<8>();
};

template <>
struct ButtonGroup<SizeTips::LARGE>
{
    static constexpr Unit gap = dp<8>();
};

template <>
struct ButtonGroup<SizeTips::XLARGE>
{
    static constexpr Unit gap = dp<8>();
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
    static constexpr Unit iconSize = dp<24>();
    static constexpr Unit size     = dp<56>();
    static constexpr Unit rounding = dp<16>();
};

template <>
struct FAB<SizeTips::MEDIUM>
{
    static constexpr Unit iconSize = dp<28>();
    static constexpr Unit size     = dp<80>();
    static constexpr Unit rounding = dp<20>();
};

template <>
struct FAB<SizeTips::LARGE>
{
    static constexpr Unit iconSize = dp<36>();
    static constexpr Unit size     = dp<96>();
    static constexpr Unit rounding = dp<28>();
};

} // namespace ImGuiEx::M3::Spec
