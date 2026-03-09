//
// Created by jamie on 2026/1/18.
//

#pragma once

#include "m3/colors.h"
#include "m3/spec/others.h"

#include <array>
#include <cstdint>
#include <imgui.h>
#include <utility>

/// https://m3.material.io/
namespace ImGuiEx::M3
{
struct Text
{
    float textSize;
    float lineHeight;
};

static constexpr auto TEXT_LABEL_LARGE = Text(14.F, 20.F);
static constexpr auto ICON_SIZE        = 24.F;

constexpr auto HALF = 0.5F;

inline auto HalfLineGap(const Text &text) -> float
{
    return (text.lineHeight - text.textSize) * HALF;
}

inline auto HalfDiff(float v1, float v2) -> float
{
    return std::abs((v2 - v1) * HALF);
}

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

namespace detail
{

struct CachedTypeScale
{
    Spec::TextRole currRole;
    Text           unScaledText; //! The original text size and line height defined in the spec, without scaling applied.
    //! The text size and line height after applying the current scale factor. Updated whenever the text role
    //! changes or scaling is updated.
    Text           currText;
    //! A derived value calculated as `(currText.lineHeight - currText.textSize) / 2`, used for vertical centering
    //! of text elements.
    float          currHalfLineGap;
};

class FontScope
{
    bool             m_pushed = false;
    CachedTypeScale  m_lastTypeScale;
    CachedTypeScale *m_currTypeScale = nullptr;

public:
    FontScope() = default;

    explicit FontScope(ImFont *font, const float fontSize = 0.F) : m_pushed(true) { ImGui::PushFont(font, fontSize); }

    explicit FontScope(const CachedTypeScale lastTypeScale, CachedTypeScale *role, ImFont *font, const float fontSize = 0.F)
        : m_pushed(true), m_lastTypeScale(lastTypeScale), m_currTypeScale(role)
    {
        ImGui::PushFont(font, fontSize);
    }

    FontScope(const FontScope &other)               = delete;
    auto operator=(FontScope &other) -> FontScope & = delete;

    FontScope(FontScope &&other) noexcept : m_pushed(other.m_pushed), m_lastTypeScale(other.m_lastTypeScale), m_currTypeScale(other.m_currTypeScale)
    {
        other.m_pushed        = false;
        other.m_currTypeScale = nullptr;
    }

    auto operator=(FontScope &&other) noexcept -> FontScope &
    {
        if (this == &other) return *this;
        m_pushed              = other.m_pushed;
        m_lastTypeScale       = other.m_lastTypeScale;
        m_currTypeScale       = other.m_currTypeScale;
        other.m_pushed        = false;
        other.m_currTypeScale = nullptr;
        return *this;
    }

    auto LastTypeScale() const -> const CachedTypeScale & { return m_lastTypeScale; }

    auto CurrTypeScale() const -> const CachedTypeScale & { return *m_currTypeScale; }

    //! @brief early pop the font before the scope ends.
    //! This may be used to avoid the ImGui "ASSERT" before EndChild/End/EndPopup and etc.
    void _PopFont()
    {
        if (std::exchange(m_pushed, false))
        {
            ImGui::PopFont();
        }
        if (m_currTypeScale != nullptr)
        {
            *m_currTypeScale = m_lastTypeScale;
            m_currTypeScale  = nullptr;
        }
    }

    ~FontScope() { _PopFont(); }
};
} // namespace detail

/// @brief Material 3 Design System styles.
///
/// @warning NOT thread-safe. All UI rendering must happen on a single thread.
/// The calling thread can change between frames, but concurrent access is undefined behavior.
///
/// @note This is consistent with ImGui's threading model, which also requires
/// single-threaded access to each ImGuiContext.
class M3Styles
{
public:
    /**
     * The expected maximum unit is deduced from `Spec::List::width`, which is 280dp. The actual size depends on the
     * BASE_UNIT defined in the spec(default is 2dp, so the maximum unit is 280 / 2 = 140).
     *
     * For better performance, the 0 - (Spec::List::width / BASE_UNIT) units are precomputed into a lookup table. If you
     * require units beyond this range, you should verify the layout and calculate the pixel values on the fly. It is
     * recommended to use the precomputed values as much as possible for optimal performance.
     */
    using GridUnitsPx = std::array<float, static_cast<size_t>(Spec::List::width)>;

private:
    alignas(16) ColorScheme m_scheme;
    GridUnitsPx             m_precomputedPx{};
    //! The pixels size will not calculate until the first call to UseTextRole or UpdateScaling.
    //! @see UseTextRole
    detail::CachedTypeScale m_cachedTypeScale;
    /**
     * It is highly recommended to provide a standalone ImFont pointer that has NOT been merged with other fonts.
     * Merging icon fonts often causes "Ascent" and "Descent" metrics to be re-calculated or corrupted to fit the
     * primary font's baseline, leading to vertical misalignment and "bleeding" outside the glyph bounding box. For
     * pixel-perfect grid alignment, use an independently loaded font.
     */
    ImFont                 *iconFont{nullptr};
    float                   m_currentScale = 0.0F;

public:
    constexpr explicit M3Styles(ColorScheme colors, ImFont *iconFont) : m_scheme(std::move(colors)), iconFont(iconFont) {}

private:
    static void UpdateTypeScaleScaling(detail::CachedTypeScale &cachedTypeScale, float newScale);

public:
    /**
     * You can call this before frame. Only re-compute size when the scale factor changes.
     * @param newScale the new scale factor.
     */
    void UpdateScaling(float newScale);

    /**
     * rebuild colors according to the new scheme config.
     * DON'T call this frequently, as it is a heavy operation.
     * @param schemeConfig A simple struct. see Colors::SchemeConfig
     *
     * @note Some ImGui StyleColor also still config by ImGui `Styles` object, like WindowBg,ChildBg.
     * This can be reduce some style colors push.
     */
    void RebuildColors(const ColorScheme::SchemeConfig &schemeConfig);

    void RebuildColors(bool darkMode)
    {
        RebuildColors({.contrastLevel = m_scheme.GetContrastLevel(), .sourceColor = m_scheme.GetSourceColor(), .darkMode = darkMode});
    }

    //! @brief Helper method to switch colors between light and dark mode.
    void ToggleLightDarkScheme() { RebuildColors(!m_scheme.IsDark()); }

    /**
     * @brief Sets the text role and returns an RAII FontScope to manage font lifecycle.
     *
     * Designed to align with ImGui's dynamic font API. Calling this at the start of a window
     * or container ensures all nested elements inherit the same role without redundant calls.
     *
     * @param value The type scale value containing role, text size, and line height.
     * @return A FontScope that automatically pops the font when it leaves scope.
     * @code
     * {
     * const auto _ = m3Styles.UseTextRole<Spec::TextRole::LabelLarge>();
     * ImGui::Text("This uses LabelLarge styling.");
     * } // Font automatically popped here
     * @endcode
     *
     * @note **Warning Suppression:** If the return value is not needed, use `const auto _ = ...` to satisfy `[[nodiscard]]`.
     * @important Please not use `UseTextRole` in some scoped components like `DialogModalScope`, `AppBarScope`, etc.,
     * because these components will manage the font scope internally, and using `UseTextRole` may cause unexpected behavior.
     */
    [[nodiscard]] auto UseTextRole(const Spec::TypeScaleValue &value) -> detail::FontScope
    {
        if (m_cachedTypeScale.currRole != value.role)
        {
            detail::CachedTypeScale cachedTypeScale   = m_cachedTypeScale;
            m_cachedTypeScale.currRole                = value.role;
            m_cachedTypeScale.unScaledText.textSize   = value.textSize;
            m_cachedTypeScale.unScaledText.lineHeight = value.lineHeight;
            UpdateTypeScaleScaling(m_cachedTypeScale, m_currentScale);
            return detail::FontScope(cachedTypeScale, &m_cachedTypeScale, nullptr, m_cachedTypeScale.currText.textSize);
        }
        return {};
    }

    /**
     * @brief Sets the text role and returns an RAII FontScope to manage font lifecycle.
     *
     * @tparam Role The text role to apply.
     */
    template <Spec::TextRole Role>
    [[nodiscard]] auto UseTextRole() -> detail::FontScope
    {
        return UseTextRole(Spec::TypeScaleValue::of<Role>());
    }

    [[nodiscard]] auto GetLastText() const -> const detail::CachedTypeScale & { return m_cachedTypeScale; }

    [[nodiscard]] auto Colors() const -> const ColorScheme & { return m_scheme; }

    [[nodiscard]] auto IconFont() const -> ImFont * { return iconFont; }

    [[deprecated("Please use GetPixels.")]] [[nodiscard]] auto Get(Spacing s) const -> float { return m_precomputedPx.at(static_cast<uint8_t>(s)); }

    /**
     * Get the pixels with current scale applied for the given units. The units are defined in the spec and represent a
     * multiple of the base unit (e.g., 4dp, 8dp, etc.). The method first checks if the pixel value for the given units
     * has been precomputed and stored in the `precomputedPx` array. If it has, it returns the precomputed value for
     * optimal performance. If not, it calculates the pixel value on the fly using the formula: `units * Spec::BASE_UNIT
     * * m_currentScale`. This allows for dynamic scaling while still providing fast access to commonly used spacing
     * values.
     *
     * @param units the unit of spacing or component size. 1unit = Spec::BASE_UNIT * currentScale.
     * @return the pixel value of the given units after scaling. If the units
     */
    [[nodiscard]] auto GetPixels(const Spec::Unit units) const -> float
    {
        if (m_precomputedPx.size() > units)
        {
            return m_precomputedPx[units];
        }
        return static_cast<float>(units) * static_cast<float>(Spec::BASE_UNIT) * m_currentScale;
    }

    /**
     * @brief Get the pixel value for a given size in dp (density-independent pixels) based on the current scale factor.
     * @param dpSize The size in dp units to be converted to pixels. This value is multiplied by the current scale
     * factor to get the final pixel value.
     * @return The calculated pixel value corresponding to the given dp size.
     */
    [[nodiscard]] auto GetPixels(const float dpSize) const -> float { return dpSize * m_currentScale; }

    [[deprecated("Please use GetPixels.")]] auto operator[](Spacing s) const -> float { return Get(s); }

    /**
     * This method does **not** internally calculate text size based on its TextRole.
     * It assumes the caller has already set the appropriate TextRole before invocation.
     * @tparam Component The Component.
     * @return The FramePadding (ImVec2) for the specified Component.
     */
    template <Spec::HasPaddingX Component>
    [[nodiscard]] auto GetPadding() const -> ImVec2
    {
        return {GetPixels(Component::paddingX), (GetPixels(Component::height) - m_cachedTypeScale.currText.lineHeight) * HALF};
    }

    /**
     * The component has exact padding defined.
     * @tparam Component The Component.
     * @return The FramePadding (ImVec2) for the specified Component.
     */
    template <Spec::HasExactPadding Component>
    [[nodiscard]] auto GetPadding() const -> ImVec2
    {
        return {GetPixels(Component::paddingX), GetPixels(Component::paddingY)};
    }

    template <Spec::HasRounding Component>
    [[nodiscard]] auto GetRounding() const -> float
    {
        return GetPixels(Component::rounding);
    }

    template <Spec::HasGap Component>
    [[nodiscard]] auto GetGap() const -> float
    {
        return GetPixels(Component::gap);
    }

    template <Spec::HasWidth Component>
    [[nodiscard]] auto GetWidth() const -> float
    {
        return GetPixels(Component::width);
    }

    template <Spec::HasHeight Component>
    [[nodiscard]] auto GetHeight() const -> float
    {
        return GetPixels(Component::height);
    }
};

static_assert(alignof(M3Styles) >= 16, "M3Styles must be 16-byte aligned to support SIMD optimizations.");

namespace Context
{
/**
 * @brief Initializes the global M3Styles instance.
 * @important **Why iconFont is REQUIRED:**
 * M3 components (NavItems, Buttons, etc.) rely on icon metrics to calculate precise layouts
 * and vertical centering. Without a valid @c ImFont, layout dimensions cannot be determined.
 * @note **Font Source & Recommendations:**
 * We do not bundle default fonts. Recommended icon sets:
 * - **Lucide Icons** (Highly Recommended)
 * - **Heroicons** or **Material Symbols**
 * @note **Automation Toolchain:**
 * To minimize binary size, we provide a specialized script: scripts/build_icons_font.py.
 * This tool works with **FontForge** to:
 * 1. Extract specific SVGs from an icon list.
 * 2. Package them into a lightweight, custom font file.
 * 3. Generate a matching `icons.h` header with glyph definitions.
 * @note **Font Merging:** It is strongly recommended to use a standalone @c ImFont.
 * Merged fonts may inherit incompatible baselines, breaking MD3's vertical alignment.
 *
 * @param iconFont     [Required] The font containing M3 icons. Pass @c ImGui::GetFont()
 * as a placeholder only if no icons will be rendered.
 * @param schemeConfig Color configuration. Defaults to @c ColorScheme::GetM3ClassicSchemeConfig().
 * @return A reference to the initialized @c M3Styles instance.
 */
auto CreateM3Styles(ImFont *iconFont, const ColorScheme::SchemeConfig &schemeConfig = ColorScheme::GetM3ClassicSchemeConfig()) -> M3Styles &;
auto DestroyM3Styles() -> void;

auto GetM3Styles() -> M3Styles &;

} // namespace Context

} // namespace ImGuiEx::M3
