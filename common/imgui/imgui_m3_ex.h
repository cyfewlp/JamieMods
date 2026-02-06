//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "common/imgui/Material3.h"

#include <string_view>

namespace ImGuiEx::M3
{
constexpr int  CHANNEL_FG = 1;
constexpr int  CHANNEL_BG = 0;
constexpr auto HALF       = 0.5f;

inline auto HalfLineGap(const Text &text) -> float
{
    return (text.lineHeight - text.fontSize) * HALF;
}

inline auto TextStart(const std::string_view text) -> const char *
{
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return text.data();
}

inline auto TextEnd(const std::string_view text) -> const char *
{
    return text.data() + text.size(); // NOLINT(*-pro-bounds-pointer-arithmetic)
}

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
void LineTextUnformatted(const std::string_view &text, float lineHeight = 0.0f);

/**
 * Currently, the menu button on the navigation rail is meaningless; this method only handles padding, layout, and
 * simply renders the icon.
 * @param icon Due to the different icon fonts used ultimately, you need to provide the icon string for "menu" by
 * yourself.
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

////////////////////////////////////////////////////////////////////
/// ICON

auto DrawIcon(
    std::string_view icon, const M3Styles &m3Styles, ContentToken contentToken, SizeTips sizeTips = SizeTips::SMALL
) -> void;

////////////////////////////////////////////////////////////////////
/// BUTTON

auto DrawIconButton(
    std::string_view icon, const M3Styles &m3Styles, SurfaceToken surfaceColorToken, ContentToken contentColorToken,
    SizeTips sizeTips = SizeTips::SMALL
) -> bool;

inline auto DrawIconButtonSurfaceContainerVariant(
    std::string_view icon, const M3Styles &m3Styles, SizeTips sizeTips = SizeTips::SMALL
)
{
    return DrawIconButton(icon, m3Styles, SurfaceToken::surfaceContainer, ContentToken::onSurfaceVariant, sizeTips);
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

////////////////////////////////////////////////////////////////////
/// LIST
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
/// Tooltip
////////////////////////////////////////////////////////////////////

void SetItemToolTip(std::string_view text, const M3Styles &m3Styles);

/// Popup

auto BeginModalPopup(std::string_view title, const M3Styles &m3Styles) -> bool;

auto EndPopup(std::string_view title, const M3Styles &m3Styles) -> bool;

} // namespace ImGuiEx::M3
