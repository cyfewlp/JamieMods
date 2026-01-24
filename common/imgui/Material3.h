//
// Created by jamie on 2026/1/18.
//

#pragma once

#include "common/config.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
/// https://m3.material.io/
namespace ImGuiEx::M3
{
static constexpr auto GRID_UNIT                    = 4.f;
static constexpr auto STANDARD_FONT_SIZE           = 20.f;
static constexpr auto CUSTOM_WINDOW_PADDING1       = ImVec2{32.f, 32.f};
static constexpr auto CUSTOM_WINDOW_PADDING2       = ImVec2{16.f, 16.f};
static constexpr auto CUSTOM_WINDOW_PADDING4       = ImVec2{8.f, 8.f};
static constexpr auto CUSTOM_THICK_SCROLL_BAR_SIZE = 8.f;

struct ButtonStyle
{
    float  fontSize;
    ImVec2 padding;
    ImVec2 spacing;
    float  rounding;
};

static constexpr auto XSMALL_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{12.f, 6.f },
         .spacing = ImVec2{18.f, 16.f},
         .rounding = 12.f
};

static constexpr auto SMALL_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{16.f, 10.f},
         .spacing = ImVec2{12.f, 8.f },
         .rounding = 12.f
};

static constexpr auto MEDIUM_BUTTON = ButtonStyle{
    .fontSize = 24.f, .padding = ImVec2{24.f, 24.f},
         .spacing = ImVec2{8.f,  8.f },
         .rounding = 16.f
};

static constexpr auto LARGE_BUTTON = ButtonStyle{
    .fontSize = 32.f, .padding = ImVec2{48.f, 32.f},
         .spacing = ImVec2{12.f, 8.f },
         .rounding = 28.f
};

static constexpr auto XSMALL_ICON_BUTTON = ButtonStyle{
    .fontSize = 20.f, .padding = ImVec2{6.f,  6.f },
         .spacing = ImVec2{16.f, 16.f},
         .rounding = 8.f
};

static constexpr auto SMALL_ICON_BUTTON = ButtonStyle{
    .fontSize = 24.f, .padding = ImVec2{8.f,  8.f },
         .spacing = ImVec2{12.f, 12.f},
         .rounding = 8.f
};

static constexpr auto MEDIUM_ICON_BUTTON = ButtonStyle{
    .fontSize = 24.f, .padding = ImVec2{16.f, 16.f},
         .spacing = ImVec2{12.f, 12.f},
         .rounding = 12.f
};

struct ListStyle
{
    float  fontSize;
    float  supportFontSize;
    ImVec2 padding;
};

static constexpr auto LIST_2DENSITY = ListStyle{
    .fontSize        = 24.f,
    .supportFontSize = 20.f,
    .padding         = ImVec2{8.f, 12.f},
};

static constexpr auto LIST_4DENSITY = ListStyle{
    .fontSize        = 24.f,
    .supportFontSize = 20.f,
    .padding         = ImVec2{8.f, 8.f},
};

struct ToolBarStyle
{
    float  fontSize;
    ImVec2 padding;
    float  rounding;
};

static constexpr auto TOOL_BAR_STANDARD = ToolBarStyle{
    .fontSize = 20.f, .padding = ImVec2{8.f, 8.f},
         .rounding = 24.f
};

struct SearchStyle
{
    float  fontSize;
    ImVec2 padding;
    float  rounding;
};

static constexpr auto SEARCH_SMALL    = SearchStyle{.fontSize = 20.f, .padding = ImVec2(8.f, 8.f), .rounding = 28.f};
static constexpr auto SEARCH_STANDARD = SearchStyle{.fontSize = 24.f, .padding = ImVec2(16.f, 16.f), .rounding = 28.f};

constexpr ImVec2 TOOLTIP_PADDING = {8.f, 4.f};

struct NavigationRailSpec
{
    float  padding;
    float  iconSize;
    float  fontSize;
    ImVec2 spacing;
    float  width;
};

struct Colors
{
    ImColor primary;
    ImColor surface_tint;
    ImColor on_primary;
    ImColor primary_container;
    ImColor on_primary_container;
    ImColor secondary;
    ImColor on_secondary;
    ImColor secondary_container;
    ImColor on_secondary_container;
    ImColor tertiary;
    ImColor on_tertiary;
    ImColor tertiary_container;
    ImColor on_tertiary_container;
    ImColor error;
    ImColor on_error;
    ImColor error_container;
    ImColor on_error_container;
    ImColor background;
    ImColor on_background;
    ImColor surface;
    ImColor on_surface;
    ImColor surface_variant;
    ImColor on_surface_variant;
    ImColor outline;
    ImColor outline_variant;
    ImColor shadow;
    ImColor scrim;
    ImColor inverse_surface;
    ImColor inverse_on_surface;
    ImColor inverse_primary;
    ImColor primary_fixed;
    ImColor on_primary_fixed;
    ImColor primary_fixed_dim;
    ImColor on_primary_fixed_variant;
    ImColor secondary_fixed;
    ImColor on_secondary_fixed;
    ImColor secondary_fixed_dim;
    ImColor on_secondary_fixed_variant;
    ImColor tertiary_fixed;
    ImColor on_tertiary_fixed;
    ImColor tertiary_fixed_dim;
    ImColor on_tertiary_fixed_variant;
    ImColor surface_dim;
    ImColor surface_bright;
    ImColor surface_container_lowest;
    ImColor surface_container_low;
    ImColor surface_container;
    ImColor surface_container_high;
    ImColor surface_container_highest;

    ///////////////////////////////////
    /// Extension fields

    ImColor primary_container_hovered;
    ImColor primary_container_pressed;

    static ImVec4 GetStateColor(ImVec4 source, ImVec4 tint, float state_alpha)
    {
        return ImVec4(
            source.x + (tint.x - source.x) * state_alpha,
            source.y + (tint.y - source.y) * state_alpha,
            source.z + (tint.z - source.z) * state_alpha,
            source.w
        );
    }

    void calculateExtensionFields()
    {
        primary_container_hovered = GetStateColor(primary_container, on_primary_container, 0.08f);
        primary_container_pressed = GetStateColor(primary_container, on_primary_container, 0.12f);
    }
};

struct M3Styles
{
    Colors colors;
    /**
     * It is highly recommended to provide a standalone ImFont pointer that has NOT been merged with other fonts.
     * Merging icon fonts often causes "Ascent" and "Descent" metrics to be re-calculated or corrupted to fit the
     * primary font's baseline, leading to vertical misalignment and "bleeding" outside the glyph bounding box. For
     * pixel-perfect grid alignment, use an independently loaded font.
     */
    ImFont *iconFont;

    constexpr M3Styles() = default;
};

namespace NavigationRail
{
static constexpr auto Standard =
    NavigationRailSpec{.padding = 16.f, .iconSize = 24.f, .fontSize = 16.f, .spacing = ImVec2(2.f, 4.f), .width = 96.f};
}

}
}