//
// Created by jamie on 2026/2/13.
//

#pragma once

#include "base.h"

namespace ImGuiEx::M3::Spec
{
struct Layout
{

    struct Compact
    {
        static constexpr float Breakpoint = 0.0F;
        static constexpr Unit  Margin     = dp<16>();
    };

    /**
     * @brief Layouts for medium window size classes are for screen widths from 600dp to 839dp.
     * A single pane layout is recommended because of limited screen width.
     * However, a two-pane layout is possible for content with lower information density, such as a settings screen.
     *
     * Each pane in a two-pane layouts should take up 50% of the window width. Avoid setting custom widths.
     * A drag handle can be used to expand or collapse panes to be 100% of the window width.
     */
    struct Medium
    {
        static constexpr float Breakpoint = 600.0F;
        static constexpr float MaxWidth   = 839.0F;
        static constexpr Unit  Margin     = dp<24>();
    };

    /**
     * @brief Layouts for expanded window size classes are for **screen widths 840dp to 1199dp.**
     *
     * Use a single pane layout or two-pane layout.
     * A two-pane layout is often best for expanded window classes. However, a single pane layout can work when
     * displaying visually dense or information-dense content, such as videos.
     */
    struct Expanded
    {
        static constexpr float Breakpoint     = 840.0F;
        static constexpr float MaxWidth       = 1199.0F;
        static constexpr Unit  Margin         = dp<24>();
        static constexpr auto  FixedPaneWidth = 360.0F;
    };

    /**
     * @brief Layouts for large window size classes are for screen widths from 1200dp to 1599dp.
     */
    struct Large
    {
        static constexpr float Breakpoint     = 1200.0F;
        static constexpr float MaxWidth       = 1599.0F;
        static constexpr Unit  Margin         = dp<24>();
        static constexpr auto  FixedPaneWidth = 412.0F;
    };

    /**
     * @brief Layouts for extra-large window size classes are for screen widths of 1600dp and larger.
     *
     * The extra-large window size class supports using a standard side sheet as a third pane. When
     * the side sheet is present, the navigation drawer can remain visible, collapse into a navigation
     * rail, or hide completely. Don't use more than three panes.
     */
    struct ExtraLarge
    {
        static constexpr float Breakpoint         = 1600.0F;
        static constexpr Unit  Margin             = dp<24>();
        static constexpr auto  FixedPaneWidth     = 412.0F;
        static constexpr auto  SideSheetsMaxWidth = 400.0F;
    };
};
} // namespace ImGuiEx::M3::Spec
