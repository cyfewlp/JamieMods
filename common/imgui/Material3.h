//
// Created by jamie on 2026/1/18.
//

#pragma once

#include "common/config.h"

#include <cstdint>
#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
/// https://m3.material.io/
namespace ImGuiEx::M3
{

static constexpr auto ARGB_R_SHIFT = 16;
static constexpr auto ARGB_G_SHIFT = 8;
static constexpr auto ARGB_B_SHIFT = 0;
static constexpr auto ARGB_A_SHIFT = 24;
static constexpr auto ARGB_R_MASK  = 0xFF0000;
static constexpr auto ARGB_G_MASK  = 0xFF00;
static constexpr auto ARGB_B_MASK  = 0xFF;
static constexpr auto ARGB_A_MASK  = 0xFF000000;

struct Text
{
    float fontSize;
    float lineHeight;
};

static constexpr auto TEXT_LABEL_SMALL    = Text(12.f, 16.f);
static constexpr auto TEXT_LABEL_LARGE    = Text(14.f, 20.f);
static constexpr auto TEXT_TITLE_MEDIUM   = Text(16.f, 24.f);
static constexpr auto TEXT_HEADLINE_SMALL = Text(24.f, 32.f);
static constexpr auto ICON_SIZE           = 24.f;

class ColorBase
{
    ImVec4 raw{0, 0, 0, 0};

public:
    constexpr ColorBase() = default;

    constexpr ColorBase(float r, float g, float b, float a = 1.0f) : raw(r, g, b, a) {}

    constexpr explicit ColorBase(const ImVec4 &col) : raw(col) {}

    constexpr auto operator=(const ImVec4 &col) -> ColorBase &
    {
        raw = col;
        return *this;
    }

    constexpr ColorBase(int r, int g, int b, int a = 255) : raw(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f) {}

    constexpr explicit ColorBase(ImU32 rgba) : raw(ImGui::ColorConvertU32ToFloat4(rgba)) {}

    constexpr operator ImVec4() const // NOLINT(*-explicit-constructor)
    {
        return raw;
    }

    constexpr explicit operator ImU32() const
    {
        return ImGui::ColorConvertFloat4ToU32(raw);
    }
};

struct ContentColor : ColorBase
{
    using ColorBase::ColorBase;
    using ColorBase::operator=;

    static constexpr float DISABLED_OPACITY = 0.38f;
};

class SurfaceColor : public ColorBase
{
public:
    using ColorBase::ColorBase;
    using ColorBase::operator=;

    static constexpr float HOVER_OPACITY    = 0.08f;
    static constexpr float PRESSED_OPACITY  = 0.12f;
    static constexpr float DISABLED_OPACITY = 0.10f;

    constexpr SurfaceColor GetState(const ContentColor &onColor, const float stateOpacity) const
    {
        const ImVec4 l = static_cast<ImVec4>(*this);
        const ImVec4 r = static_cast<ImVec4>(onColor);
        return SurfaceColor(
            l.x + (r.x - l.x) * stateOpacity, l.y + (r.y - l.y) * stateOpacity, l.z + (r.z - l.z) * stateOpacity, l.w
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

    friend class ThemeBuilder;

public:
    explicit Colors(const uint32_t seedArgb, const bool darkMode) : seedArgb(seedArgb), darkMode(darkMode) {}

    // clang-format off
    auto SeedArgb() const -> uint32_t { return seedArgb; }
    auto DarkMode() const -> bool { return darkMode; }

    auto at(SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors.at(static_cast<uint8_t>(token));
    }

    auto operator[](SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors.at(static_cast<uint8_t>(token));
    }

    auto at(ContentToken token) const -> const ContentColor &
    {
        return contentColors.at(static_cast<uint8_t>(token));
    }

    auto operator[](ContentToken token) const -> const ContentColor &
    {
        return contentColors.at(static_cast<uint8_t>(token));
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
    PADDING_XL = 12,     // 48dp
    Max        = 16,     // 64dp，
    Double_XS  = 1 << 1, // 8dp
    Double_S   = 2 << 1, // 16dp
    Double_M   = 3 << 1, // 24dp
    Double_L   = 4 << 1, // 32dp
    Double_XL  = 6 << 1, // 48dp
    Double_XXL = 8 << 1, // 64dp
    Count      = 32
};

enum class ComponentSize : uint8_t
{
    LIST_WIDTH      = 70, // 280dp
    NAV_RAIL_WIDTH  = 24, // 96dp
    MENU_WIDTH      = 52, // 208dp
    BUTTON_ROUNDING = 25, // 100dp
    ICON_BUTTON     = 12, // 48dp
};

class M3Styles
{
    static constexpr float BASE_UNIT = 4.0f;

    Colors colors;
    /**
     * It is highly recommended to provide a standalone ImFont pointer that has NOT been merged with other fonts.
     * Merging icon fonts often causes "Ascent" and "Descent" metrics to be re-calculated or corrupted to fit the
     * primary font's baseline, leading to vertical misalignment and "bleeding" outside the glyph bounding box. For
     * pixel-perfect grid alignment, use an independently loaded font.
     */
    ImFont *iconFont{nullptr};

    Text  smallLabelText = TEXT_LABEL_LARGE;
    Text  labelText      = TEXT_LABEL_LARGE;
    Text  titleText      = TEXT_TITLE_MEDIUM;
    float iconSize       = ICON_SIZE;

    std::array<float, static_cast<size_t>(Spacing::Count)> precomputedPx{};

    float currentScale = 1.0f;

public:
    constexpr explicit M3Styles(const Colors &colors, ImFont *iconFont) : colors(colors), iconFont(iconFont)
    {
        UpdateScaling(currentScale);
    }

    void UpdateScaling(const float newScale)
    {
        currentScale = newScale;
        for (size_t i = 0; i < precomputedPx.size(); ++i)
        {
            precomputedPx.at(i) = std::floor(static_cast<float>(i) * BASE_UNIT * newScale);
        }
        labelText.fontSize   = TEXT_LABEL_LARGE.fontSize * newScale;
        labelText.lineHeight = TEXT_LABEL_LARGE.lineHeight * newScale;

        titleText.fontSize   = TEXT_TITLE_MEDIUM.fontSize * newScale;
        titleText.lineHeight = TEXT_TITLE_MEDIUM.lineHeight * newScale;

        iconSize = ICON_SIZE * newScale;
    }

    void RebuildColors(const uint32_t sourceColor, const bool isDark);

    auto Colors() const -> const Colors &
    {
        return colors;
    }

    auto IconFont() const -> ImFont *
    {
        return iconFont;
    }

    auto Get(Spacing s) const -> float
    {
        return precomputedPx.at(static_cast<uint8_t>(s));
    }

    auto GetUnit(uint8_t units) const -> float
    {
        return precomputedPx.at(units < 32 ? units : 31);
    }

    auto GetSize(ComponentSize componentSize) const -> float
    {
        return BASE_UNIT * currentScale * static_cast<float>(componentSize);
    }

    auto operator[](Spacing s) const -> float
    {
        return Get(s);
    }

    [[nodiscard]] auto SmallLabelText() const -> const Text &
    {
        return smallLabelText;
    }

    [[nodiscard]] auto LabelText() const -> const Text &
    {
        return labelText;
    }

    [[nodiscard]] auto TitleText() const -> const Text &
    {
        return titleText;
    }

    [[nodiscard]] auto IconSize() const -> float
    {
        return iconSize;
    }
};

}
}