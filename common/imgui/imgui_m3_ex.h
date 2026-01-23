//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "common/config.h"
#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"

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
    styleGuard
        .Push(ColorHolder::Text(colors.on_primary_container))
        .Push(ColorHolder::Button(colors.primary_container)) // default behavior
        .Push(ColorHolder::ButtonActive(colors.primary_container_pressed))
        .Push(ColorHolder::ButtonHovered(colors.primary_container_hovered))
        .Push(StyleHolder::FramePadding(buttonStyle.padding))
        .Push(StyleHolder::FrameRounding(buttonStyle.rounding));
    return styleGuard;
}
}
}