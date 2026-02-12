//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "base.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{
struct Search
{
    static constexpr Unit paddingY = dp<16>();
    static constexpr Unit paddingX = dp<16>();
    static constexpr Unit rounding = dp<28>();
    static constexpr Unit gap      = dp<16>();
};

//! [List Tokens & specs](https://m3.material.io/components/lists/specs#f2afa345-6c7d-4681-b011-54d3f1a01338)
//!
//! For ImGui implementation, you can either manually configure a selectable item or use the group wrapper.
//!
//! ## Manual `Selectable` Item
//! To match the M3 specification manually, ensure the following:
//! - **SpanAllColumns**: Set to `true`.
//! - **Vertical Padding**: Use `ItemSpacing.y`.
//! - **Horizontal Padding**: Controlled via `Indent()` / `Unindent()`.
//! - **Typography**: Apply `TextRole::BodyLarge` with `SelectableTextAlign(0.0f, 0.5f)`.
//!
//! ## Group Wrapper (Automated)
//! Wrap content between `M3::BeginListItem()` and `M3::EndListItem()`.
//! The wrapper automatically manages height and internal padding.
//!
//! @note Text will be vertically centered by calling `AlignTextToFramePadding()` prior to rendering.
struct List
{
    static constexpr auto textRole     = TextRole::BodyLarge;
    static constexpr auto iconSize     = TypeScale<TextRole::BodyLarge>::lineHeight;
    static constexpr Unit paddingY     = dp<12>();
    static constexpr Unit paddingX     = dp<16>();
    static constexpr Unit height       = dp<56>();
    static constexpr Unit width        = dp<280>();
    static constexpr Unit gap          = dp<12>();
    //! Already merged to paddingY: 10 -> 12.
    static constexpr Unit segmentedGap = dp<2>(); ///< md.comp.list.segmented.gap
};

/**
 * @note **Toolbar is designed for SMALL icon buttons.** Internal spacing (padding/gap)
 * excludes additional button margins. **To ensure consistent styling**, please call
 * `M3::IconButton<SizeTips::SMALL>`.
 */
struct ToolBar
{
    static constexpr Unit rounding = dp<32>();
    static constexpr Unit paddingX = dp<8>();
    static constexpr Unit paddingY = dp<8>();
    static constexpr Unit gap      = dp<4>();
};

struct Menu
{
    static constexpr auto textRole     = TextRole::LabelLarge;
    //! The expected width of the menu.
    //!
    //! This size can be used to set the ChildWindow minimum width when implementing the menu, but it is not mandatory.
    //! You can also choose to let the menu width adapt to the content, but it is recommended to ensure that the minimum
    //! width of the menu is not less than this value for better visual consistency.
    static constexpr Unit width        = dp<208>();
    static constexpr Unit paddingX     = dp<4>();
    static constexpr Unit paddingY     = dp<4>();
    static constexpr Unit gapY         = dp<4>();
    static constexpr Unit itemHeight   = dp<44>();
    static constexpr Unit itemPaddingX = dp<12>(); ///< item inner padding.
    static constexpr Unit itemPaddingY = dp<12>(); ///< item inner padding.
};

struct NavRail
{
    static constexpr Unit width = dp<96>(); ///< collapsed
};

} // namespace ImGuiEx::M3::Spec
