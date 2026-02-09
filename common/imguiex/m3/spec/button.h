//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "base.h"
#include "text_role.h"

namespace ImGuiEx::M3::Spec
{
//! No provide padding preset because padding usually not a multiple of grid unit.
template <SizeTips Size>
struct IconButton;

template <>
struct IconButton<SizeTips::XSMALL>
{
    static constexpr Unit iconSize = 5; ///< 20dp
    static constexpr Unit size     = 8; ///< 32dp
    static constexpr Unit margin   = 2; ///< 8dp
    static constexpr Unit rounding = 2; ///< 8dp
};

template <>
struct IconButton<SizeTips::SMALL>
{
    static constexpr Unit iconSize = 6;  ///< 24dp
    static constexpr Unit size     = 10; ///< 40dp
    static constexpr Unit margin   = 1;  ///< 4dp
    static constexpr Unit rounding = 2;  ///< 8dp
};

template <>
struct IconButton<SizeTips::MEDIUM>
{
    static constexpr Unit iconSize = 6;  ///< 24dp
    static constexpr Unit size     = 14; ///< 56dp
    static constexpr Unit margin   = 0;  ///< 0dp
    static constexpr Unit rounding = 3;  ///< 12dp
};

template <>
struct IconButton<SizeTips::LARGE>
{
    static constexpr Unit iconSize = 8;  ///< 32dp
    static constexpr Unit size     = 24; ///< 96dp
    static constexpr Unit margin   = 0;  ///< 0dp
    static constexpr Unit rounding = 4;  ///< 16dp
};

template <>
struct IconButton<SizeTips::XLARGE>
{
    static constexpr Unit iconSize = 10; ///< 40dp
    static constexpr Unit size     = 34; ///< 136dp
    static constexpr Unit margin   = 0;  ///< 0dp
    static constexpr Unit rounding = 4;  ///< 16dp
};

template <SizeTips Size>
struct Button;

template <>
struct Button<SizeTips::XSMALL>
{
    static constexpr auto textRole = TextRole::LabelLarge;
    static constexpr Unit iconSize = 5;  ///< 20dp
    static constexpr Unit height   = 8;  ///< 32dp
    static constexpr Unit marginY  = 2;  ///< 8dp
    static constexpr Unit rounding = 25; ///< 100dp
};

template <>
struct Button<SizeTips::SMALL>
{
    static constexpr auto textRole = TextRole::LabelLarge;
    static constexpr Unit iconSize = 5;  ///< 20dp
    static constexpr Unit height   = 10; ///< 40dp
    static constexpr Unit marginY  = 1;  ///< 4dp
    static constexpr Unit rounding = 25; ///< 100dp
};

template <>
struct Button<SizeTips::MEDIUM>
{
    static constexpr auto textRole = TextRole::TitleMedium;
    static constexpr Unit iconSize = 6;  ///< 24dp
    static constexpr Unit height   = 14; ///< 56dp
    static constexpr Unit margin   = 0;  ///< 0dp
    static constexpr Unit rounding = 25; ///< 100dp
};

template <>
struct Button<SizeTips::LARGE>
{
    static constexpr auto textRole = TextRole::HeadlineSmall;
    static constexpr Unit iconSize = 8;  ///< 32dp
    static constexpr Unit height   = 24; ///< 96dp
    static constexpr Unit margin   = 0;  ///< 0dp
    static constexpr Unit rounding = 25; ///< 100dp
};

template <>
struct Button<SizeTips::XLARGE>
{
    static constexpr auto textRole = TextRole::HeadlineLarge;
    static constexpr Unit iconSize = 10; ///< 40dp
    static constexpr Unit height   = 34; ///< 136dp
    static constexpr Unit margin   = 0;  ///< 0dp
    static constexpr Unit rounding = 25; ///< 100dp
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
    static constexpr Unit iconSize = 6;  ///< 24dp
    static constexpr Unit size     = 14; ///< 56dp
    static constexpr Unit rounding = 4;  ///< 16dp
};

template <>
struct FAB<SizeTips::MEDIUM>
{
    static constexpr Unit iconSize = 7;  ///< 28dp
    static constexpr Unit size     = 20; ///< 80dp
    static constexpr Unit rounding = 5;  ///< 20dp
};

template <>
struct FAB<SizeTips::LARGE>
{
    static constexpr Unit iconSize = 9;  ///< 36dp
    static constexpr Unit size     = 24; ///< 96dp
    static constexpr Unit rounding = 7;  ///< 28dp
};

} // namespace ImGuiEx::M3::Spec
