//
// Created by jamie on 2026/1/18.
//

#pragma once

#include "common/config.h"

#include <imgui.h>

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

struct TextSizeSpec
{
    float fontSize;
    float lineHeight;

    consteval auto GetTextSpacing() const -> float
    {
        return lineHeight - fontSize;
    }
};

namespace TextSize
{
static constexpr auto SMALL  = TextSizeSpec{.fontSize = 12.f, .lineHeight = 16.f};
static constexpr auto MEDIUM = TextSizeSpec{.fontSize = 14.f, .lineHeight = 20.f};
static constexpr auto LARGE  = TextSizeSpec{.fontSize = 16.f, .lineHeight = 24.f};
}

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

class ThemeBuilder;

class Colors
{
    ImColor primary;
    ImColor surfaceTint;
    ImColor onPrimary;
    ImColor primaryContainer;
    ImColor onPrimaryContainer;
    ImColor secondary;
    ImColor onSecondary;
    ImColor secondaryContainer;
    ImColor onSecondaryContainer;
    ImColor tertiary;
    ImColor onTertiary;
    ImColor tertiaryContainer;
    ImColor onTertiaryContainer;
    ImColor error;
    ImColor onError;
    ImColor errorContainer;
    ImColor onErrorContainer;
    ImColor background;
    ImColor onBackground;
    ImColor surface;
    ImColor onSurface;
    ImColor surfaceVariant;
    ImColor onSurfaceVariant;
    ImColor outline;
    ImColor outlineVariant;
    ImColor shadow;
    ImColor scrim;
    ImColor inverseSurface;
    ImColor inverseOnSurface;
    ImColor inversePrimary;
    ImColor primaryFixed;
    ImColor onPrimaryFixed;
    ImColor primaryFixedDim;
    ImColor onPrimaryFixedVariant;
    ImColor secondaryFixed;
    ImColor onSecondaryFixed;
    ImColor secondaryFixedDim;
    ImColor onSecondaryFixedVariant;
    ImColor tertiaryFixed;
    ImColor onTertiaryFixed;
    ImColor tertiaryFixedDim;
    ImColor onTertiaryFixedVariant;
    ImColor surfaceDim;
    ImColor surfaceBright;
    ImColor surfaceContainerLowest;
    ImColor surfaceContainerLow;
    ImColor surfaceContainer;
    ImColor surfaceContainerHigh;
    ImColor surfaceContainerHighest;

    static constexpr ImColor GetStateColor(const ImColor &source, const ImColor &tint, const float state_alpha)
    {
        return ImColor(
            source.Value.x + (tint.Value.x - source.Value.x) * state_alpha,
            source.Value.y + (tint.Value.y - source.Value.y) * state_alpha,
            source.Value.z + (tint.Value.z - source.Value.z) * state_alpha,
            source.Value.w
        );
    }

    uint32_t seedArgb;
    uint32_t primaryPalette;
    uint32_t secondaryPalette;
    uint32_t tertiaryPalette;
    uint32_t neutralPalette;
    uint32_t neutralVariantPalette;
    uint32_t errorPalette;

    friend class ThemeBuilder;

public:
    static constexpr ImColor GetHoveredColor(const ImColor &source, const ImColor &tint)
    {
        return GetStateColor(source, tint, 0.08f);
    }

    static constexpr ImColor GetActiveColor(const ImColor &source, const ImColor &tint)
    {
        return GetStateColor(source, tint, 0.12f);
    }

    // clang-format off
    auto SeedArgb() const -> uint32_t { return seedArgb; }
    auto PrimaryPalette() const -> uint32_t {return primaryPalette;}
    auto SecondaryPalette() const -> uint32_t {return secondaryPalette;}
    auto TertiaryPalette() const -> uint32_t {return tertiaryPalette;}
    auto NeutralPalette() const -> uint32_t {return neutralPalette;}
    auto NeutralVariantPalette() const -> uint32_t {return neutralVariantPalette;}
    auto ErrorPalette() const -> uint32_t {return errorPalette;}

    auto Primary() const -> const ImColor & { return primary; }
    auto SurfaceTint() const -> const ImColor & { return surfaceTint; }
    auto OnPrimary() const -> const ImColor & { return onPrimary; }
    auto PrimaryContainer() const -> const ImColor & { return primaryContainer; }
    auto OnPrimaryContainer() const -> const ImColor & { return onPrimaryContainer; }
    auto Secondary() const -> const ImColor & { return secondary; }
    auto OnSecondary() const -> const ImColor & { return onSecondary; }
    auto SecondaryContainer() const -> const ImColor & { return secondaryContainer; }
    auto OnSecondaryContainer() const -> const ImColor & { return onSecondaryContainer; }
    auto Tertiary() const -> const ImColor & { return tertiary; }
    auto OnTertiary() const -> const ImColor & { return onTertiary; }
    auto TertiaryContainer() const -> const ImColor & { return tertiaryContainer; }
    auto OnTertiaryContainer() const -> const ImColor & { return onTertiaryContainer; }
    auto Error() const -> const ImColor & { return error; }
    auto OnError() const -> const ImColor & { return onError; }
    auto ErrorContainer() const -> const ImColor & { return errorContainer; }
    auto OnErrorContainer() const -> const ImColor & { return onErrorContainer; }
    auto Background() const -> const ImColor & { return background; }
    auto OnBackground() const -> const ImColor & { return onBackground; }
    auto Surface() const -> const ImColor & { return surface; }
    auto OnSurface() const -> const ImColor & { return onSurface; }
    auto SurfaceVariant() const -> const ImColor & { return surfaceVariant; }
    auto OnSurfaceVariant() const -> const ImColor & { return onSurfaceVariant; }
    auto Outline() const -> const ImColor & { return outline; }
    auto OutlineVariant() const -> const ImColor & { return outlineVariant; }
    auto Shadow() const -> const ImColor & { return shadow; }
    auto Scrim() const -> const ImColor & { return scrim; }
    auto InverseSurface() const -> const ImColor & { return inverseSurface; }
    auto InverseOnSurface() const -> const ImColor & { return inverseOnSurface; }
    auto InversePrimary() const -> const ImColor & { return inversePrimary; }
    auto PrimaryFixed() const -> const ImColor & { return primaryFixed; }
    auto OnPrimaryFixed() const -> const ImColor & { return onPrimaryFixed; }
    auto PrimaryFixedDim() const -> const ImColor & { return primaryFixedDim; }
    auto OnPrimaryFixedVariant() const -> const ImColor & { return onPrimaryFixedVariant; }
    auto SecondaryFixed() const -> const ImColor & { return secondaryFixed; }
    auto OnSecondaryFixed() const -> const ImColor & { return onSecondaryFixed; }
    auto SecondaryFixedDim() const -> const ImColor & { return secondaryFixedDim; }
    auto OnSecondaryFixedVariant() const -> const ImColor & { return onSecondaryFixedVariant; }
    auto TertiaryFixed() const -> const ImColor & { return tertiaryFixed; }
    auto OnTertiaryFixed() const -> const ImColor & { return onTertiaryFixed; }
    auto TertiaryFixedDim() const -> const ImColor & { return tertiaryFixedDim; }
    auto OnTertiaryFixedVariant() const -> const ImColor & { return onTertiaryFixedVariant; }
    auto SurfaceDim() const -> const ImColor & { return surfaceDim; }
    auto SurfaceBright() const -> const ImColor & { return surfaceBright; }
    auto SurfaceContainerLowest() const -> const ImColor & { return surfaceContainerLowest; }
    auto SurfaceContainerLow() const -> const ImColor & { return surfaceContainerLow; }
    auto SurfaceContainer() const -> const ImColor & { return surfaceContainer; }
    auto SurfaceContainerHigh() const -> const ImColor & { return surfaceContainerHigh; }
    auto SurfaceContainerHighest() const -> const ImColor & { return surfaceContainerHighest; }

    // clang-format on

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

struct ListStyle
{
    TextSizeSpec text;
    TextSizeSpec supportText;
    ImVec2       padding;
    float        gap;
};

struct ButtonSpec
{
    float  fontSize;
    ImVec2 padding;
    ImVec2 spacing;
    float  size; // < 0 invalid
    float  rounding;
};

namespace FAB
{
static constexpr auto STANDARD =
    ButtonSpec{.fontSize = 24.f, .padding = ImVec2(16.f, 16.f), .spacing = ImVec2(), .size = 56.f, .rounding = 16.f};
}

namespace IconButton
{
static constexpr auto XSMALL = ButtonSpec{
    .fontSize = 20.f, .padding = ImVec2(6.f, 6.f), .spacing = ImVec2(8.f, 8.f), .size = 48.f, .rounding = 8.f
};
static constexpr auto SMALL = ButtonSpec{
    .fontSize = 24.f, .padding = ImVec2(8.f, 8.f), .spacing = ImVec2(4.f, 4.f), .size = 48.f, .rounding = 8.f
};
}

namespace List
{
static constexpr auto STANDARD = ListStyle{
    .text = TextSize::LARGE, .supportText = TextSize::MEDIUM, .padding = ImVec2{16.f, 10.f},
            .gap = 12.f
};
}

struct ToolbarSpec
{
    float  gap;
    ImVec2 padding;
    float  rounding;
    ImVec2 margin;
};

namespace Toolbar
{
static constexpr auto FLOAT = ToolbarSpec{
    .gap = 4.f, .padding = ImVec2{8.f, 8.f},
         .rounding = 32.f, .margin = ImVec2(16.f, 24.f)
};
static constexpr auto DOCKED = ToolbarSpec{
    .gap = -1.f, .padding = ImVec2{16.f, 8.f},
         .rounding = 0.f, .margin = ImVec2(16.f, 24.f)
};
}

struct TooltipSpec
{
    TextSizeSpec textSize;
    ImVec2       padding;

    consteval auto GetFullPadding() const -> ImVec2
    {
        return {padding.x, padding.y + textSize.GetTextSpacing()};
    }
};

namespace Tooltip
{
static constexpr auto PLAIN = TooltipSpec{.textSize = TextSize::SMALL, .padding = ImVec2(8.f, 4.f)};
}

}
}