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
static constexpr auto CUSTOM_STANDARD_MENU_WIDTH   = 208.f;
static constexpr auto CUSTOM_STANDARD_MENU_PADDING = 12.f;

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

struct ColorBase
{
    ImVec4 raw{0, 0, 0, 0};

    constexpr ColorBase() = default;

    constexpr ColorBase(float r, float g, float b, float a = 1.0f) : raw(r, g, b, a) {}

    constexpr ColorBase(const ImVec4 &col) : raw(col) {}

    constexpr ColorBase(int r, int g, int b, int a = 255) : raw(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f) {}

    constexpr ColorBase(ImU32 rgba)
        : raw(((rgba >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f, ((rgba >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
              ((rgba >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f, ((rgba >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f)
    {
    }

    constexpr operator ImVec4() const
    {
        return raw;
    }

    constexpr operator ImU32() const
    {
        return ((ImU32)(raw.x * 255.0f + 0.5f) << IM_COL32_R_SHIFT) |
               ((ImU32)(raw.y * 255.0f + 0.5f) << IM_COL32_G_SHIFT) |
               ((ImU32)(raw.z * 255.0f + 0.5f) << IM_COL32_B_SHIFT) |
               ((ImU32)(raw.w * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
    }
};

class ContentColor : public ColorBase
{
    using ColorBase::ColorBase;
};

class SurfaceColor : public ColorBase
{
public:
    using ColorBase::ColorBase;

    static constexpr float HOVER_OPACITY   = 0.08f;
    static constexpr float PRESSED_OPACITY = 0.12f;

    constexpr SurfaceColor GetState(const ContentColor &onColor, const float stateOpacity) const
    {
        return SurfaceColor(
            raw.x + (onColor.raw.x - raw.x) * stateOpacity,
            raw.y + (onColor.raw.y - raw.y) * stateOpacity,
            raw.z + (onColor.raw.z - raw.z) * stateOpacity,
            raw.w
        );
    }

    constexpr SurfaceColor GetHoveredState(const ContentColor &onColor) const
    {
        return GetState(onColor, HOVER_OPACITY);
    }

    constexpr SurfaceColor GetPressedState(const ContentColor &onColor) const
    {
        return GetState(onColor, PRESSED_OPACITY);
    }
};

class ThemeBuilder;

class Colors
{
    SurfaceColor primary;
    SurfaceColor surfaceTint;
    ContentColor onPrimary;
    SurfaceColor primaryContainer;
    ContentColor onPrimaryContainer;
    SurfaceColor secondary;
    ContentColor onSecondary;
    SurfaceColor secondaryContainer;
    ContentColor onSecondaryContainer;
    SurfaceColor tertiary;
    ContentColor onTertiary;
    SurfaceColor tertiaryContainer;
    ContentColor onTertiaryContainer;
    SurfaceColor error;
    ContentColor onError;
    SurfaceColor errorContainer;
    ContentColor onErrorContainer;
    SurfaceColor background;
    ContentColor onBackground;
    SurfaceColor surface;
    ContentColor onSurface;
    SurfaceColor surfaceVariant;
    ContentColor onSurfaceVariant;
    SurfaceColor outline;
    SurfaceColor outlineVariant;
    SurfaceColor shadow;
    SurfaceColor scrim;
    SurfaceColor inverseSurface;
    SurfaceColor inverseOnSurface;
    SurfaceColor inversePrimary;
    SurfaceColor primaryFixed;
    ContentColor onPrimaryFixed;
    SurfaceColor primaryFixedDim;
    ContentColor onPrimaryFixedVariant;
    SurfaceColor secondaryFixed;
    ContentColor onSecondaryFixed;
    SurfaceColor secondaryFixedDim;
    ContentColor onSecondaryFixedVariant;
    SurfaceColor tertiaryFixed;
    ContentColor onTertiaryFixed;
    SurfaceColor tertiaryFixedDim;
    ContentColor onTertiaryFixedVariant;
    SurfaceColor surfaceDim;
    SurfaceColor surfaceBright;
    SurfaceColor surfaceContainerLowest;
    SurfaceColor surfaceContainerLow;
    SurfaceColor surfaceContainer;
    SurfaceColor surfaceContainerHigh;
    SurfaceColor surfaceContainerHighest;

    uint32_t seedArgb;
    bool     darkMode = false;

    uint32_t primaryPalette;
    uint32_t secondaryPalette;
    uint32_t tertiaryPalette;
    uint32_t neutralPalette;
    uint32_t neutralVariantPalette;
    uint32_t errorPalette;

    friend class ThemeBuilder;

public:
    // clang-format off
    auto SeedArgb() const -> uint32_t { return seedArgb; }
    auto DarkMode() const -> bool { return darkMode; }

    auto PrimaryPalette() const -> uint32_t {return primaryPalette;}
    auto SecondaryPalette() const -> uint32_t {return secondaryPalette;}
    auto TertiaryPalette() const -> uint32_t {return tertiaryPalette;}
    auto NeutralPalette() const -> uint32_t {return neutralPalette;}
    auto NeutralVariantPalette() const -> uint32_t {return neutralVariantPalette;}
    auto ErrorPalette() const -> uint32_t {return errorPalette;}

    auto Primary() const -> const SurfaceColor & { return primary; }
    auto SurfaceTint() const -> const SurfaceColor & { return surfaceTint; }
    auto OnPrimary() const -> const ContentColor & { return onPrimary; }
    auto PrimaryContainer() const -> const SurfaceColor & { return primaryContainer; }
    auto OnPrimaryContainer() const -> const ContentColor & { return onPrimaryContainer; }
    auto Secondary() const -> const SurfaceColor & { return secondary; }
    auto OnSecondary() const -> const ContentColor & { return onSecondary; }
    auto SecondaryContainer() const -> const SurfaceColor & { return secondaryContainer; }
    auto OnSecondaryContainer() const -> const ContentColor & { return onSecondaryContainer; }
    auto Tertiary() const -> const SurfaceColor & { return tertiary; }
    auto OnTertiary() const -> const ContentColor & { return onTertiary; }
    auto TertiaryContainer() const -> const SurfaceColor & { return tertiaryContainer; }
    auto OnTertiaryContainer() const -> const ContentColor & { return onTertiaryContainer; }
    auto Error() const -> const SurfaceColor & { return error; }
    auto OnError() const -> const ContentColor & { return onError; }
    auto ErrorContainer() const -> const SurfaceColor & { return errorContainer; }
    auto OnErrorContainer() const -> const ContentColor & { return onErrorContainer; }
    auto Background() const -> const SurfaceColor & { return background; }
    auto OnBackground() const -> const ContentColor & { return onBackground; }
    auto Surface() const -> const SurfaceColor & { return surface; }
    auto OnSurface() const -> const ContentColor & { return onSurface; }
    auto SurfaceVariant() const -> const SurfaceColor & { return surfaceVariant; }
    auto OnSurfaceVariant() const -> const ContentColor & { return onSurfaceVariant; }
    auto Outline() const -> const SurfaceColor & { return outline; }
    auto OutlineVariant() const -> const SurfaceColor & { return outlineVariant; }
    auto Shadow() const -> const SurfaceColor & { return shadow; }
    auto Scrim() const -> const SurfaceColor & { return scrim; }
    auto InverseSurface() const -> const SurfaceColor & { return inverseSurface; }
    auto InverseOnSurface() const -> const SurfaceColor & { return inverseOnSurface; }
    auto InversePrimary() const -> const SurfaceColor & { return inversePrimary; }
    auto PrimaryFixed() const -> const SurfaceColor & { return primaryFixed; }
    auto OnPrimaryFixed() const -> const ContentColor & { return onPrimaryFixed; }
    auto PrimaryFixedDim() const -> const SurfaceColor & { return primaryFixedDim; }
    auto OnPrimaryFixedVariant() const -> const ContentColor & { return onPrimaryFixedVariant; }
    auto SecondaryFixed() const -> const SurfaceColor & { return secondaryFixed; }
    auto OnSecondaryFixed() const -> const ContentColor & { return onSecondaryFixed; }
    auto SecondaryFixedDim() const -> const SurfaceColor & { return secondaryFixedDim; }
    auto OnSecondaryFixedVariant() const -> const ContentColor & { return onSecondaryFixedVariant; }
    auto TertiaryFixed() const -> const SurfaceColor & { return tertiaryFixed; }
    auto OnTertiaryFixed() const -> const ContentColor & { return onTertiaryFixed; }
    auto TertiaryFixedDim() const -> const SurfaceColor & { return tertiaryFixedDim; }
    auto OnTertiaryFixedVariant() const -> const ContentColor & { return onTertiaryFixedVariant; }
    auto SurfaceDim() const -> const SurfaceColor & { return surfaceDim; }
    auto SurfaceBright() const -> const SurfaceColor & { return surfaceBright; }
    auto SurfaceContainerLowest() const -> const SurfaceColor & { return surfaceContainerLowest; }
    auto SurfaceContainerLow() const -> const SurfaceColor & { return surfaceContainerLow; }
    auto SurfaceContainer() const -> const SurfaceColor & { return surfaceContainer; }
    auto SurfaceContainerHigh() const -> const SurfaceColor & { return surfaceContainerHigh; }
    auto SurfaceContainerHighest() const -> const SurfaceColor & { return surfaceContainerHighest; }

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
    TextSizeSpec text;
    ImVec2       padding;
    ImVec2       spacing;
    float        size; // < 0 invalid
    float        rounding;
};

namespace FAB
{
static constexpr auto STANDARD = ButtonSpec{
    .text = TextSize::LARGE, .padding = ImVec2(16.f, 16.f), .spacing = ImVec2(), .size = 56.f, .rounding = 16.f
};
}

namespace IconButton
{
static constexpr auto XSMALL = ButtonSpec{
    .text = TextSize::MEDIUM, .padding = ImVec2(6.f, 6.f), .spacing = ImVec2(8.f, 8.f), .size = 48.f, .rounding = 8.f
};
static constexpr auto SMALL = ButtonSpec{
    .text = TextSize::LARGE, .padding = ImVec2(8.f, 8.f), .spacing = ImVec2(4.f, 4.f), .size = 48.f, .rounding = 8.f
};
}

namespace Button
{
static constexpr auto SMALL = ButtonSpec{
    .text = TextSize::LARGE, .padding = ImVec2(16.f, 10.f), .spacing = ImVec2(0.f, 4.f), .size = -1.f, .rounding = 12.f
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