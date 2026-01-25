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
[[nodiscard]] inline auto ItemToolTipStyles(const Colors &colors) -> StyleGuard
{
    StyleGuard styleGuard;
    styleGuard.Push(ColorHolder::Text(colors.inverse_on_surface))
        .Push(ColorHolder::PopupBg(colors.inverse_surface))
        .Push(StyleHolder::WindowPadding(TOOLTIP_PADDING));
    return styleGuard;
}

[[nodiscard]] inline auto IconButtonStyles(const Colors &colors, const ButtonStyle &buttonStyle) -> StyleGuard
{
    StyleGuard styleGuard;
    styleGuard
        .Push(ColorHolder::Text(colors.on_primary))
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
    styleGuard.Push(ColorHolder::Text(colors.on_primary_container))
        .Push(ColorHolder::Button(colors.primary_container)) // default behavior
        .Push(ColorHolder::ButtonActive(colors.primary_container_pressed))
        .Push(ColorHolder::ButtonHovered(colors.primary_container_hovered))
        .Push(StyleHolder::FramePadding(buttonStyle.padding))
        .Push(StyleHolder::FrameRounding(buttonStyle.rounding));
    return styleGuard;
}

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

auto DrawFabButton(std::string_view icon, const M3Styles &m3Styles) -> bool;

////////////////////////////////////////////////////////////////////
/// LIST
////////////////////////////////////////////////////////////////////
}
}