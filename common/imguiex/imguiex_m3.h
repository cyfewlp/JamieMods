//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "ImGuiEx.h"
#include "Material3.h"
#include "imguiex_enum_wrap.h"
#include "m3/spec/specs.h"

#include <functional>
#include <string_view>

namespace ImGuiEx::M3
{
constexpr int CHANNEL_FG = 1;
constexpr int CHANNEL_BG = 0;

/**
 * @brief Renders unformatted text with vertical alignment compensation.
 * * This helper calculates the vertical offset needed to position text within a
 * custom line height. It modifies the internal 'CurrLineTextBaseOffset' to
 * push the text baseline down.
 * * @note This uses ImGui internal APIs (imgui_internal.h).
 * @warning Do not rely on this for general-purpose text rendering.
 * * @param text The string view to render.
 * @param lineHeight The desired total height of the line.
 */
void LineTextUnformatted(const std::string_view &text, float lineHeight = 0.0F);

//! Support line height with last used TextRole's line height. If the line height is not set or less than font size, it
//! will fall back to normal text rendering.
void TextUnformatted(
    const std::string_view &text, const M3Styles &m3Styles, ContentToken contentToken = ContentToken::onSurface
);

/**
 * Currently, the menu button on the navigation rail is meaningless; this method only handles padding, layout, and
 * simply renders the icon.
 * @param icon Due to the different icon fonts used ultimately, you need to provide the icon string for "menu" by
 * yourself.
 * @param m3Styles
 */
void DrawNavMenu(std::string_view icon, const M3Styles &m3Styles);

/**
 * @brief Renders a custom navigation item in Material Design 3 style.
 * * This component draws an interactive navigation element consisting of an icon and a label.
 * It uses a multi-layered rendering approach (via ImDrawList channels) to ensure proper
 * background/foreground depth and precise alignment.
 * @param label    The text label associated with the navigation item.
 * @param selected    is this nav item selected? Used to determine the colors needed for drawing.
 * @param icon     The icon glyph to render (typically a UTF-8 string literal).
 * @param m3Styles see @c ImGuiEx::M3Styles
 * @return true if the item is clicked, false otherwise.
 */
auto DrawNavItem(std::string_view label, bool selected, std::string_view icon, const M3Styles &m3Styles) -> bool;

namespace detail
{
struct IconLayout
{
    float iconSize;
    float size;
    float margin;
    float rounding;
};

auto Icon(std::string_view icon, const M3Styles &m3Styles, const IconLayout &layout, ContentToken contentToken) -> void;
} // namespace detail

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto Icon(std::string_view icon, const M3Styles &m3Styles, ContentToken contentToken) -> void
{
    detail::Icon(
        icon,
        m3Styles,
        detail::IconLayout{
            .iconSize = m3Styles.GetPixels(Spec::IconButton<Size>::iconSize),
            .size     = m3Styles.GetPixels(Spec::IconButton<Size>::size),
            .margin   = m3Styles.GetPixels(Spec::IconButton<Size>::margin),
            .rounding = m3Styles.GetPixels(Spec::IconButton<Size>::rounding),
        },
        contentToken
    );
}

/////////////////////////////////////////////////
/// ... Buttons ...
/////////////////////////////////////////////////

namespace detail
{
auto IconButton(
    std::string_view icon, const M3Styles &m3Styles, const IconLayout &layout, SurfaceToken surfaceColorToken,
    ContentToken contentColorToken
) -> bool;
}

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto IconButton(std::string_view icon, const M3Styles &m3Styles, SurfaceToken surfaceToken, ContentToken contentToken)
    -> bool
{
    return detail::IconButton(
        icon,
        m3Styles,
        detail::IconLayout{
            .iconSize = m3Styles.GetPixels(Spec::IconButton<Size>::iconSize),
            .size     = m3Styles.GetPixels(Spec::IconButton<Size>::size),
            .margin   = m3Styles.GetPixels(Spec::IconButton<Size>::margin),
            .rounding = m3Styles.GetPixels(Spec::IconButton<Size>::rounding),
        },
        surfaceToken,
        contentToken
    );
}

inline auto IconButtonXS(
    const std::string_view icon, const M3Styles &m3Styles, const SurfaceToken surfaceToken,
    const ContentToken contentToken
) -> bool
{
    return IconButton<Spec::SizeTips::XSMALL>(icon, m3Styles, surfaceToken, contentToken);
}

inline auto IconButtonM(
    const std::string_view icon, const M3Styles &m3Styles, const SurfaceToken surfaceToken,
    const ContentToken contentToken
) -> bool
{
    return IconButton<Spec::SizeTips::MEDIUM>(icon, m3Styles, surfaceToken, contentToken);
}

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto IconButtonSurfaceContainerVariant(std::string_view icon, const M3Styles &m3Styles)
{
    return IconButton<Size>(icon, m3Styles, SurfaceToken::surfaceContainer, ContentToken::onSurfaceVariant);
}

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto FAB(
    std::string_view icon, const M3Styles &m3Styles, SurfaceToken surfaceToken = SurfaceToken::primary,
    ContentToken contentToken = ContentToken::onPrimary
) -> bool
{
    return detail::IconButton(
        icon,
        m3Styles,
        detail::IconLayout{
            .iconSize = m3Styles.GetPixels(Spec::FAB<Size>::iconSize),
            .size     = m3Styles.GetPixels(Spec::FAB<Size>::size),
            .margin   = 0.0F,
            .rounding = m3Styles.GetPixels(Spec::FAB<Size>::rounding),
        },
        surfaceToken,
        contentToken
    );
}

using Func = std::function<void()>;

/**
 * @brief Renders a Material Design 3 list item with a scoped layout and custom content.
 *
 * Combines background rendering, interaction handling, and content scoping into a single call.
 * It automatically manages state layers (Hover/Pressed) based on M3 specifications.
 *
 * @param strId Unique identifier for ImGui state management.
 * @param m3Styles Theme configuration and layout tokens.
 * @param contentFunc Lambda/Callable for inner content. Note: Content is vertically centered
 * based on m3Styles.GetPadding<Spec::List>().
 * @param plain set true to ignore hovered staus.
 *
 * @note
 * - **Clipping**: Content is strictly clipped to the M3-defined height.
 * - **Layout**: Internally uses `BeginGroup` and `SetCursorScreenPos` to reset the local
 * coordinate system, ensuring `contentFunc` starts at the correctly padded origin.
 * - **Interaction**: The entire rectangular area (including padding) acts as a single
 * ButtonBehavior hotlink.
 *
 * @see M3Styles for available styling options.
 * @see Spec namespace for Material Design 3 specifications.
 */
void ListItem(std::string_view strId, const M3Styles &m3Styles, Func &&contentFunc, bool plain = false);

inline void ListItemPlain(std::string_view strId, const M3Styles &m3Styles, Func &&contentFunc)
{
    ListItem(strId, m3Styles, std::forward<Func>(contentFunc), true);
}

//! Render a single-line label aligned to the current line’s text baseline.
//! Works inside ListItem content and also for any line where you want centered text alignment.
//! Requires calling `m3Styles.UseTextRole<Spec::List::textRole>()` (or an equivalent role) beforehand.
void AlignedLabel(
    std::string_view label, const M3Styles &m3Styles, ContentToken contentToken = ContentToken::onSurface
);

//! adjust cursor position.y for the leading color button in ListItem.
void ListLayoutLeadingColorButton(float height = 0.F);

struct ListScope
{
    bool       visible;
    StyleGuard styleGuard;

    operator bool() const { return visible; }
};

[[nodiscard]] auto BeginList(const M3Styles &m3Styles, float width = 0.F, ChildFlags childFlags = {}) -> ListScope;

inline auto EndList()
{
    ImGui::EndChild();
}

inline void ListDivider()
{
    ImGui::Separator();
}

inline auto ListLeadingImageSize(const M3Styles &m3Styles) -> ImVec2
{
    const auto size = m3Styles.GetPixels(Spec::List::leadingImageSize);
    return {size, size};
}

/**
 * The toolbar is a container with multiple slots, and you must
 * provide your expected dimensions and draw your button according to the agreement.
 * @return true is Toolbar visible
 */
auto BeginDockedToolbar(const ImVec2 &buttonSize, uint8_t count, SurfaceToken surfaceToken, const M3Styles &m3Styles)
    -> bool;

inline auto BeginDockedToolbar(float buttonSize, uint8_t count, SurfaceToken surfaceToken, const M3Styles &m3Styles)
    -> bool
{
    return BeginDockedToolbar(ImVec2{buttonSize, buttonSize}, count, surfaceToken, m3Styles);
}

auto EndDockedToolbar() -> void;

inline void BeginMenu(const M3Styles &m3Styles)
{
    ImGui::Indent(m3Styles.GetPixels(Spec::Menu::itemPaddingX));
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, m3Styles.GetPixels(Spec::Menu::gapY));
}

inline void EndMenu(const M3Styles &m3Styles)
{
    ImGui::PopStyleVar();
    ImGui::Unindent(m3Styles.GetPixels(Spec::Menu::itemPaddingX));
}

inline auto MenuItem(
    const std::string_view label, bool selected, const M3Styles &m3Styles, const SelectableFlags flags = {}
) -> bool
{
    return ImGui::Selectable(TextStart(label), selected, flags, {0, m3Styles.GetPixels(Spec::Menu::itemHeight)});
}

void SetItemToolTip(std::string_view text, const M3Styles &m3Styles);

/// Popup

auto BeginModalPopup(std::string_view title, const M3Styles &m3Styles) -> bool;

auto EndPopup(std::string_view title, const M3Styles &m3Styles) -> bool;

// \todo implement M3 style TextField iteme
// auto TextField();

} // namespace ImGuiEx::M3
