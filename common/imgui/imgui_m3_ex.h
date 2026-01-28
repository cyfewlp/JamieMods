//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "common/config.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"

#include <string_view>

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{

constexpr int CHANNEL_FG = 1;
constexpr int CHANNEL_BG = 0;

[[nodiscard]] inline auto IconButtonStyles(const Colors &colors, const ButtonStyle &buttonStyle) -> StyleGuard
{
    StyleGuard styleGuard;
    styleGuard
        .Push(ColorHolder::Text(colors.OnPrimary()))
        // .Push(ColorHolder::Button(colors.primary)) // default behavior
        // .Push(ColorHolder::ButtonActive(colors.primary_pressed))
        // .Push(ColorHolder::ButtonHovered(colors.primary_hovered))
        .Push(StyleHolder::FramePadding(buttonStyle.padding))
        .Push(StyleHolder::FrameRounding(buttonStyle.rounding));
    return styleGuard;
}

[[nodiscard]] inline auto FabButtonStyles(const Colors &colors, const ButtonStyle &buttonStyle) -> StyleGuard
{
    StyleGuard styleGuard;
    styleGuard.Push(ColorHolder::Text(colors.OnPrimaryContainer()))
        .Push(ColorHolder::Button(colors.PrimaryContainer())) // default behavior
        .Push(ColorHolder::ButtonActive(colors.PrimaryContainer().GetPressedState(colors.OnPrimaryContainer())))
        .Push(ColorHolder::ButtonHovered(colors.PrimaryContainer().GetHoveredState(colors.OnPrimaryContainer())))
        .Push(StyleHolder::FramePadding(buttonStyle.padding))
        .Push(StyleHolder::FrameRounding(buttonStyle.rounding));

    return styleGuard;
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
void DrawNavMenu(std::string_view icon);

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
/// BUTTON
////////////////////////////////////////////////////////////////////

auto DrawIconButton(
    std::string_view icon, const SurfaceColor &containerColor, const ContentColor &textColor, ImFont *iconFont,
    const ButtonSpec &spec
) -> bool;

auto DrawIconButton(std::string_view icon, const ButtonSpec &spec, const M3Styles &m3Styles) -> bool;

/**
 * The toolbar is a container with multiple slots, and you must
 * provide your expected dimensions and draw your button according to the agreement.
 * @return true is Toolbar visible
 */
auto BeginDockedToolbar(const ImVec2 &buttonSize, uint8_t count, ImU32 bgColor) -> bool;
auto BeginDockedToolbar(float buttonSize, uint8_t count, ImU32 bgColor) -> bool;

auto EndDockedToolbar() -> void;

////////////////////////////////////////////////////////////////////
/// LIST
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
/// Tooltip
////////////////////////////////////////////////////////////////////

void SetItemToolTip(std::string_view text, const Colors &colors);
}
}