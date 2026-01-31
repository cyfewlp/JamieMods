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
struct Text
{
    float fontSize;
    float lineHeight;

    constexpr auto GetTextSpacing() const -> float
    {
        return lineHeight - fontSize;
    }
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

    constexpr SurfaceColor Hovered(const ContentColor &onColor) const
    {
        return GetState(onColor, HOVER_OPACITY);
    }

    constexpr SurfaceColor Pressed(const ContentColor &onColor) const
    {
        return GetState(onColor, PRESSED_OPACITY);
    }
};

class ThemeBuilder;

enum class SurfaceToken : std::uint8_t
{
    background = 0,
    error,
    errorContainer,
    inversePrimary,
    inverseSurface,
    outline,
    outlineVariant,
    primary,
    primaryContainer,
    primaryFixed,
    primaryFixedDim,
    scrim,
    secondary,
    secondaryContainer,
    secondaryFixed,
    secondaryFixedDim,
    shadow,
    surface,
    surfaceBright,
    surfaceContainer,
    surfaceContainerHigh,
    surfaceContainerHighest,
    surfaceContainerLow,
    surfaceContainerLowest,
    surfaceDim,
    surfaceTint,
    surfaceVariant,
    tertiary,
    tertiaryContainer,
    tertiaryFixed,
    tertiaryFixedDim,
    count
};

enum class ContentToken : std::uint8_t
{
    onBackground = 0,
    onError,
    onErrorContainer,
    onPrimary,
    onPrimaryContainer,
    onPrimaryFixed,
    onPrimaryFixedVariant,
    onSecondary,
    onSecondaryContainer,
    onSecondaryFixed,
    onSecondaryFixedVariant,
    onSurface,
    onSurfaceVariant,
    onTertiary,
    onTertiaryContainer,
    onTertiaryFixed,
    onTertiaryFixedVariant,
    inverseOnSurface,
    count
};

class Colors
{
    std::array<SurfaceColor, static_cast<uint8_t>(SurfaceToken::count)> surfaceColors;
    std::array<ContentColor, static_cast<uint8_t>(ContentToken::count)> contentColors;
    uint32_t                                                            seedArgb;
    bool                                                                darkMode = false;

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

    auto Get(SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors[static_cast<uint8_t>(token)];
    }

    auto operator[](SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors[static_cast<uint8_t>(token)];
    }

    auto Get(ContentToken token) const -> const ContentColor &
    {
        return contentColors[static_cast<uint8_t>(token)];
    }

    auto operator[](ContentToken token) const -> const ContentColor &
    {
        return contentColors[static_cast<uint8_t>(token)];
    }

    // clang-format on
};

// Size tips to pass item draw functions
enum class SizeTips : uint8_t
{
    XSMALL = 0,
    SMALL  = 1,
    MEDIUM = 2,
    LARGE  = 4,
    XLARGE = 8
};

enum class Spacing : uint8_t
{
    None       = 0,
    XS         = 1,      // 4dp
    S          = 2,      // 8dp
    M          = 3,      // 12dp
    L          = 4,      // 16dp
    XL         = 6,      // 24dp
    XXL        = 8,      // 32dp
    Max        = 16,     // 64dp，
    Double_XS  = 1 << 1, // 8dp
    Double_S   = 2 << 1, // 16dp
    Double_M   = 3 << 1, // 24dp
    Double_L   = 4 << 1, // 32dp
    Double_XL  = 6 << 1, // 48dp
    Double_XXL = 8 << 1, // 64dp
};

enum class ComponentSize : uint8_t
{
    LIST_WIDTH     = 70, // 280dp
    NAV_RAIL_WIDTH = 24, // 96dp
    MENU_WIDTH     = 52, // 208dp
};

struct M3Styles
{
    static constexpr float BASE_UNIT = 4.0f;

    Colors colors;
    /**
     * It is highly recommended to provide a standalone ImFont pointer that has NOT been merged with other fonts.
     * Merging icon fonts often causes "Ascent" and "Descent" metrics to be re-calculated or corrupted to fit the
     * primary font's baseline, leading to vertical misalignment and "bleeding" outside the glyph bounding box. For
     * pixel-perfect grid alignment, use an independently loaded font.
     */
    ImFont *iconFont;

    constexpr M3Styles() = default;

private:
    Text  smallText{.fontSize = 12.f, .lineHeight = 16.f};
    Text  mediumText{.fontSize = 14.f, .lineHeight = 20.f};
    Text  largeText{.fontSize = 16.f, .lineHeight = 24.f};
    float iconSize = 24.f;

    std::array<float, 32> precomputedPx;
    float                 currentScale = 1.0f;

public:
    void UpdateScaling(const float newScale)
    {
        currentScale = newScale;
        for (size_t i = 0; i < precomputedPx.size(); ++i)
        {
            precomputedPx[i] = std::floor(static_cast<float>(i) * BASE_UNIT * newScale);
        }
        smallText.fontSize   = 12.f * newScale;
        smallText.lineHeight = 16.f * newScale;

        mediumText.fontSize   = 14.f * newScale;
        mediumText.lineHeight = 20.f * newScale;

        largeText.fontSize   = 16.f * newScale;
        largeText.lineHeight = 24.f * newScale;

        iconSize = 24.f * newScale;
    }

    auto Get(Spacing s) const -> float
    {
        return precomputedPx[static_cast<uint8_t>(s)];
    }

    auto GetUnit(uint8_t units) const -> float
    {
        return precomputedPx[units < 32 ? units : 31];
    }

    auto GetSize(ComponentSize componentSize) const -> float
    {
        return BASE_UNIT * currentScale * static_cast<float>(componentSize);
    }

    auto operator[](Spacing s) const -> float
    {
        return Get(s);
    }

    [[nodiscard]] auto GetSmallText() const -> const Text &
    {
        return smallText;
    }

    [[nodiscard]] auto GetMediumText() const -> const Text &
    {
        return mediumText;
    }

    [[nodiscard]] auto GetLargeText() const -> const Text &
    {
        return largeText;
    }

    [[nodiscard]] auto GetIconSize() const -> float
    {
        return iconSize;
    }
};

}
}