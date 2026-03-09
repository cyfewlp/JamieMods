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
#include "m3/spec/layout.h"
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

/**
 * @brief Renders unformatted text using Material Design 3 typography and color roles.
 *
 * Unlike @c ImGui::TextUnformatted, this function integrates with @c M3Styles to
 * apply MD3-specific styling, cached type scales, and optional text wrapping.
 *
 * @pre **IMPORTANT:** You must call @c M3Styles::UseTextRole() before this function
 * to initialize the appropriate font and line height metrics.
 *
 * @note **Vertical Centering Logic:**
 * Instead of using the global ImGui font size, this function utilizes cached MD3
 * metrics (line height, text size, and half-line gap). This allows the text to be
 * **vertically centered by default** within its layout slot without manual offset
 * calculations.
 *
 * @note **Modern C++ Recommendation:**
 * C-style variadic formatting is intentionally unsupported. For efficiency and
 * type safety, use @c std::format or @c fmt::format before passing the string.
 *
 * @param text        The text to render (@c std::string_view for zero-copy handling).
 * @param contentRole [Optional] The MD3 color role. Defaults to current context style.
 * @param wrapWidth   [Optional] Wrap threshold in pixels. Negative values disable wrapping.
 */
void TextUnformatted(std::string_view text, Spec::ColorRole contentRole = Spec::ColorRole::onSurface, const float wrapWidth = -1.0F);

template <Spec::TextRole Role>
void TextUnformatted(const std::string_view &text, Spec::ColorRole contentRole = Spec::ColorRole::onSurface, const float wrapWidth = -1.0F)
{
    auto &m3Styles = Context::GetM3Styles();

    const auto fontScope = m3Styles.UseTextRole(Spec::TypeScaleValue::of<Role>());
    TextUnformatted(text, contentRole, wrapWidth);
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

/**
 * @brief Begin a Navigation Rail container.
 * @param strId The ID for the Navigation Rail, used for ImGui's ID stack and should be unique within the scope of the UI.
 * @param expanded true to start in expanded state (showing both icons and labels), false to start in collapsed state (showing only icons).
 * @return true if the Navigation Rail container is successfully created and should be rendered; otherwise false. Always call EndNavRail() if this
 * returns true.
 */
auto BeginNavRail(std::string_view strId, bool expanded = false) -> bool;

/**
 * @copydoc BeginNavRail(std::string_view, bool)
 *
 * @param minWidthToExpand The unscaled width threshold (in dp) at which the Navigation Rail should switch between collapsed and expanded states. If
 * the available width is greater than this value, the Navigation Rail will be in expanded state; otherwise, it will be collapsed.
 */
auto BeginResponsiveNavRail(std::string_view strId, float minWidthToExpand = Spec::Layout::ExtraLarge::Breakpoint) -> bool;

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
 * @brief Docked Toolbar component following Material Design 3 specifications.
 * Implemented by ImGui Table API and custom Item style.
 *
 * FIXME-OPT: the Icon is left-aligned, how to center align?
 */
class DockedToolbarScope
{
    bool m_visible{false};

public:
    explicit DockedToolbarScope(std::string_view strId, uint8_t count, const Spec::ToolBarColors colors = Spec::ToolBarColors::Standard);

    ~DockedToolbarScope();

    auto Icon(std::string_view icon) const -> bool;

    explicit operator bool() const { return m_visible; }
};

/**
 * @brief The toolbar is a container with multiple slots, and you must
 * provide your expected dimensions and draw your button according to the agreement.
 * @return true is Toolbar visible
 */
inline auto DockedToolBar(std::string_view strId, uint8_t count, const Spec::ToolBarColors colors = Spec::ToolBarColors::Standard)
    -> DockedToolbarScope
{
    return DockedToolbarScope(strId, count, colors);
}

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
 * @brief Scoped AppBar component following Material Design 3 specifications.
 *
 * Lifecycle is managed via RAII: the constructor reserves layout space and begins the
 * internal draw group;
 *
 * @note Intended call order within the scope: `LeadingIcon` → `Title` → `TrailingIcon`
 * (one or more). Calling out of order will almost certainly produce broken layout.
 */
class AppBarScope
{
    //! Top-left corner of the AppBar bounding rect.
    ImVec2              m_minPos;
    //! Pixel width of one icon layout slot (`IconButtonCommon::MinLayoutSize`).
    float               m_iconLayoutWidth{0.F};
    //! X position for the *next* trailing icon to be placed.
    //! Initialised to `bb.Max.x - m_paddingX` and decremented by `m_iconLayoutWidth`
    //! before each `TrailingIcon` call, so icons are stacked right-to-left.
    float               m_trailingIconCursorPosX{0.F};
    //! Rightmost X reached by leading-side content (`LeadingIcon` and `Title`).
    //! `TrailingIcon` uses this to guard against overlap: it clamps its own cursor to
    //! `max(m_iconLineItemMaxPosX + m_paddingX, m_trailingIconCursorPosX)`.
    float               m_iconLineItemMaxPosX{0.F};
    //! Leading/trailing horizontal padding of the AppBar container (mapped from
    //! `AppBarCommon::LeadingSpace`). Shared between leading and trailing sides.
    float               m_paddingX{0.F};
    Spec::AppBarVariant m_variant{Spec::AppBarVariant::Small};
    bool                m_visible{false};

public:
    explicit AppBarScope(Spec::AppBarVariant variant = Spec::AppBarVariant::Small);
    //! Calls `ImGui::EndGroup`, which restores the cursor/line state saved by `BeginGroup` in the constructor
    //! and terminates any active `SameLine` mode left by the last `TrailingIcon` call.
    //! Because `GroupStack.back().EmitItem` is set to `false` in the constructor, `EndGroup` does **not**
    //! submit the group bounding box as a layout item — layout space was already reserved by the `ItemSize`
    //! call made before `BeginGroup`.
    ~AppBarScope();
    AppBarScope(const AppBarScope &)                     = delete;
    auto operator=(const AppBarScope &) -> AppBarScope & = delete;

    AppBarScope(AppBarScope &&other) noexcept { *this = std::move(other); }

    auto operator=(AppBarScope &&other) noexcept -> AppBarScope &
    {
        m_minPos                 = std::exchange(other.m_minPos, {});
        m_iconLayoutWidth        = other.m_iconLayoutWidth;
        m_trailingIconCursorPosX = std::exchange(other.m_trailingIconCursorPosX, 0.F);
        m_iconLineItemMaxPosX    = std::exchange(other.m_iconLineItemMaxPosX, 0.F);
        m_paddingX               = other.m_paddingX;
        m_variant                = other.m_variant;
        m_visible                = std::exchange(other.m_visible, false);
        return *this;
    }

    //! Renders the leading icon using `AppBarCommon::LeadingIconColor`.
    //! Must be called before `Title`. Leaves the cursor immediately after the icon
    //! via `SameLine` so that `Title` is positioned relative to it.
    auto LeadingIcon(std::string_view icon) -> bool; // NOLINT(*-use-nodiscard)

    //! Renders the title (and optional subtitle) according to the active variant's
    //! typography and position spec. For `Small`, the cursor advances rightward for
    //! a potential `TrailingIcon`. For `MediumFlexible`/`LargeFlexible`, the title
    //! is placed in the lower section and the cursor is not advanced horizontally.
    auto Title(std::string_view title, std::string_view subTitle = "") -> void;

    //! Renders a trailing icon using `AppBarCommon::TrailingIconColor`.
    //! May be called multiple times; icons are stacked from right to left.
    //! Each call decrements the internal trailing cursor by one icon slot width.
    //! Emits `IM_ASSERT` if the trailing cursor would overlap the leading content.
    //! Each call except the conceptual "last" relies on `SameLine` to stay on the
    //! icon row; the scope destructor (`EndGroup`) resets the line state on exit.
    auto TrailingIcon(std::string_view icon) -> bool; // NOLINT(*-use-nodiscard)

    explicit operator bool() const { return m_visible; }
};

/**
 * @brief Constructs a Material Design 3 AppBar with the specified variant.
 *
 * @param variant Determines the AppBar's height and title layout. Defaults to `Small`.
 *   - `Small`: single icon row, title inline.
 *   - `MediumFlexible` / `LargeFlexible`: title placed in a taller lower section.
 * @return `AppBarScope` managing the AppBar's lifecycle. Use `operator bool` to check
 *   visibility before calling slot methods.
 */
[[nodiscard]] inline auto AppBar(Spec::AppBarVariant variant = Spec::AppBarVariant::Small) -> AppBarScope
{
    return AppBarScope(variant);
}

class DialogModalScope
{
    detail::FontScope m_supportingTextFontScope;
    bool              m_visible{false};
    bool              m_submittedBody{false};

public:
    explicit DialogModalScope(std::string_view name, WindowFlags flags = {});
    ~DialogModalScope();

    /**
     * @brief Render the supporting text for the dialog.
     * @param text the supporting text content, usually a description or instruction for the dialog.
     * @param wrapText true to enable text wrapping, false for single line. Default is false.
     */
    void SupportingText(std::string_view text, bool wrapText = false);

    /**
     * @brief Add a action button(g.g. apply/cancel) to the dialog.
     * REQUIRED! Diaglog no titlebar and popup name will render to HeadLine!
     * So, also no close button, you need to add a action button to
     * close the dialog.
     *
     * Multiple action button must call ImGui::SameLine by yourself.
     *
     * Action button will close the dialog when clicked, so you don't need to call `ImGui::CloseCurrentPopup()` in your logic, just handle the
     * business logic and return.
     *
     * @note The action buttons won't aligned-right, not supported yet.
     * @param label the button label, e.g. "Apply", "Cancel".
     */
    auto ActionButton(std::string_view label) const -> bool;

    explicit operator bool() const { return m_visible; }
};

inline auto DialogModal(std::string_view name, WindowFlags flags = {}) -> DialogModalScope
{
    return DialogModalScope(name, flags);
}

/**
 * @brief Setup the default imgui style colors by ColorScheme.
 *
 * This can be reduces some colors push/pop.
 */
void SetupDefaultImGuiStyles(ImGuiStyle &style);

} // namespace ImGuiEx::M3
