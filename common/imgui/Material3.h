//
// Created by jamie on 2026/1/18.
//

#pragma once

#include <cstdint>
#include <imgui.h>
#include <utility>

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

static constexpr auto CONTRAST_MIN = -1.0;
static constexpr auto CONTRAST_MAX = 1.0;

struct Text
{
    float fontSize;
    float lineHeight;
};

static constexpr auto TEXT_LABEL_SMALL    = Text(12.F, 16.F);
static constexpr auto TEXT_LABEL_LARGE    = Text(14.F, 20.F);
static constexpr auto TEXT_TITLE_MEDIUM   = Text(16.F, 24.F);
static constexpr auto TEXT_HEADLINE_SMALL = Text(24.F, 32.F);
static constexpr auto ICON_SIZE           = 24.F;

class ColorBase
{
    ImVec4 raw{0, 0, 0, 0};

public:
    constexpr ColorBase() = default;

    constexpr ColorBase(float r, float g, float b, float a = 1.0F) : raw(r, g, b, a) {}

    constexpr explicit ColorBase(const ImVec4 &col) : raw(col) {}

    constexpr auto operator=(const ImVec4 &col) -> ColorBase &
    {
        raw = col;
        return *this;
    }

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

    static constexpr float DISABLED_OPACITY = 0.38F;
};

class SurfaceColor : public ColorBase
{
public:
    using ColorBase::ColorBase;
    using ColorBase::operator=;

    static constexpr float HOVER_OPACITY    = 0.08F;
    static constexpr float PRESSED_OPACITY  = 0.12F;
    static constexpr float DISABLED_OPACITY = 0.10F;

    [[nodiscard]] constexpr auto GetState(const ContentColor &onColor, const float stateOpacity) const -> SurfaceColor
    {
        const auto l = static_cast<ImVec4>(*this);
        const auto r = static_cast<ImVec4>(onColor);
        return {
            l.x + ((r.x - l.x) * stateOpacity),
            l.y + ((r.y - l.y) * stateOpacity),
            l.z + ((r.z - l.z) * stateOpacity),
            l.w
        };
    }

    [[nodiscard]] constexpr auto Hovered(const ContentColor &onColor) const -> SurfaceColor
    {
        return GetState(onColor, HOVER_OPACITY);
    }

    [[nodiscard]] constexpr auto Pressed(const ContentColor &onColor) const -> SurfaceColor
    {
        return GetState(onColor, PRESSED_OPACITY);
    }
};

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

using Argb = std::uint32_t;

static constexpr auto IM_COL32_R_MASK = 0xFF;
static constexpr auto IM_COL32_G_MASK = 0xFF00;
static constexpr auto IM_COL32_B_MASK = 0xFF0000;

constexpr auto ImU32ToArgb(const ImU32 imU32) -> Argb
{
    return (imU32 & IM_COL32_A_MASK) | (imU32 & IM_COL32_R_MASK) << ARGB_R_SHIFT | (imU32 & IM_COL32_G_MASK) |
           (imU32 & IM_COL32_B_MASK) >> IM_COL32_B_SHIFT;
}

constexpr auto ArgbToImVec4(const Argb argb) -> ImVec4
{
    const uint8_t a = (argb & ARGB_A_MASK) >> ARGB_A_SHIFT;
    const uint8_t r = (argb & ARGB_R_MASK) >> ARGB_R_SHIFT;
    const uint8_t g = (argb & ARGB_G_MASK) >> ARGB_G_SHIFT;
    const uint8_t b = argb & ARGB_B_MASK;
    return ImColor(r, g, b, a);
}

class Colors
{
public:
    using SurfaceColors = std::array<SurfaceColor, static_cast<uint8_t>(SurfaceToken::count)>;
    using ContentColors = std::array<ContentColor, static_cast<uint8_t>(ContentToken::count)>;

    struct SchemeConfig
    {
        double contrastLevel = 0.0;
        Argb   sourceColor{};
        bool   darkMode = false;
    };

private:
    SurfaceColors surfaceColors{};
    ContentColors contentColors{};
    SchemeConfig  schemeConfig;

public:
    explicit Colors(
        const SchemeConfig &schemeConfig, const SurfaceColors &surfaceColors, const ContentColors &contentColors
    )
        : surfaceColors(surfaceColors), contentColors(contentColors), schemeConfig(schemeConfig)
    {
    }

    Colors(const Colors &other) = default;

    Colors(Colors &&other) noexcept
        : surfaceColors(other.surfaceColors), contentColors(other.contentColors), schemeConfig(other.schemeConfig)
    {
    }

    auto operator=(const Colors &other) -> Colors &
    {
        if (this == &other) return *this;
        surfaceColors = other.surfaceColors;
        contentColors = other.contentColors;
        schemeConfig  = other.schemeConfig;
        return *this;
    }

    auto operator=(Colors &&other) noexcept -> Colors &
    {
        if (this == &other) return *this;
        surfaceColors = other.surfaceColors;
        contentColors = other.contentColors;
        schemeConfig  = other.schemeConfig;
        return *this;
    }

    [[nodiscard]] auto GetSchemeConfig() const -> const SchemeConfig &
    {
        return schemeConfig;
    }

    [[nodiscard]] auto at(SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors.at(static_cast<uint8_t>(token));
    }

    auto operator[](SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors.at(static_cast<uint8_t>(token));
    }

    [[nodiscard]] auto at(ContentToken token) const -> const ContentColor &
    {
        return contentColors.at(static_cast<uint8_t>(token));
    }

    auto operator[](ContentToken token) const -> const ContentColor &
    {
        return contentColors.at(static_cast<uint8_t>(token));
    }

    ////////////////////////////////////////////////////////////////////////
    /// Helpers

    [[nodiscard]] auto Hovered(SurfaceToken surfaceToken, ContentToken contentToken) const -> SurfaceColor
    {
        return at(surfaceToken).Hovered(at(contentToken));
    }

    [[nodiscard]] auto Pressed(SurfaceToken surfaceToken, ContentToken contentToken) const -> SurfaceColor
    {
        return at(surfaceToken).Pressed(at(contentToken));
    }
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
    LIST_WIDTH       = 70, // 280dp
    NAV_RAIL_WIDTH   = 24, // 96dp
    MENU_WIDTH       = 52, // 208dp
    BUTTON_ROUNDING  = 25, // 100dp
    TOOLBAR_ROUNDING = 8,  // 32dp
    ICON_BUTTON      = 12, // 48dp
};

class M3Styles
{
    static constexpr float BASE_UNIT = 4.0F;

    Colors colors;
    /**
     * It is highly recommended to provide a standalone ImFont pointer that has NOT been merged with other fonts.
     * Merging icon fonts often causes "Ascent" and "Descent" metrics to be re-calculated or corrupted to fit the
     * primary font's baseline, leading to vertical misalignment and "bleeding" outside the glyph bounding box. For
     * pixel-perfect grid alignment, use an independently loaded font.
     */
    ImFont *iconFont{nullptr};

    Text  smallLabelText = TEXT_LABEL_SMALL;
    Text  labelText      = TEXT_LABEL_LARGE;
    Text  titleText      = TEXT_TITLE_MEDIUM;
    float iconSize       = ICON_SIZE;

    std::array<float, static_cast<size_t>(Spacing::Count)> precomputedPx{};

    float currentScale = 0.0F;

public:
    constexpr explicit M3Styles(Colors colors, ImFont *iconFont) : colors(std::move(colors)), iconFont(iconFont)
    {
        UpdateScaling(1.0F);
    }

    void UpdateScaling(const float newScale)
    {
        if (currentScale == newScale)
        {
            return;
        }
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

    void RebuildColors(const Colors::SchemeConfig &schemeConfig);

    [[nodiscard]] auto Colors() const -> const Colors &
    {
        return colors;
    }

    [[nodiscard]] auto IconFont() const -> ImFont *
    {
        return iconFont;
    }

    [[nodiscard]] auto Get(Spacing s) const -> float
    {
        return precomputedPx.at(static_cast<uint8_t>(s));
    }

    [[nodiscard]] auto GetUnit(uint8_t units) const -> float
    {
        return precomputedPx.at(units < 32 ? units : 31);
    }

    [[nodiscard]] auto GetSize(ComponentSize componentSize) const -> float
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

} // namespace ImGuiEx::M3
