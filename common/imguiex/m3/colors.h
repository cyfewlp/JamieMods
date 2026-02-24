//
// Created by jamie on 2026/2/11.
//

#pragma once

#include "imgui.h"
#include "spec/color_roles.h"

#include <array>
#include <cstdint>

namespace ImGuiEx::M3
{
static constexpr auto ARGB_R_SHIFT = 16;
static constexpr auto ARGB_G_SHIFT = 8;
static constexpr auto ARGB_B_SHIFT = 0;
static constexpr auto ARGB_A_SHIFT = 24;
static constexpr auto ARGB_R_MASK  = 0x00FF0000U;
static constexpr auto ARGB_G_MASK  = 0x0000FF00U;
static constexpr auto ARGB_B_MASK  = 0x000000FFU;
static constexpr auto ARGB_A_MASK  = 0xFF000000U;

static constexpr auto CONTRAST_MIN = -1.0;
static constexpr auto CONTRAST_MAX = 1.0;

using Argb = std::uint32_t;

static constexpr auto IM_COL32_R_MASK = 0x000000FFU;
static constexpr auto IM_COL32_G_MASK = 0x0000FF00U;
static constexpr auto IM_COL32_B_MASK = 0x00FF0000U;

static constexpr float HOVER_OPACITY    = 0.08F;
static constexpr float PRESSED_OPACITY  = 0.10F;
static constexpr float FOCUS_OPACITY    = 0.10F;
static constexpr float DRAGGED_OPACITY  = 0.16F;
static constexpr float DISABLED_CONTENT = 0.38F;

constexpr auto ImU32ToArgb(const ImU32 imU32) -> Argb
{
    return (imU32 & IM_COL32_A_MASK) | (imU32 & IM_COL32_R_MASK) << ARGB_R_SHIFT | (imU32 & IM_COL32_G_MASK) |
           (imU32 & IM_COL32_B_MASK) >> IM_COL32_B_SHIFT;
}

constexpr auto ArgbToImU32(uint32_t argb) -> ImU32
{
    return (argb & ARGB_A_MASK) | (argb & ARGB_B_MASK) << 16 | (argb & ARGB_G_MASK) | (argb & ARGB_R_MASK) >> 16;
};

constexpr auto ArgbToImVec4(const Argb argb) -> ImVec4
{
    const uint8_t a = (argb & ARGB_A_MASK) >> ARGB_A_SHIFT;
    const uint8_t r = (argb & ARGB_R_MASK) >> ARGB_R_SHIFT;
    const uint8_t g = (argb & ARGB_G_MASK) >> ARGB_G_SHIFT;
    const uint8_t b = argb & ARGB_B_MASK;
    return ImColor(r, g, b, a);
}

namespace ColorUtils
{
//! Standard sRGB -> Linear conversion
inline float SrgbToLinear(float c)
{
    return c <= 0.04045F ? c / 12.92F : powf((c + 0.055F) / 1.055F, 2.4F);
}

//! Standard Linear -> sRGB conversion
inline float LinearToSrgb(float c)
{
    return c <= 0.0031308F ? c * 12.92F : 1.055F * powf(c, 1.0F / 2.4F) - 0.055F;
}

/**
 * @brief A fast approximation of sRGB -> Linear conversion. It is not accurate, but it is fast.
 */
inline float FastToLinear(float s)
{
    return s * s;
}

/**
 * @brief A fast approximation of Linear -> sRGB conversion. It is not accurate, but it is fast.
 */
inline float FastFromLinear(float l)
{
    return sqrtf(l);
}

inline auto BlendState(ImVec4 base, ImVec4 overlay, float opacity) -> ImVec4
{
    ImVec4 bL = {FastToLinear(base.x), FastToLinear(base.y), FastToLinear(base.z), base.w};
    ImVec4 oL = {FastToLinear(overlay.x), FastToLinear(overlay.y), FastToLinear(overlay.z), overlay.w};

    return {
        FastFromLinear(bL.x + (oL.x - bL.x) * opacity),
        FastFromLinear(bL.y + (oL.y - bL.y) * opacity),
        FastFromLinear(bL.z + (oL.z - bL.z) * opacity),
        1.0F // The alpha of the blended color is always 1.0F, as it is used for blending state layers on top of surface
             // colors.
    };
}

inline auto BlendHovered(ImVec4 color, ImVec4 overlay) -> ImVec4
{
    return BlendState(color, overlay, HOVER_OPACITY);
}

inline auto BlendPressed(ImVec4 color, ImVec4 overlay) -> ImVec4
{
    return BlendState(color, overlay, PRESSED_OPACITY);
}

inline auto BlendState(ImVec4 color, ImVec4 overlay, bool pressed, bool focused, bool hovered) -> ImVec4
{
    return BlendState(color, overlay, (pressed || focused) ? PRESSED_OPACITY : (hovered ? HOVER_OPACITY : 0.0F));
}

} // namespace ColorUtils

class ColorScheme
{
public:
    struct SchemeConfig
    {
        double contrastLevel;
        Argb   sourceColor;
        bool   darkMode;
    };

    using Colors = std::array<ImVec4, static_cast<uint8_t>(Spec::ColorRole::count)>;

private:
    alignas(16) Colors m_colors{};
    SchemeConfig m_schemeConfig;

public:
    explicit ColorScheme(const Colors &colors, const SchemeConfig &schemeConfig) : m_colors(colors), m_schemeConfig(schemeConfig) {}

    ~ColorScheme()                            = default;
    ColorScheme(const ColorScheme &other)     = default;
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

    [[nodiscard]] auto at(Spec::ColorRole role) const -> const ImVec4 & { return m_colors.at(static_cast<size_t>(role)); }

    auto operator[](Spec::ColorRole role) const -> const ImVec4 & { return m_colors[static_cast<uint8_t>(role)]; }

    [[nodiscard]] auto BlendState(Spec::ColorRole containerRole, Spec::ColorRole contentRole, const float opacity) const -> ImVec4
    {
        return ColorUtils::BlendState(at(containerRole), at(contentRole), opacity);
    }

    /**
     * @brief Helper function. Add a hovered state layer on top of a surface color.
     *
     * @param surfaceRole container color role, used as the base color for blending.
     * @param contentRole content color role, used as the overlay color for blending.
     * @return the blended color for the hovered state.
     */
    [[nodiscard]] auto Hovered(Spec::ColorRole surfaceRole, Spec::ColorRole contentRole) const -> ImVec4
    {
        return ColorUtils::BlendState(at(surfaceRole), at(contentRole), HOVER_OPACITY);
    }

    /**
     * @brief Helper function. Add a pressed state layer on top of a surface color.
     *
     * @param surfaceRole container color role, used as the base color for blending.
     * @param contentRole content color role, used as the overlay color for blending.
     * @return the blended color for the pressed state.
     */
    [[nodiscard]] auto Pressed(Spec::ColorRole surfaceRole, Spec::ColorRole contentRole) const -> ImVec4
    {
        return ColorUtils::BlendState(at(surfaceRole), at(contentRole), PRESSED_OPACITY);
    }

    /**
     * @brief Helper function to get the color for a specific state. It will automatically blend the state layer on top
     * of the surface color based on the state flags.
     * @see ColorUtils::BlendState for more details on how the blending works.
     */
    auto GetStateColor(Spec::ColorRole containerRole, Spec::ColorRole contentRole, bool hovered, bool pressed) const -> ImVec4
    {
        if (pressed)
        {
            return Pressed(containerRole, contentRole);
        }
        else if (hovered)
        {
            return Hovered(containerRole, contentRole);
        }
        else
        {
            return at(containerRole);
        }
    }
};
} // namespace ImGuiEx::M3
