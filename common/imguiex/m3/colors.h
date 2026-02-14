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

enum class ColorRole : std::uint8_t
{
    background,
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
    onBackground,
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

static constexpr float HOVER_OPACITY      = 0.08F;
static constexpr float PRESSED_OPACITY    = 0.12F;
static constexpr float FOCUS_OPACITY      = 0.12F;
static constexpr float DRAGGED_OPACITY    = 0.16F;
static constexpr float DISABLED_CONTENT   = 0.38F;
static constexpr float DISABLED_CONTAINER = 0.12F;

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
    struct SchemeConfig
    {
        double contrastLevel;
        Argb   sourceColor;
        bool   darkMode;
    };

    using Colors = std::array<ImVec4, static_cast<uint8_t>(ColorRole::count)>;

private:
    alignas(16) Colors m_colors{};
    SchemeConfig m_schemeConfig;

public:
    explicit ColorScheme(const Colors &colors, const SchemeConfig &schemeConfig)
        : m_colors(colors), m_schemeConfig(schemeConfig)
    {
    }

    ColorScheme(const ColorScheme &other) = default;

    ColorScheme(ColorScheme &&other) noexcept = default;

    auto operator=(const ColorScheme &other) -> ColorScheme &
    {
        if (this != &other)
        {
            m_colors       = other.m_colors;
            m_schemeConfig = other.m_schemeConfig;
        }
        return *this;
    }

    auto operator=(ColorScheme &&other) noexcept -> ColorScheme & = default;

    [[nodiscard]] auto GetSchemeConfig() const -> const SchemeConfig & { return m_schemeConfig; }

    //! \todo should refactor color system. All components colors should be defined in Specs.
    [[nodiscard]] auto at(ColorRole role) const -> const ImVec4 & { return m_colors.at(static_cast<size_t>(role)); }

    auto operator[](ColorRole role) const -> const ImVec4 & { return m_colors[static_cast<uint8_t>(role)]; }

    ////////////////////////////////////////////////////////////////////////
    /// Helpers

    [[nodiscard]] auto Hovered(ColorRole surfaceRole, ColorRole contentRole) const -> ImVec4
    {
        return BlendState(at(surfaceRole), at(contentRole), HOVER_OPACITY);
    }

    [[nodiscard]] auto Pressed(ColorRole surfaceRole, ColorRole contentRole) const -> ImVec4
    {
        return BlendState(at(surfaceRole), at(contentRole), PRESSED_OPACITY);
    }

    [[nodiscard]] auto DisabledSurface(ColorRole containerRole, ColorRole onSurfaceRole = ColorRole::onSurface) const
        -> ImVec4
    {
        return BlendState(at(containerRole), at(onSurfaceRole), DISABLED_CONTAINER);
    }

    [[nodiscard]] auto DisabledContent(ColorRole contentRole) const -> ImVec4
    {
        const ImVec4 onColor = at(contentRole);
        return {onColor.x, onColor.y, onColor.z, onColor.w * DISABLED_CONTENT};
    }

    static ImVec4 BlendState(ImVec4 base, ImVec4 overlay, float opacity)
    {
        return {
            base.x + (overlay.x - base.x) * opacity,
            base.y + (overlay.y - base.y) * opacity,
            base.z + (overlay.z - base.z) * opacity,
            base.w
        };
    }
};
} // namespace ImGuiEx::M3
