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

static constexpr auto TEXT_LABEL_SMALL    = Text(12.F, 16.F);
static constexpr auto TEXT_LABEL_LARGE    = Text(14.F, 20.F);
static constexpr auto TEXT_TITLE_MEDIUM   = Text(16.F, 24.F);
static constexpr auto TEXT_HEADLINE_SMALL = Text(24.F, 32.F);
static constexpr auto ICON_SIZE           = 24.F;

constexpr auto HALF = 0.5F;

inline auto HalfLineGap(const Text &text) -> float
{
    return (text.lineHeight - text.textSize) * HALF;
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
class FontScope
{
    bool            m_pushed   = false;
    Spec::TextRole  m_lastRole = Spec::TextRole();
    Spec::TextRole *m_currRole = nullptr;

public:
    FontScope() = default;

    explicit FontScope(ImFont *font, const float fontSize = 0.F) : m_pushed(true) { ImGui::PushFont(font, fontSize); }

    explicit FontScope(const Spec::TextRole lastRole, Spec::TextRole *role, ImFont *font, const float fontSize = 0.F)
        : m_pushed(true), m_lastRole(lastRole), m_currRole(role)
    {
        ImGui::PushFont(font, fontSize);
    }

    FontScope(const FontScope &other)               = delete;
    auto operator=(FontScope &other) -> FontScope & = delete;

    FontScope(FontScope &&other) noexcept
        : m_pushed(other.m_pushed), m_lastRole(other.m_lastRole), m_currRole(other.m_currRole)
    {
        other.m_pushed   = false;
        other.m_currRole = nullptr;
    }

    auto operator=(FontScope &&other) noexcept -> FontScope &
    {
        if (this == &other) return *this;
        m_pushed         = other.m_pushed;
        m_lastRole       = other.m_lastRole;
        m_currRole       = other.m_currRole;
        other.m_pushed   = false;
        other.m_currRole = nullptr;
        return *this;
    }

    ~FontScope()
    {
        if (m_pushed)
        {
            ImGui::PopFont();
        }
        if (m_currRole != nullptr)
        {
            *m_currRole = m_lastRole;
        }
    }
};
} // namespace detail

class M3Styles
{
public:
    /**
     * The expected maximum unit is 70 (Spec::List::width), which is 280dp. Precompute the pixel values for 0-70 units
     * to improve performance.
     *
     * If require unit more than 70, should check if the layout is correct, and then calculate on the fly. It is
     * recommended to use the precomputed ones as much as possible.
     */
    using GridUnitsPx = std::array<float, static_cast<size_t>(Spec::List::width)>;

    struct CachedTypeScale
    {
        Spec::TextRole currRole = Spec::TextRole::None;
        Text  unScaledText{}; //! The original text size and line height defined in the spec, without scaling applied.
        //! The text size and line height after applying the current scale factor. Updated whenever the text role
        //! changes or scaling is updated.
        Text  currText{};
        //! A derived value calculated as `(currText.lineHeight - currText.textSize) / 2`, used for vertical centering
        //! of text elements.
        float currHalfLineGap = 0.F;
    };

private:
    alignas(16) Colors colors;
    GridUnitsPx     precomputedPx{};
    //! The pixels size will not calculate until the first call to UseTextRole or UpdateScaling.
    //! @see UseTextRole
    CachedTypeScale m_cachedTypeScale;
    /**
     * It is highly recommended to provide a standalone ImFont pointer that has NOT been merged with other fonts.
     * Merging icon fonts often causes "Ascent" and "Descent" metrics to be re-calculated or corrupted to fit the
     * primary font's baseline, leading to vertical misalignment and "bleeding" outside the glyph bounding box. For
     * pixel-perfect grid alignment, use an independently loaded font.
     */
    ImFont         *iconFont{nullptr};
    float           m_currentScale = 0.0F;

    Text  smallLabelText = TEXT_LABEL_SMALL;
    Text  labelText      = TEXT_LABEL_LARGE;
    Text  titleText      = TEXT_TITLE_MEDIUM;
    float iconSize       = ICON_SIZE;

public:
    constexpr explicit M3Styles(Colors colors, ImFont *iconFont) : colors(std::move(colors)), iconFont(iconFont) {}

private:
    void UpdateTypeScaleScaling(float newScale);

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
     */
    void RebuildColors(const Colors::SchemeConfig &schemeConfig);

    /**
     * Sets the current text role and adjusts font size accordingly.
     *
     * Designed to align with ImGui's dynamic font API, this function returns a
     * `FontScope` object that manages the font's lifecycle via RAII.
     * **Usage Guide:**
     * * Call this method before drawing any windows, items, or text intended to use this role.
     * * The font will be automatically popped when the returned `FontScope` goes out of scope.
     *
     * **Best Practice:**
     * * It is recommended to call this at the beginning of a window or container's drawing code.
     * * This ensures all nested elements inherit the same text role without needing individual
     * calls for every text element.
     * @code
     * ImGui::NewFrame();
     * m3Styles.UseTextRole<Spec::TextRole::LabelLarge>();
     * ImGui::EndFrame();
     * @endcode
     *
     * **Usage Note:** To suppress `[[nodiscard]]` or unused return value warnings, use:
     * @code const auto _ =  m3Styles.UseTextRole<Role>() @endcode
     * @tparam Role The text role to use.
     * @return A FontScope that manages the font lifecycle.
     */
    template <Spec::TextRole Role>
    [[nodiscard]] auto UseTextRole() -> detail::FontScope
    {
        if (m_cachedTypeScale.currRole != Role)
        {
            const auto lastRole                       = m_cachedTypeScale.currRole;
            m_cachedTypeScale.unScaledText.textSize   = Spec::TypeScale<Role>::textSize;
            m_cachedTypeScale.unScaledText.lineHeight = Spec::TypeScale<Role>::lineHeight;
            UpdateTypeScaleScaling(m_currentScale);
            return detail::FontScope(
                lastRole, &m_cachedTypeScale.currRole, nullptr, m_cachedTypeScale.currText.textSize
            );
        }
        return {};
    }

    [[nodiscard]] auto GetLastText() const -> const CachedTypeScale & { return m_cachedTypeScale; }

    [[nodiscard]] auto Colors() const -> const Colors & { return colors; }

    [[nodiscard]] auto IconFont() const -> ImFont * { return iconFont; }

    [[deprecated("Please use GetPixels.")]] [[nodiscard]] auto Get(Spacing s) const -> float
    {
        return precomputedPx.at(static_cast<uint8_t>(s));
    }

    /**
     * The 0 - 140(list width 280dp / BASE_UNIT(2dp)) is precomputed for better performance, and the rest will be
     * calculated on the fly. It is recommended to use the precomputed ones as much as possible.
     * @param units the unit of spacing or component size. 1unit = 2ddp * currentScale.
     * @return the pixel value of the given units after scaling. If the units is less than 32, it will return the
     * precomputed
     */
    [[nodiscard]] auto GetPixels(const Spec::Unit units) const -> float
    {
        if (precomputedPx.size() > units)
        {
            return precomputedPx[units];
        }
        return static_cast<float>(units) * Spec::BASE_UNIT * m_currentScale;
    }

    [[deprecated("Please use GetPixels.")]] auto operator[](Spacing s) const -> float { return Get(s); }

    [[nodiscard]] auto SmallLabelText() const -> const Text & { return smallLabelText; }

    [[nodiscard]] auto LabelText() const -> const Text & { return labelText; }

    [[nodiscard]] auto TitleText() const -> const Text & { return titleText; }

    [[nodiscard]] auto IconSize() const -> float { return iconSize; }

    /**
     * This method does **not** internally calculate text size based on its TextRole.
     * It assumes the caller has already set the appropriate TextRole before invocation.
     * @tparam Component The Component.
     * @return The FramePadding (ImVec2) for the specified Component.
     */
    template <Spec::HasPaddingX Component>
    [[nodiscard]] auto GetPadding() const -> ImVec2
    {
        return {
            GetPixels(Component::paddingX),
            (GetPixels(Component::height) - m_cachedTypeScale.currText.lineHeight) * HALF
        };
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

}; // namespace ImGuiEx::M3
