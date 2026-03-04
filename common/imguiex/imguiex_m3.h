//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "ImGuiEx.h"
#include "Material3.h"
#include "imguiex_enum_wrap.h"
#include "m3/facade/appbar.h"
#include "m3/facade/icon_button.h"
#include "m3/spec/buttons.h"
#include "m3/spec/chips.h"
#include "m3/spec/fab.h"
#include "m3/spec/menu.h"
#include "m3/spec/text_field.h"
#include "m3/spec/tool_bar.h"

#include <functional>
#include <string_view>

namespace ImGuiEx::M3
{
constexpr int CHANNEL_FG = 1;
constexpr int CHANNEL_BG = 0;

template <typename Derived>
struct BaseConfiguration
{
    Spec::SizeTips size = Spec::SizeTips::SMALL; ///< default to small

    constexpr auto Size(Spec::SizeTips s) -> Derived &
    {
        size = s;
        return static_cast<Derived &>(*this);
    }

    constexpr auto XSmall() { return Size(Spec::SizeTips::XSMALL); }

    constexpr auto Small() { return Size(Spec::SizeTips::SMALL); }

    constexpr auto Medium() { return Size(Spec::SizeTips::MEDIUM); }

    constexpr auto Large() { return Size(Spec::SizeTips::LARGE); }

    constexpr auto XLarge() { return Size(Spec::SizeTips::XLARGE); }
};

//! Support line height with last used TextRole's line height. If the line height is not set or less than font size, it
//! will fall back to normal text rendering.
void TextUnformatted(const std::string_view &text, Spec::ColorRole contentRole = Spec::ColorRole::onSurface);

template <Spec::TextRole Role>
void TextUnformatted(const std::string_view &text, Spec::ColorRole contentRole = Spec::ColorRole::onSurface)
{
    auto &m3Styles = Context::GetM3Styles();

    const auto fontScope = m3Styles.UseTextRole(Spec::TypeScaleValue::of<Role>());
    TextUnformatted(text, contentRole);
}

/**
 * @brief Renders a Material Design 3 style Navigation Rail item.
 * * Draws an interactive element comprising an active indicator (background), an icon,  and a label.
 *
 * @note **Layout Constraint:** By design, this component is "width-unaware" and
 * fills all available X-space. This is a **strict requirement** to force usage
 * inside a `ChildWindow` (like a Navigation Rail or Drawer), preventing
 * non-compliant full-screen-width navigation items.
 *
 * @param label     The text label displayed below or beside the icon.
 * @param selected  Whether the item is in the active state, affecting colors and indicator visibility.
 * @param icon      The icon glyph (typically a UTF-8 string literal from an icon font).
 * @param m3Styles  Reference to @c ImGuiEx::M3Styles for theme and scaling.
 * @return true if the item was clicked; otherwise false.
 */
auto NavItem(std::string_view label, bool selected, std::string_view icon) -> bool;

auto BeginNavRail(std::string_view strId, bool expanded) -> bool;

auto BeginNavRail(std::string_view strId) -> bool;

//! Like ImGui::EndChild: You always need to call it after BeginNavRail.
auto EndNavRail() -> void;

//! @brief Icon only, no interaction.
//! This is the Standard `IconButton` special case: no bg and interaction, only render the icon with correct size and color.
auto Icon(const std::string_view icon, const Spec::SizeTips sizeTips) -> void;

inline auto XSmallIcon(std::string_view icon) -> void
{
    Icon(icon, Spec::SizeTips::XSMALL);
}

inline auto SmallIcon(std::string_view icon) -> void
{
    Icon(icon, Spec::SizeTips::SMALL);
}

inline auto MediumIcon(std::string_view icon) -> void
{
    Icon(icon, Spec::SizeTips::MEDIUM);
}

inline auto LargeIcon(std::string_view icon) -> void
{
    Icon(icon, Spec::SizeTips::LARGE);
}

inline auto XLargeIcon(std::string_view icon) -> void
{
    Icon(icon, Spec::SizeTips::XLARGE);
}

//! @brief IconButtons
//! Pass `Spec::IconButtonColors` to specify the color scheme for the icon button.
//! The default is `Filled`, which applies the standard filled style.
auto IconButton(
    std::string_view icon, Spec::SizeTips sizeTips, Spec::IconButtonColorsValues colors,
    Spec::IconButtonWidths widths = Spec::IconButtonWidths::Default
) -> bool;

inline auto IconButton(
    std::string_view icon, Spec::SizeTips sizeTips, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled,
    Spec::IconButtonWidths widths = Spec::IconButtonWidths::Default
) -> bool
{
    return IconButton(icon, sizeTips, Spec::GetIconButtonColorsValues(ibColors), widths);
}

inline auto XSmallIconButton(std::string_view icon, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::XSMALL, ibColors, Spec::IconButtonWidths::Default);
}

inline auto SmallIconButton(std::string_view icon, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::SMALL, ibColors, Spec::IconButtonWidths::Default);
}

inline auto MediumIconButton(std::string_view icon, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::MEDIUM, ibColors, Spec::IconButtonWidths::Default);
}

inline auto LargeIconButton(std::string_view icon, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::LARGE, ibColors, Spec::IconButtonWidths::Default);
}

inline auto XLargeIconButton(std::string_view icon, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::XLARGE, ibColors, Spec::IconButtonWidths::Default);
}

auto Fab(std::string_view icon, Spec::SizeTips sizeTips, Spec::FabColors fabColors) -> bool;

inline auto Fab(std::string_view icon, Spec::FabColors fabColors) -> bool
{
    return Fab(icon, Spec::SizeTips::MEDIUM, fabColors);
}

inline auto FabLarge(std::string_view icon, Spec::FabColors fabColors) -> bool
{
    return Fab(icon, Spec::SizeTips::LARGE, fabColors);
}

struct ButtonConfiguration : BaseConfiguration<ButtonConfiguration>
{
    using BaseConfiguration::BaseConfiguration;

    std::string_view   icon;                                  ///< optional
    Spec::ButtonColors colors   = Spec::ButtonColors::filled; ///< default to filled
    Spec::ButtonShape  shape    = Spec::ButtonShape::Round;   ///< default to round
    bool               toggle   = false;                      ///< default to false, set true for toggle varint
    bool               selected = false;                      ///< default to false, only used for toggle varint, set true for selected state

    // allow implict conversionn for ImGuiEx::icon.
    ButtonConfiguration(std::string_view a_icon = "") : icon(a_icon) {} // NOLINT(*-explicit-constructor)

    constexpr auto Icon(std::string_view i) -> ButtonConfiguration &
    {
        icon = i;
        return *this;
    }

    constexpr auto Variant(Spec::ButtonColors v) -> ButtonConfiguration &
    {
        colors = v;
        return *this;
    }

    constexpr auto Shape(Spec::ButtonShape a_shape) -> ButtonConfiguration &
    {
        shape = a_shape;
        return *this;
    }

    constexpr auto Elevated() { return Variant(Spec::ButtonColors::elevated); }

    constexpr auto Filled() { return Variant(Spec::ButtonColors::filled); }

    constexpr auto Tonal() { return Variant(Spec::ButtonColors::tonal); }

    constexpr auto Outlined() { return Variant(Spec::ButtonColors::outlined); }

    constexpr auto Text() { return Variant(Spec::ButtonColors::text); }

    constexpr auto Round() { return Shape(Spec::ButtonShape::Round); }

    constexpr auto Square() { return Shape(Spec::ButtonShape::Square); }
};

auto Button(std::string_view label, const ButtonConfiguration &config = {}) -> bool;

inline auto Button(std::string_view label, std::string_view icon, Spec::SizeTips sizeTips) -> bool
{
    return Button(label, ButtonConfiguration(icon).Size(sizeTips));
}

inline auto XSmallButton(std::string_view label, std::string_view icon) -> bool
{
    return Button(label, icon, Spec::SizeTips::XSMALL);
}

inline auto SmallButton(std::string_view label, std::string_view icon) -> bool
{
    return Button(label, icon, Spec::SizeTips::SMALL);
}

inline auto MediumButton(std::string_view label, std::string_view icon) -> bool
{
    return Button(label, icon, Spec::SizeTips::MEDIUM);
}

inline auto LargeButton(std::string_view label, std::string_view icon) -> bool
{
    return Button(label, icon, Spec::SizeTips::LARGE);
}

inline auto XLargeButton(std::string_view label, std::string_view icon) -> bool
{
    return Button(label, icon, Spec::SizeTips::XLARGE);
}

struct ChipConfiguration
{
    std::string_view icon;                                ///< optional.
    std::string_view trailingIcon;                        ///< optional. should be used for filter chip.
    Spec::ChipColors colors   = Spec::ChipColors::Assist; ///< default to Assist
    bool             selected = false;                    ///< used for filter chips.
};

/**
 * @brief Material Design 3 Chip component.
 *
 * Similar to Button, but with a different set of styling rules and interaction states defined by the [M3
 * specification](https://m3.material.io/components/chips/specs). Chip: no size configuration, no container color for default state.
 *
 * It's suitable for secondary actions, e.g. tags, categories, or filters. It can be used in groups or standalone, and supports optional leading and
 * trailing icons.
 *
 * @param label required text label for the chip.
 * @param config optional configuration struct to customize the chip's appearance and behavior.
 * @return true if the chip was clicked, false otherwise.
 */
auto Chip(std::string_view label, const ChipConfiguration &config = {}) -> bool;

//! @brief A chips `ItemSpacing` style wrapper.
void BeginChipGroup();
void EndChipGroup();

/**
 * @brief Configuration for Material Design 3 text fields.
 */
struct TextFieldConfiguration
{
    //! Optional icon glyph rendered before the text field's label or input text.
    std::string_view leadingIcon;
    //! Optional icon glyph rendered after the text field's label or input text.
    std::string_view trailingIcon;
};

/**
 * @brief A filled text field that follows Material Design 3 specifications.
 *
 * Implemented by ImGui::TempInputText and custom Item style.
 * @return true if edited, false otherwise.
 */
auto FilledTextField(std::string_view label, char *buffer, size_t bufferSize, const TextFieldConfiguration &config = {}) -> bool;

//! @brief Overload for read-only or pre-populated text fields. The presence of inputText determines if the field is
//! "populated".
auto FilledTextField(std::string_view label, std::string_view inputText, const TextFieldConfiguration &config = {}) -> bool;

//! @brief An outlined text field that follows Material Design 3 specifications.
auto OutlinedTextField(std::string_view label, char *buffer, size_t bufferSize, const TextFieldConfiguration &config = {}) -> bool;

//! @brief Overload for read-only or pre-populated outlined text fields. The presence of inputText determines if the
//! field is "populated".
auto OutlinedTextField(std::string_view label, std::string_view inputText, const TextFieldConfiguration &config = {}) -> bool;

using Func = std::function<void()>;

/**
 * @brief Renders a Material Design 3 list item with a scoped layout and custom content.
 *
 * Combines background rendering, interaction handling, and content scoping into a single call.
 * It automatically manages state layers (Hover/Pressed) based on M3 specifications.
 *
 * @param m3Styles Theme configuration and layout tokens.
 * @param contentFunc Lambda/Callable for inner content. Note: Content is vertically centered
 * based on m3Styles.GetPadding<Spec::List>().
 * @param plain set true to ignore hovered staus.
 *
 * @note
 * - **Clipping**: Content is strictly clipped to the M3-defined height.
 * - **Layout**: Internally uses `BeginGroup` and `SetCursorScreenPos` to reset the local
 * coordinate system, ensuring `contentFunc` starts at the correctly padded origin.
 * - **Interaction**: The entire rectangular area (including padding) acts as a single
 * ButtonBehavior hotlink.
 *
 * @see M3Styles for available styling options.
 * @see Spec namespace for Material Design 3 specifications.
 */
void ListItem(Func &&contentFunc, bool plain = false);

inline void ListItemPlain(Func &&contentFunc)
{
    ListItem(std::forward<Func>(contentFunc), true);
}

//! Render a single-line label aligned to the current line’s text baseline.
//! Works inside ListItem content and also for any line where you want centered text alignment.
//! Requires calling `m3Styles.UseTextRole<Spec::List::textRole>()` (or an equivalent role) beforehand.
void AlignedLabel(std::string_view label, Spec::ColorRole contentRole = Spec::ColorRole::onSurface);

//! adjust cursor position.y for the leading color button in ListItem.
void ListLayoutLeadingColorButton(float height = 0.F);

struct ListScope
{
    bool       visible;
    StyleGuard styleGuard;

    explicit operator bool() const { return visible; }
};

[[nodiscard]] auto BeginList(const M3Styles &m3Styles, float width = 0.F, ChildFlags childFlags = {}) -> ListScope;

inline auto EndList()
{
    ImGui::EndChild();
}

/**
 * @brief A divider component that follows Material Design 3 specifications.
 *
 * It's the same as ImGui::Separator but has a layout height (Separator will not affect layout height) and color defined by the M3 spec.
 */
void Divider();

inline auto ListLeadingImageSize() -> ImVec2
{
    const auto size = Context::GetM3Styles().GetPixels(Spec::List::leadingImageSize);
    return {size, size};
}

struct SearchConfiguration
{
    std::string_view icon         = ""; ///< optional
    std::string_view hintText     = ""; ///< optional
    std::string_view trailingIcon = ""; ///< optional
};

auto SearchBar(std::string_view strId, char *buffer, size_t bufferSize, const SearchConfiguration &config) -> bool;

/**
 * The toolbar is a container with multiple slots, and you must
 * provide your expected dimensions and draw your button according to the agreement.
 * @return true is Toolbar visible
 */
auto BeginDockedToolbar(const ImVec2 &buttonSize, uint8_t count, Spec::ColorRole surfaceRole) -> bool;

inline auto BeginDockedToolbar(float buttonSize, uint8_t count, Spec::ColorRole surfaceRole) -> bool
{
    return BeginDockedToolbar(ImVec2{buttonSize, buttonSize}, count, surfaceRole);
}

auto EndDockedToolbar() -> void;

/**
 * @brief A floating Toolbar
 * A window `Begin/End` wrapepr.
 *
 * @note Toolbar will limit its position to stay within the screen with a margin defined by `Spec::ToolBarSizing::VerticalContainerExternalSpace` and
 * `Spec::ToolBarSizing::HorizontalContainerExternalSpace`.
 */
auto BeginFloatingToolbar(const char *name, bool *p_open, Spec::ToolBarColors colors = Spec::ToolBarColors::Standard, WindowFlags flags = {}) -> bool;
auto EndFloatingToolbar() -> void;

auto MenuItem(std::string_view label, const bool selected, Spec::MenuColors menuitemColors) -> bool;

inline auto MenuItem(std::string_view label, const bool selected) -> bool
{
    return MenuItem(label, selected, Spec::MenuColors::Standard);
}

inline auto MenuItemVibrant(std::string_view label, const bool selected) -> bool
{
    return MenuItem(label, selected, Spec::MenuColors::Vibrant);
}

constexpr int32_t SMALL_MAX_MENU_ITEM_COUNT  = 4;
constexpr int32_t MEDIUM_MAX_MENU_ITEM_COUNT = 8;
constexpr int32_t LARGE_MAX_MENU_ITEM_COUNT  = 16;

/**
 * @brief M3-styled Menu popup (Internal wrapper of ImGui::BeginComboPopup).
 * @note **Concept:** This follows the Material Design 3 Menu spec, not the
 * traditional ImGui/Windows menu system. It provides a modern popup surface.
 * @note **Implementation:** Reuses `BeginComboPopup` for stable positioning logic.
 *
 * @param strId      Popup ID.
 * @param m3Styles   M3 style configuration.
 * @param menuitemColors  Color scheme for menu items, affecting background.
 * @param maxItemCount Optional limit for visible items before scrolling. Pass a negative value for no limit.
 * @return true if the menu is open and ready for item rendering; false if the popup is closed or failed to open.
 */
auto BeginMenu(std::string_view strId, Spec::MenuColors menuitemColors, int32_t maxItemCount = SMALL_MAX_MENU_ITEM_COUNT) -> bool;

inline auto BeginMenu(std::string_view strId, int32_t maxItemCount = SMALL_MAX_MENU_ITEM_COUNT) -> bool
{
    return BeginMenu(strId, Spec::MenuColors::Standard, maxItemCount);
}

inline auto BeginMenuVibrant(std::string_view strId, int32_t maxItemCount = SMALL_MAX_MENU_ITEM_COUNT) -> bool
{
    return BeginMenu(strId, Spec::MenuColors::Vibrant, maxItemCount);
}

void EndMenu();

auto BeginCombo(std::string_view label, std::string_view previewValue) -> bool;
void EndCombo();

void SetItemToolTip(std::string_view text);

/**
 *  @brief Scoped AppBar component that follows Material Design 3 specifications.
 *
 * @important You must call `LeadingIcon`, `Title`, or `TrailingIcon` in sequence to populate the AppBar's content.
 * The layout will adjust dynamically based on which elements are present, following M3 guidelines.
 * Otherwise，the layout will almost certainly look broken!
 */
class AppBarScope
{
    ImVec2              m_minPos; ///< The bounding rect left top position, used for auto layout and interaction.
    // The next line cursorPos. Be used to reset cursor pos when scope exit.
    // Because we must change the "window->Dc.CursorPos" in the scope to achieve the layout defined by M3 spec,
    // but we don't want to affect the content after the scope, so we need to reset the cursor pos when scope exit.
    ImVec2              m_nextLineCursorPos;
    float               m_paddingX{0.F}; ///< The AppBar leading/trailing space. Already merged into the single field.
    Spec::AppBarVariant m_variant{Spec::AppBarVariant::Small};
    bool                m_visible = false;

public:
    explicit AppBarScope(Spec::AppBarVariant variant = Spec::AppBarVariant::Small);
    ~AppBarScope();
    AppBarScope(const AppBarScope &other)                     = delete;
    auto operator=(const AppBarScope &other) -> AppBarScope & = delete;

    AppBarScope(AppBarScope &&other) noexcept { *this = std::move(other); }

    auto operator=(AppBarScope &&other) noexcept -> AppBarScope &
    {
        m_minPos            = std::exchange(other.m_minPos, {});
        m_nextLineCursorPos = std::exchange(other.m_nextLineCursorPos, {});
        m_paddingX          = other.m_paddingX;
        m_variant           = other.m_variant;
        m_visible           = std::exchange(other.m_visible, false);
        return *this;
    }

    auto LeadingIcon(std::string_view icon) const -> bool; // NOLINT(*-use-nodiscard)

    auto Title(std::string_view title, std::string_view subTitle = "") -> void;

    //! Only be called once.
    auto TrailingIcon(std::string_view icon) const -> bool; // NOLINT(*-use-nodiscard)

    explicit operator bool() const { return m_visible; }
};

/**
 * @brief Constructs a Material Design 3 AppBar with the specified variant.
 * @param variant default to `Small`. Determines the AppBar's height and layout according to M3 specifications.
 * @return The AppBarScope object manages the AppBar's lifecycle, including rendering and interaction handling. It provides methods to set
 * leading/trailing icons and title text.
 * @note The AppBarScope must be used in a scoped manner (e.g., within a block) to ensure proper resource management. The AppBar will only be rendered
 * if the returned scope is valid (i.e., the AppBar is visible based on internal logic, such as screen width constraints).
 */
[[nodiscard]] inline auto AppBar(Spec::AppBarVariant variant = Spec::AppBarVariant::Small) -> AppBarScope
{
    return AppBarScope(variant);
}

} // namespace ImGuiEx::M3
