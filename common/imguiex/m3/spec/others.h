//
// Created by jamie on 2026/2/10.
//

#pragma once

#include "base.h"
#include "typography.h"

namespace ImGuiEx::M3::Spec
{
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
    static constexpr auto textRole         = TextRole::BodyLarge;
    static constexpr auto iconSize         = TypeScale<TextRole::BodyLarge>::lineHeight;
    static constexpr Unit leadingImageSize = dp<56>();
    static constexpr Unit paddingY         = dp<10>();
    static constexpr Unit paddingX         = dp<16>();
    static constexpr Unit minHeight        = dp<52>();
    static constexpr Unit width            = dp<280>();
    static constexpr Unit gap              = dp<12>();
    static constexpr Unit segmentedGap     = dp<2>(); ///< md.comp.list.segmented.gap
    //! extended field
    static constexpr Unit minContentHeight = minHeight - paddingY * 2U;
};

struct Tooltips
{
    static constexpr auto textRole = TextRole::BodySmall;
    static constexpr Unit height   = dp<24>();
    static constexpr Unit paddingX = dp<8>();
};

//! https://m3.material.io/components/dialogs/specs
struct Dialogs
{
    static constexpr auto  textRole = TextRole::BodySmall;
    static constexpr Unit  paddingX = dp<24>();
    static constexpr Unit  paddingY = dp<24>();
    static constexpr Unit  minWidth = dp<280>();
    static constexpr float maxWidth = 560.0F;
};

} // namespace ImGuiEx::M3::Spec
