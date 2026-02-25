//
// Created by jamie on 2026/1/22.
//

#pragma once

#include "ImGuiEx.h"
#include "Material3.h"
#include "imguiex_enum_wrap.h"
#include "m3/facade/icon_button.h"
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

//! Support line height with last used TextRole's line height. If the line height is not set or less than font size, it
//! will fall back to normal text rendering.
void TextUnformatted(const std::string_view &text, const M3Styles &m3Styles, Spec::ColorRole contentRole = Spec::ColorRole::onSurface);

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
auto NavItem(std::string_view label, bool selected, std::string_view icon, const M3Styles &m3Styles) -> bool;

auto BeginNavRail(std::string_view strId, const M3Styles &m3Styles, bool expanded) -> bool;

auto BeginNavRail(std::string_view strId, const M3Styles &m3Styles) -> bool;

//! Like ImGui::EndChild: You always need to call it after BeginNavRail.
auto EndNavRail() -> void;

//! @brief Icon only, no interaction.
//! This is the Standard `IconButton` special case: no bg and interaction, only render the icon with correct size and color.
auto Icon(const std::string_view icon, const Spec::SizeTips sizeTips, const M3Styles &m3Styles) -> void;

inline auto XSmallIcon(std::string_view icon, const M3Styles &m3Styles) -> void
{
    Icon(icon, Spec::SizeTips::XSMALL, m3Styles);
}

inline auto SmallIcon(std::string_view icon, const M3Styles &m3Styles) -> void
{
    Icon(icon, Spec::SizeTips::SMALL, m3Styles);
}

inline auto MediumIcon(std::string_view icon, const M3Styles &m3Styles) -> void
{
    Icon(icon, Spec::SizeTips::MEDIUM, m3Styles);
}

inline auto LargeIcon(std::string_view icon, const M3Styles &m3Styles) -> void
{
    Icon(icon, Spec::SizeTips::LARGE, m3Styles);
}

inline auto XLargeIcon(std::string_view icon, const M3Styles &m3Styles) -> void
{
    Icon(icon, Spec::SizeTips::XLARGE, m3Styles);
}

//! @brief IconButtons
//! Pass `Spec::IconButtonColors` to specify the color scheme for the icon button.
//! The default is `Filled`, which applies the standard filled style.
auto IconButton(
    std::string_view icon, Spec::SizeTips sizeTips, Spec::IconButtonWidths widths, const M3Styles &m3Styles,
    Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled
) -> bool;

inline auto XSmallIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::XSMALL, Spec::IconButtonWidths::Default, m3Styles, ibColors);
}

inline auto SmallIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::SMALL, Spec::IconButtonWidths::Default, m3Styles, ibColors);
}

inline auto MediumIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::MEDIUM, Spec::IconButtonWidths::Default, m3Styles, ibColors);
}

inline auto LargeIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled) -> bool
{
    return IconButton(icon, Spec::SizeTips::LARGE, Spec::IconButtonWidths::Default, m3Styles, ibColors);
}

inline auto XLargeIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::XLARGE, Spec::IconButtonWidths::Default, m3Styles, ibColors);
}

inline auto XSmallNarrowIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::XSMALL, Spec::IconButtonWidths::Narrow, m3Styles, ibColors);
}

inline auto SmallNarrowIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::SMALL, Spec::IconButtonWidths::Narrow, m3Styles, ibColors);
}

inline auto MediumNarrowIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::MEDIUM, Spec::IconButtonWidths::Narrow, m3Styles, ibColors);
}

inline auto LargeNarrowIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::LARGE, Spec::IconButtonWidths::Narrow, m3Styles, ibColors);
}

inline auto XLargeNarrowIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::XLARGE, Spec::IconButtonWidths::Narrow, m3Styles, ibColors);
}

inline auto XSmallWideIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::XSMALL, Spec::IconButtonWidths::Wide, m3Styles, ibColors);
}

inline auto SmallWideIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::SMALL, Spec::IconButtonWidths::Wide, m3Styles, ibColors);
}

inline auto MediumWideIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::MEDIUM, Spec::IconButtonWidths::Wide, m3Styles, ibColors);
}

inline auto LargeWideIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::LARGE, Spec::IconButtonWidths::Wide, m3Styles, ibColors);
}

inline auto XLargeWideIconButton(std::string_view icon, const M3Styles &m3Styles, Spec::IconButtonColors ibColors = Spec::IconButtonColors::Filled)
    -> bool
{
    return IconButton(icon, Spec::SizeTips::XLARGE, Spec::IconButtonWidths::Wide, m3Styles, ibColors);
}

auto Fab(std::string_view icon, Spec::SizeTips sizeTips, const M3Styles &m3Styles, Spec::FabColors fabColors) -> bool;

inline auto Fab(std::string_view icon, const M3Styles &m3Styles, Spec::FabColors fabColors) -> bool
{
    return Fab(icon, Spec::SizeTips::MEDIUM, m3Styles, fabColors);
}

inline auto FabLarge(std::string_view icon, const M3Styles &m3Styles, Spec::FabColors fabColors) -> bool
{
    return Fab(icon, Spec::SizeTips::LARGE, m3Styles, fabColors);
}

auto Button(std::string_view label, std::string_view icon, Spec::SizeTips sizeTips, const M3Styles &m3Styles) -> bool;

inline auto XSmallButton(std::string_view label, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    return Button(label, icon, Spec::SizeTips::XSMALL, m3Styles);
}

inline auto SmallButton(std::string_view label, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    return Button(label, icon, Spec::SizeTips::SMALL, m3Styles);
}

inline auto MediumButton(std::string_view label, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    return Button(label, icon, Spec::SizeTips::MEDIUM, m3Styles);
}

inline auto LargeButton(std::string_view label, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    return Button(label, icon, Spec::SizeTips::LARGE, m3Styles);
}

inline auto XLargeButton(std::string_view label, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    return Button(label, icon, Spec::SizeTips::XLARGE, m3Styles);
}

/**
 * @brief Content model for Material Design 3 text fields.
 *
 * Encapsulates the label and optional leading/trailing icon glyphs used when
 * rendering filled or outlined text fields in the M3 style helpers.
 */
struct TextFieldContent
{
    //! Optional icon glyph rendered before the text field's label or input text.
    std::string_view leadingIcon;
    //! The label text associated with the text field.
    std::string_view label;
    //! Optional icon glyph rendered after the text field's label or input text.
    std::string_view trailingIcon;
};

/**
 * @brief A filled text field that follows Material Design 3 specifications.
 *
 * Implemented by ImGui::TempInputText and custom Item style.
 * @return true if edited, false otherwise.
 */
auto FilledTextField(const TextFieldContent &tfContent, char *buffer, size_t bufferSize, const M3Styles &m3Styles) -> bool;

//! @brief Overload for read-only or pre-populated text fields. The presence of inputText determines if the field is
//! "populated".
auto FilledTextField(const TextFieldContent &tfContent, std::string_view inputText, const M3Styles &m3Styles) -> bool;

//! @brief An outlined text field that follows Material Design 3 specifications.
auto OutlinedTextField(const TextFieldContent &tfContent, char *buffer, size_t bufferSize, const M3Styles &m3Styles) -> bool;

//! @brief Overload for read-only or pre-populated outlined text fields. The presence of inputText determines if the
//! field is "populated".
auto OutlinedTextField(const TextFieldContent &tfContent, std::string_view inputText, const M3Styles &m3Styles) -> bool;

using Func = std::function<void()>;

/**
 * @brief Renders a Material Design 3 list item with a scoped layout and custom content.
 *
 * Combines background rendering, interaction handling, and content scoping into a single call.
 * It automatically manages state layers (Hover/Pressed) based on M3 specifications.
 *
 * @param strId Unique identifier for ImGui state management.
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
void ListItem(std::string_view strId, const M3Styles &m3Styles, Func &&contentFunc, bool plain = false);

inline void ListItemPlain(std::string_view strId, const M3Styles &m3Styles, Func &&contentFunc)
{
    ListItem(strId, m3Styles, std::forward<Func>(contentFunc), true);
}

//! Render a single-line label aligned to the current line’s text baseline.
//! Works inside ListItem content and also for any line where you want centered text alignment.
//! Requires calling `m3Styles.UseTextRole<Spec::List::textRole>()` (or an equivalent role) beforehand.
void AlignedLabel(std::string_view label, const M3Styles &m3Styles, Spec::ColorRole contentRole = Spec::ColorRole::onSurface);

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

inline void ListDivider()
{
    ImGui::Separator();
}

inline auto ListLeadingImageSize(const M3Styles &m3Styles) -> ImVec2
{
    const auto size = m3Styles.GetPixels(Spec::List::leadingImageSize);
    return {size, size};
}

/**
 * The toolbar is a container with multiple slots, and you must
 * provide your expected dimensions and draw your button according to the agreement.
 * @return true is Toolbar visible
 */
auto BeginDockedToolbar(const ImVec2 &buttonSize, uint8_t count, Spec::ColorRole surfaceRole, const M3Styles &m3Styles) -> bool;

inline auto BeginDockedToolbar(float buttonSize, uint8_t count, Spec::ColorRole surfaceRole, const M3Styles &m3Styles) -> bool
{
    return BeginDockedToolbar(ImVec2{buttonSize, buttonSize}, count, surfaceRole, m3Styles);
}

auto EndDockedToolbar() -> void;

/**
 * @brief A floating Toolbar
 * A window `Begin/End` wrapepr.
 */
auto BeginFloatingToolbar(
    const char *name, bool *p_open, const M3Styles &m3Styles, Spec::ToolBarColors colors = Spec::ToolBarColors::Standard, WindowFlags flags = {}
) -> bool;
auto EndFloatingToolbar() -> void;

auto MenuItem(std::string_view label, const bool selected, Spec::MenuColors menuitemColors, const M3Styles &m3Styles) -> bool;

inline auto MenuItem(std::string_view label, const bool selected, const M3Styles &m3Styles) -> bool
{
    return MenuItem(label, selected, Spec::MenuColors::Standard, m3Styles);
}

inline auto MenuItemVibrant(std::string_view label, const bool selected, const M3Styles &m3Styles) -> bool
{
    return MenuItem(label, selected, Spec::MenuColors::Vibrant, m3Styles);
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
auto BeginMenu(std::string_view strId, const M3Styles &m3Styles, Spec::MenuColors menuitemColors, int32_t maxItemCount = SMALL_MAX_MENU_ITEM_COUNT)
    -> bool;

inline auto BeginMenu(std::string_view strId, const M3Styles &m3Styles, int32_t maxItemCount = SMALL_MAX_MENU_ITEM_COUNT) -> bool
{
    return BeginMenu(strId, m3Styles, Spec::MenuColors::Standard, maxItemCount);
}

inline auto BeginMenuVibrant(std::string_view strId, const M3Styles &m3Styles, int32_t maxItemCount = SMALL_MAX_MENU_ITEM_COUNT) -> bool
{
    return BeginMenu(strId, m3Styles, Spec::MenuColors::Vibrant, maxItemCount);
}

void EndMenu();

auto BeginCombo(std::string_view label, std::string_view previewValue, const M3Styles &m3Styles) -> bool;
void EndCombo();

void SetItemToolTip(std::string_view text, const M3Styles &m3Styles);

} // namespace ImGuiEx::M3
