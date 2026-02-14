//
// Created by jamie on 2026/2/11.
//

#pragma once

#include "imgui.h"

#include <array>
#include <cstdint>

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

    explicit ColorBase(ImU32 rgba) : raw(ImGui::ColorConvertU32ToFloat4(rgba)) {}

    constexpr operator ImVec4() const // NOLINT(*-explicit-constructor)
    {
        return raw;
    }

    explicit operator ImU32() const { return ImGui::ColorConvertFloat4ToU32(raw); }
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

class ColorScheme
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
    alignas(16) SurfaceColors surfaceColors{};
    ContentColors contentColors{};
    SchemeConfig  schemeConfig;

public:
    explicit ColorScheme(
        const SchemeConfig &schemeConfig, const SurfaceColors &surfaceColors, const ContentColors &contentColors
    )
        : surfaceColors(surfaceColors), contentColors(contentColors), schemeConfig(schemeConfig)
    {
    }

    ColorScheme(const ColorScheme &other) = default;

    ColorScheme(ColorScheme &&other) noexcept = default;

    auto operator=(const ColorScheme &other) -> ColorScheme &
    {
        if (this == &other) return *this;
        surfaceColors = other.surfaceColors;
        contentColors = other.contentColors;
        schemeConfig  = other.schemeConfig;
        return *this;
    }

    auto operator=(ColorScheme &&other) noexcept -> ColorScheme & = default;

    [[nodiscard]] auto GetSchemeConfig() const -> const SchemeConfig & { return schemeConfig; }

    //! \todo should refactor color system. All components colors should be defined in Specs.
    [[nodiscard]] auto at(SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors.at(static_cast<uint8_t>(token));
    }

    auto operator[](SurfaceToken token) const -> const SurfaceColor &
    {
        return surfaceColors[static_cast<uint8_t>(token)];
    }

    [[nodiscard]] auto at(ContentToken token) const -> const ContentColor &
    {
        return contentColors.at(static_cast<uint8_t>(token));
    }

    auto operator[](ContentToken token) const -> const ContentColor &
    {
        return contentColors[static_cast<uint8_t>(token)];
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

    [[nodiscard]] auto DisabledSurface() const -> SurfaceColor
    {
        const ImVec4 baseColor = at(SurfaceToken::surface);
        const ImVec4 onColor   = at(ContentToken::onSurface);
        return {
            baseColor.x + ((onColor.x - baseColor.x) * SurfaceColor::DISABLED_OPACITY),
            baseColor.y + ((onColor.y - baseColor.y) * SurfaceColor::DISABLED_OPACITY),
            baseColor.z + ((onColor.z - baseColor.z) * SurfaceColor::DISABLED_OPACITY),
            baseColor.w
        };
    }

    [[nodiscard]] auto DisabledContent() const -> ContentColor
    {
        const ImVec4 onColor = at(ContentToken::onSurface);
        return {onColor.x, onColor.y, onColor.z, onColor.w * ContentColor::DISABLED_OPACITY};
    }
};
} // namespace ImGuiEx::M3
