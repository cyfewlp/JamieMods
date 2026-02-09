//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "base.h"
#include "text_role.h"

namespace ImGuiEx::M3::Spec
{
struct Search
{
    static constexpr Unit paddingY = 4; ///< 16dp
    static constexpr Unit paddingX = 4; ///< 16dp
    static constexpr Unit rounding = 7; ///< 28dp
    static constexpr Unit gap      = 4; ///< 16dp
};

struct List
{
    static constexpr Unit paddingY = 4;  ///< 16dp
    static constexpr Unit paddingX = 4;  ///< 16dp
    static constexpr Unit height   = 13; ///< 52dp
    //! This size will be computed on every call times because it is greater than 32, but it is not a problem because it
    //! is only used in some specific components, such as the navigation rail.
    static constexpr Unit width = 70; ///< 280dp.
};

/**
 * @note **Toolbar is designed for SMALL icon buttons.** Internal spacing (padding/gap)
 * excludes additional button margins. **To ensure consistent styling**, please call
 * `M3::IconButton<SizeTips::SMALL>`.
 */
struct ToolBar
{
    static constexpr Unit rounding = 8; ///< 32dp
    static constexpr Unit paddingX = 2; ///< 8dp
    static constexpr Unit paddingY = 2; ///< 8dp
    static constexpr Unit gap      = 1; ///< 4dp
};

struct Menu
{
    static constexpr auto textRole = TextRole::LabelLarge;
    //! The expected width of the menu.
    //!
    //! This size can be used to set the ChildWindow minimum width when implementing the menu, but it is not mandatory.
    //! You can also choose to let the menu width adapt to the content, but it is recommended to ensure that the minimum
    //! width of the menu is not less than this value for better visual consistency.
    static constexpr Unit width        = 52; ///< 208dp.
    static constexpr Unit paddingX     = 1;  ///< 4dp
    static constexpr Unit paddingY     = 1;  ///< 4dp
    static constexpr Unit gapY         = 1;  ///< 4dp
    static constexpr Unit itemHeight   = 11; ///< 44dp
    static constexpr Unit itemPaddingX = 3;  ///< 12dp. item inner padding.
    static constexpr Unit itemPaddingY = 3;  ///< 12dp. item inner padding.
};

struct NavRail
{
    static constexpr Unit width = 24; ///< 96dp. collapsed
};

} // namespace ImGuiEx::M3::Spec
