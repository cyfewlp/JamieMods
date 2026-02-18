//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "ImGuiEx.h"
#include "Material3.h"
#include "imguiex_enum_wrap.h"
#include "m3/spec/specs.h"
#include "m3/spec/text_field.h"

#include <functional>
#include <string_view>

namespace ImGuiEx::M3
{
constexpr int CHANNEL_FG = 1;
constexpr int CHANNEL_BG = 0;

//! Support line height with last used TextRole's line height. If the line height is not set or less than font size, it
//! will fall back to normal text rendering.
void TextUnformatted(
    const std::string_view &text, const M3Styles &m3Styles, Spec::ColorRole contentRole = Spec::ColorRole::onSurface
);

/**
 * @brief Renders a Material Design 3 style Navigation Rail item.
 * * Draws an interactive element comprising an active indicator (background), an icon,  and a label.
 *
 * @note **Layout Constraint:** By design, this component is "width-unaware" and
 * fills all available X-space. This is a **strict requirement** to force usage
 * inside a `ChildWindow` (like a Navigation Rail or Drawer), preventing
 * non-compliant full-screen-width navigation items.
 *
 * @param label     The text label displayed below or beside the icon.
 * @param selected  Whether the item is in the active state, affecting colors and indicator visibility.
 * @param icon      The icon glyph (typically a UTF-8 string literal from an icon font).
 * @param m3Styles  Reference to @c ImGuiEx::M3Styles for theme and scaling.
 * @return true if the item was clicked; otherwise false.
 */
auto NavItem(std::string_view label, bool selected, std::string_view icon, const M3Styles &m3Styles) -> bool;

auto BeginNavRail(
    std::string_view strId, const M3Styles &m3Styles, ImGuiEx::ChildFlags childFlags = {},
    Spec::ColorRole containerColor = Spec::ColorRole::surfaceContainer
) -> bool;

//! Like ImGui::EndChild: You always need to call it after BeginNavRail.
auto EndNavRail() -> void;

namespace detail
{
struct IconLayout
{
    float iconSize;
    float size;
    float margin;
    float rounding;
};

auto Icon(std::string_view icon, const M3Styles &m3Styles, const IconLayout &layout, Spec::ColorRole contentRole)
    -> void;
} // namespace detail

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto Icon(std::string_view icon, const M3Styles &m3Styles, Spec::ColorRole contentRole) -> void
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
        contentRole
    );
}

/////////////////////////////////////////////////
/// ... Buttons ...
/////////////////////////////////////////////////

namespace detail
{
auto IconButton(
    std::string_view icon, const M3Styles &m3Styles, const IconLayout &layout, Spec::ColorRole surfaceRole,
    Spec::ColorRole contentRole
) -> bool;

/**
 * @brief Resolves a raw width value into final layout pixels, supporting relative and absolute values.
 * * This internal helper converts unscaled units to pixels and handles ImGui-style
 *
 * sizing logic:
 * - Positive values: Treated as absolute width (scaled by current DPI/Theme scale).
 * - Negative values: Treated as relative to the current available content region.
 * - Zero: Replaces the input with the provided @p defaultWidth before resolution.
 *
 * @param width        Unscaled width (positive for absolute, negative for relative).
 * @param defaultWidth Width to use if @p width is 0.
 * @param m3Styles     Reference used to scale units to pixels.
 * @return float       The resolved width in pixels, clamped to available space if necessary.
 */
auto ResolveItemWidth(float width, float defaultWidth, const M3Styles &m3Styles) -> float;

} // namespace detail

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
inline auto IconButton(
    std::string_view icon, const M3Styles &m3Styles, Spec::ColorRole surfaceRole, Spec::ColorRole contentRole
) -> bool
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
        surfaceRole,
        contentRole
    );
}

inline auto IconButtonXS(
    const std::string_view icon, const M3Styles &m3Styles, const Spec::ColorRole surfaceRole,
    const Spec::ColorRole contentRole
) -> bool
{
    return IconButton<Spec::SizeTips::XSMALL>(icon, m3Styles, surfaceRole, contentRole);
}

inline auto IconButtonM(
    const std::string_view icon, const M3Styles &m3Styles, const Spec::ColorRole surfaceRole,
    const Spec::ColorRole contentRole
) -> bool
{
    return IconButton<Spec::SizeTips::MEDIUM>(icon, m3Styles, surfaceRole, contentRole);
}

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto IconButtonSurfaceContainerVariant(std::string_view icon, const M3Styles &m3Styles)
{
    return IconButton<Size>(icon, m3Styles, Spec::ColorRole::surfaceContainer, Spec::ColorRole::onSurfaceVariant);
}

template <Spec::SizeTips Size = Spec::SizeTips::SMALL>
auto FAB(
    std::string_view icon, const M3Styles &m3Styles, Spec::ColorRole surfaceRole = Spec::ColorRole::primary,
    Spec::ColorRole contentRole = Spec::ColorRole::onPrimary
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
        surfaceRole,
        contentRole
    );
}

auto TextField(
    std::string_view label, Spec::TextFieldStyle tfStyle, char *buffer, size_t bufferSize, const M3Styles &m3Styles,
    float width
) -> bool;

/**
 * @brief A filled text field that follows Material Design 3 specifications.
 *
 * Implemented by ImGui::TempInputText and custom Item style.
 * @return true if edited, false otherwise.
 */
inline auto FilledTextField(
    std::string_view label, char *buffer, size_t bufferSize, const M3Styles &m3Styles, float width = 0.0F
) -> bool
{
    return TextField(label, Spec::TextFieldStyle::Filled, buffer, bufferSize, m3Styles, width);
}

inline auto OutlinedTextField(
    std::string_view label, char *buffer, size_t bufferSize, const M3Styles &m3Styles, float width = 0.0F
) -> bool
{
    return TextField(label, Spec::TextFieldStyle::Outlined, buffer, bufferSize, m3Styles, width);
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
    std::string_view label, const M3Styles &m3Styles, Spec::ColorRole contentRole = Spec::ColorRole::onSurface
);

//! adjust cursor position.y for the leading color button in ListItem.
void ListLayoutLeadingColorButton(float height = 0.F);

struct ListScope
{
    bool       visible;
    StyleGuard styleGuard;

    explicit operator bool() const { return visible; }
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
auto BeginDockedToolbar(const ImVec2 &buttonSize, uint8_t count, Spec::ColorRole surfaceRole, const M3Styles &m3Styles)
    -> bool;

inline auto BeginDockedToolbar(float buttonSize, uint8_t count, Spec::ColorRole surfaceRole, const M3Styles &m3Styles)
    -> bool
{
    return BeginDockedToolbar(ImVec2{buttonSize, buttonSize}, count, surfaceRole, m3Styles);
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
    return ImGui::Selectable(TextStart(label), selected, flags, {0.0F, m3Styles.GetPixels(Spec::Menu::itemHeight)});
}

void SetItemToolTip(std::string_view text, const M3Styles &m3Styles);

} // namespace ImGuiEx::M3
