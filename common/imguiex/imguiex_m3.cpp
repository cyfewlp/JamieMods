//
// Created by jamie on 2026/1/24.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiex/imguiex_m3.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "m3/facade/buttons.h"
#include "m3/facade/fab.h"
#include "m3/facade/icon_button.h"
#include "m3/facade/menu.h"
#include "m3/facade/nav_rail.h"
#include "m3/facade/text_field.h"
#include "m3/spec/layout.h"
#include "m3/spec/search.h"
#include "m3/spec/text_field.h"
#include "m3/spec/tool_bar.h"

#include <cmath>

#if IMGUI_VERSION_NUM != 19263
    #error "ImGui version changed! imguiex_m3 only supports v1.92.6"
#endif

/**
 * @brief Stateless layout implementation relying on M3Styles.
 * * **Core Design:** This implementation is entirely stateless and does not maintain
 * its own context. It relies on an **implicit layout agreement**:
 * - All components use `window->DC.CurrLineSize.y` as their definitive line height.
 * - Components are automatically centered vertically within this line.
 * * **Example:** `BeginToolBar` emits a dummy item spanning the container's full
 * height to initialize the `CurrLineSize.y` for subsequent items.
 *
 * * @todo Support configurable alignment. Currently, vertical centering is hardcoded;
 * future versions should allow custom alignment strategies.
 */
namespace ImGuiEx::M3
{

namespace
{

inline auto IsItemDisabled() -> bool
{
    return (ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
}

template <std::same_as<Spec::Unit>... Units>
inline auto GetPixels(const M3Styles &m3Styles, Units... units) -> float
{
    float totalPixels = 0.0F;
    ((totalPixels += m3Styles.GetPixels(units)), ...);
    return totalPixels;
}

inline void DrawIcon(
    ImDrawList *drawList, float iconSize, const ImVec2 &iconPos, std::string_view icon, const M3Styles &m3Styles, const ImVec4 &iconColor
)
{
    drawList->AddText(m3Styles.IconFont(), iconSize, iconPos, ImGui::ColorConvertFloat4ToU32(iconColor), TextStart(icon), TextEnd(icon));
}

inline void DrawIcon(
    ImDrawList *drawList, float iconSize, const ImVec2 &iconPos, std::string_view icon, const M3Styles &m3Styles, const Spec::ColorRole contentRole
)
{
    DrawIcon(drawList, iconSize, iconPos, icon, m3Styles, m3Styles.Colors()[contentRole]);
}

inline void DrawText(ImDrawList *drawList, const ImVec2 &textPos, std::string_view text, const ImVec4 &contentColor)
{
    drawList->AddText(nullptr, 0.F, textPos, ImGui::ColorConvertFloat4ToU32(contentColor), TextStart(text), TextEnd(text));
}

inline void DrawText(ImDrawList *drawList, const ImVec2 &textPos, std::string_view text, const M3Styles &m3Styles, const Spec::ColorRole contentRole)
{
    DrawText(drawList, textPos, text, m3Styles.Colors()[contentRole]);
}

inline void RenderNavCursor(const ImRect &bb, ImGuiID id, float rounding, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None)
{
    auto &g = *GImGui;

    const auto backupRounding = g.Style.FrameRounding;
    g.Style.FrameRounding     = rounding;
    ImGui::RenderNavCursor(bb, id, flags);
    g.Style.FrameRounding = backupRounding;
}

inline auto GetDisplayLabel(std::string_view label) -> std::string_view
{
    const auto *end = ImGui::FindRenderedTextEnd(TextStart(label), TextEnd(label));
    return {TextStart(label), end};
}

//! @brief Align the cursor position to the center of the line height if the line height is larger than the content height.
inline auto GetAlignedCursorPos(const ImGuiWindow *window, const float height) -> ImVec2
{
    if (window->DC.CurrLineSize.y > height)
    {
        const auto offsetY = HalfDiff(window->DC.CurrLineSize.y, height);
        return {window->DC.CursorPos.x, window->DC.CursorPos.y + offsetY};
    }
    return window->DC.CursorPos;
}

inline auto GetButtonState(const ImRect &bb, ImGuiID id, Spec::States &states) -> bool
{
    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    if (IsItemDisabled())
    {
        states = Spec::States::Disabled;
    }
    else if (hovered && held)
    {
        states = Spec::States::Pressed;
    }
    else if (hovered)
    {
        states = Spec::States::Hovered;
    }
    else
    {
        states = Spec::States::Enabled;
    }
    return pressed;
}

} // namespace

void TextUnformatted(const std::string_view &text, const Spec::ColorRole contentRole)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    const ImGuiContext &g        = *GImGui;
    auto               &m3Styles = Context::GetM3Styles();

    const float wrap_pos_x   = window->DC.TextWrapPos;
    const bool  wrap_enabled = (wrap_pos_x >= 0.0F);
    if (text.size() <= 2000 || wrap_enabled)
    {
        // Common case
        const float       wrap_width = wrap_enabled ? ImGui::CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0F;
        const auto *const textEnd    = TextEnd(text);
        const ImVec2      text_size  = ImGui::CalcTextSize(TextStart(text), textEnd, false, wrap_width);

        const auto   halfGap = ImMax(0.0F, m3Styles.GetLastText().currHalfLineGap);
        const auto   size    = text_size + ImVec2(0.0F, halfGap * 2.0F);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ImGui::ItemSize(size, 0.0F);
        if (!ImGui::ItemAdd(bb, 0U)) return;

        // Render (we don't hide text after ## in this end-user function)
        if (!text.empty())
        {
            const auto pos = ImVec2{bb.Min.x, bb.Min.y + halfGap};
            window->DrawList->AddText(
                g.Font,
                m3Styles.GetLastText().currText.textSize,
                pos,
                ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[contentRole]),
                TextStart(text),
                textEnd,
                wrap_width
            );
            if (g.LogEnabled) ImGui::LogRenderedText(&pos, TextStart(text), textEnd);
        }
    }
    else
    {
        IM_ASSERT(false && "ImGuiEx::M3 not designed for long text! Use ImGui::TextWrapped instead.");
        ImGui::TextWrapped("%.*s", static_cast<int>(text.size()), TextStart(text));
    }
}

namespace
{

//! Internal used by NavItem when the nav rail is expanded.
auto HorizontalNavItem(std::string_view label, bool selected, std::string_view icon, M3Styles &m3Styles) -> bool
{
    using NavItemSpecH = Spec::NavRailItemHorizontal;

    ImGuiWindow *window        = ImGui::GetCurrentWindow();
    const auto   labelTextRole = m3Styles.UseTextRole<NavItemSpecH::LabelTextRole>();
    const auto   textSize      = ImGui::CalcTextSize(TextStart(label), TextEnd(label));
    const auto   width         = textSize.x + m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorMinWidthEx);
    const ImVec2 size(width, m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorHeight));

    window->DC.CursorPos.x += m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorOffsetXEx);
    const auto   id = ImGui::GetID(TextStart(label), TextEnd(label));
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ImGui::ItemSize(size, 0.0F);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }
    const auto &colors      = m3Styles.Colors();
    bool        hovered     = false;
    bool        held        = false;
    const bool  pressed     = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    const auto  bgColorRole = selected ? Spec::ExpandedNavRail::ActiveIndicatorColor : Spec::ExpandedNavRail::ContainerColor;
    if (selected || hovered || held)
    {
        const ImVec4 bgColor = colors.GetStateColor(bgColorRole, Spec::ExpandedNavRail::ActiveIconColor, hovered, held);
        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), false, size.y * HALF);
    }
    const ImVec2 iconPos =
        bb.Min + ImVec2(m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorLeadingSpace), m3Styles.GetPixels(NavItemSpecH::FullWidthLeadingSpace));
    const auto iconSize = m3Styles.GetPixels(NavItemSpecH::IconSize);
    const auto iconRole = selected ? Spec::ExpandedNavRail::ActiveIconColor : Spec::ExpandedNavRail::InactiveIconColor;
    DrawIcon(window->DrawList, iconSize, iconPos, icon, m3Styles, iconRole);
    const ImVec2 textPos(
        iconPos.x + iconSize + m3Styles.GetPixels(Spec::NavRailItemHorizontal::IconLabelSpace),
        bb.Min.y + HalfDiff(size.y, m3Styles.GetLastText().currText.textSize)
    );
    const auto &textRole = selected ? Spec::ExpandedNavRail::ActiveLabelTextColor : Spec::ExpandedNavRail::InactiveLabelTextColor;
    DrawText(window->DrawList, textPos, label, m3Styles, textRole);
    return pressed;
}
} // namespace

auto NavItem(std::string_view label, bool selected, std::string_view icon) -> bool
{
    using NavItemSpecV = Spec::NavRailItemVertical;

    const ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    auto      &m3Styles     = Context::GetM3Styles();
    const auto displayLabel = GetDisplayLabel(label);

    const auto availX   = ImGui::GetContentRegionAvail().x;
    const bool expanded = availX > m3Styles.GetPixels(Spec::CollapsedNavRail::ContainerWidth);
    if (expanded)
    {
        return HorizontalNavItem(displayLabel, selected, icon, m3Styles);
    }

    const float itemVerticalSpace = m3Styles.GetPixels(Spec::CollapsedNavRail::ItemVerticalSpace);

    const ImVec2 size(ImGui::GetContentRegionAvail().x, m3Styles.GetPixels(NavItemSpecV::ContainerHeight));
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const auto   id = ImGui::GetID(TextStart(label), TextEnd(label));
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    const ImVec2 ActiveIndicatorSize(m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorWidth), m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorHeight));

    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    const ImVec2 ActiveIndicatorOffset(m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorOffsetXEx), itemVerticalSpace);
    const ImVec2 ActiveIndicatorMin = bb.Min + ActiveIndicatorOffset;
    const ImVec2 ActiveIndicatorMax = ActiveIndicatorMin + ActiveIndicatorSize;

    const auto &colors = m3Styles.Colors();
    {
        const Spec::ColorRole bgColorRole = selected ? Spec::CollapsedNavRail::ActiveIndicatorColor : Spec::CollapsedNavRail::ContainerColor;
        // icon bg rect
        if (selected || hovered || held)
        {
            const ImVec4 bgColor = colors.GetStateColor(bgColorRole, Spec::ExpandedNavRail::ActiveIconColor, hovered, held);
            ImGui::RenderFrame(
                ActiveIndicatorMin,
                ActiveIndicatorMax,
                ImGui::ColorConvertFloat4ToU32(bgColor),
                false,
                size.y * HALF // full shape
            );
        }
        const ImVec2          iconPos = ActiveIndicatorMin + ImVec2(m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorLeadingSpace), itemVerticalSpace);
        const Spec::ColorRole iconRole = selected ? Spec::CollapsedNavRail::ActiveIconColor : Spec::CollapsedNavRail::InactiveIconColor;
        DrawIcon(drawList, m3Styles.GetPixels(NavItemSpecV::IconSize), iconPos, icon, m3Styles, iconRole);
    }

    const auto labelTextRole = m3Styles.UseTextRole<NavItemSpecV::LabelTextRole>();
    {
        const auto textSize = ImGui::CalcTextSize(TextStart(displayLabel), TextEnd(displayLabel));
        // align left when text is too long, avoid it looks weird when center aligned with ellipsis;
        ImVec2     labelPos(bb.Min.x, ActiveIndicatorMax.y + itemVerticalSpace + m3Styles.GetLastText().currHalfLineGap);
        if (textSize.x <= size.x)
        {
            labelPos.x = bb.Min.x + HalfDiff(size.x, textSize.x); // horizontal center align.
        }

        const auto &labelColorRole = selected ? Spec::CollapsedNavRail::ActiveLabelTextColor : Spec::CollapsedNavRail::InactiveLabelTextColor;
        DrawText(drawList, labelPos, displayLabel, m3Styles, labelColorRole);
    }
    return pressed;
}

auto BeginNavRail(std::string_view strId, const bool expanded) -> bool
{
    auto &m3Styles = Context::GetM3Styles();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[Spec::CollapsedNavRail::ContainerColor]));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.F, 0.F});
    auto       width   = m3Styles.GetPixels(Spec::CollapsedNavRail::ContainerWidth);
    bool       visible = false;
    ChildFlags flags{};
    if (expanded)
    {
        const auto minWidth = m3Styles.GetPixels(Spec::ExpandedNavRail::ContainerWidthMinimum);
        const auto maxWidth = m3Styles.GetPixels(Spec::ExpandedNavRail::ContainerWidthMaximum);
        width               = minWidth;
        ImGui::SetNextWindowSizeConstraints({minWidth, -FLT_MIN}, {maxWidth, -FLT_MIN});
        flags = flags.ResizeX();
    }
    visible = ImGui::BeginChild(TextStart(strId), {width, 0.0F}, flags);
    if (!visible)
    {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::Dummy({0.F, m3Styles.GetPixels(Spec::CollapsedNavRail::TopSpace)});
    }
    return visible;
}

auto BeginResponsiveNavRail(std::string_view strId, float minWidthToExpand) -> bool
{
    auto      &m3Styles = Context::GetM3Styles();
    const bool expanded = ImGui::GetContentRegionAvail().x > m3Styles.GetPixels(minWidthToExpand);
    return BeginNavRail(strId, expanded);
}

auto EndNavRail() -> void
{
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

namespace
{
template <Spec::TextFieldVariant Style>
auto DrawTextField(
    std::string_view label, Spec::TextFieldState state, const ImRect &bb, const TextFieldConfiguration &tfContent, M3Styles &m3Styles,
    const bool populated, ImRect &inputBB
) -> void
{
    ImGuiWindow *window            = ImGui::GetCurrentWindow();
    const auto   labelTextPaddingX = m3Styles.GetPixels(Spec::TextFieldCommon::LeadingLabelTextSpace);

    inputBB.Min = ImVec2(bb.Min.x + labelTextPaddingX, bb.Min.y);
    inputBB.Max = ImVec2(bb.Max.x - labelTextPaddingX, bb.Max.y);
    if constexpr (Style == Spec::TextFieldVariant::Filled)
    {
        ImVec4       labelTextColor     = m3Styles.Colors()[Spec::FilledTextFieldEnabled::LabelTextColor];
        ImVec4       indicatorColor     = m3Styles.Colors()[Spec::FilledTextFieldEnabled::ActiveIndicatorColor];
        ImVec4       iconColor          = m3Styles.Colors()[Spec::FilledTextFieldEnabled::LeadingIconColor];
        float        indicatorHeight    = m3Styles.GetPixels(Spec::FilledTextFieldEnabled::ActiveIndicatorHeight);
        float        indicatorThickness = m3Styles.GetPixels(Spec::FilledTextFieldEnabled::ActiveIndicatorThickness);
        ImVec4       bgColor            = m3Styles.Colors()[Spec::FilledTextFieldEnabled::ContainerColor];
        const ImVec4 inputTextColor     = m3Styles.Colors()[Spec::FilledTextFieldEnabled::InputTextColor];
        switch (state)
        {
            case Spec::TextFieldState::Hovered:
                indicatorColor = m3Styles.Colors()[Spec::FilledTextFieldHovered::ActiveIndicatorColor];
                bgColor        = ColorUtils::BlendHoveredOrMakeOverlay(bgColor, inputTextColor);
                break;
            case Spec::TextFieldState::Disabled: {
                bgColor          = m3Styles.Colors()[Spec::FilledTextFieldDisabled::ContainerColor];
                indicatorColor   = m3Styles.Colors()[Spec::FilledTextFieldDisabled::ActiveIndicatorColor];
                bgColor.w        = Spec::FilledTextFieldDisabled::ContainerOpacity;
                labelTextColor.w = DISABLED_CONTENT_OPACITY;
                indicatorColor.w = DISABLED_CONTENT_OPACITY;
                iconColor.w      = DISABLED_CONTENT_OPACITY;
                break;
            }
            case Spec::TextFieldState::Focused: {
                labelTextColor     = m3Styles.Colors()[Spec::FilledTextFieldFocused::LabelTextColor];
                indicatorColor     = m3Styles.Colors()[Spec::FilledTextFieldFocused::ActiveIndicatorColor];
                indicatorHeight    = m3Styles.GetPixels(Spec::FilledTextFieldFocused::ActiveIndicatorHeight);
                indicatorThickness = m3Styles.GetPixels(Spec::FilledTextFieldFocused::ActiveIndicatorThickness);
                break;
            }
            default:
                break;
        }
        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor));

        ImVec2 labelTextPos = bb.Min;

        // the leading/trailing icon's leading/trailing space always the same.
        const float iconPadding = m3Styles.GetPixels(Spec::TextFieldCommon::IconLeadingSpace);
        const float iconSize    = m3Styles.GetPixels(Spec::TextFieldCommon::LeadingIconSize);
        const float iconOffsetY = HalfDiff(bb.GetHeight(), m3Styles.GetPixels(Spec::TextFieldCommon::LeadingIconSize));
        if (!tfContent.leadingIcon.empty())
        {
            const ImVec2 iconPos = bb.Min + ImVec2(iconPadding, iconOffsetY);
            labelTextPos.x += iconPadding + iconSize;
            DrawIcon(window->DrawList, iconSize, iconPos, tfContent.leadingIcon, m3Styles, iconColor);
        }
        if (!tfContent.trailingIcon.empty())
        {
            const ImVec2 iconPos(bb.Max.x - iconPadding - iconSize, bb.Min.y + iconOffsetY);
            DrawIcon(window->DrawList, iconSize, iconPos, tfContent.trailingIcon, m3Styles, iconColor);
            inputBB.Max.x -= iconPadding + iconSize;
        }

        // leading space == trailing space.
        labelTextPos.x += labelTextPaddingX;
        if (populated || state == Spec::TextFieldState::Focused)
        {
            {
                const auto fontScope1 = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextPopulatedRole>();
                labelTextPos.y += m3Styles.GetLastText().currHalfLineGap + m3Styles.GetPixels(Spec::TextFieldCommon ::LabelTextPopulatedTopSpace);
                inputBB.Min = ImVec2{labelTextPos.x, labelTextPos.y + m3Styles.GetLastText().currText.textSize};
                DrawText(window->DrawList, labelTextPos, label, labelTextColor);
            }
        }
        else
        {
            const auto   fontScope1 = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextRole>();
            const auto   textOffset = HalfDiff(bb.GetHeight(), m3Styles.GetLastText().currText.textSize);
            const ImVec2 textPos{labelTextPos.x, bb.Min.y + textOffset};
            DrawText(window->DrawList, textPos, label, labelTextColor);
        }

        window->DrawList->PathLineTo(bb.GetBL());
        window->DrawList->PathLineTo({bb.Max.x, bb.Max.y + indicatorHeight});
        window->DrawList->PathStroke(ImGui::ColorConvertFloat4ToU32(indicatorColor), 0, indicatorThickness);
    }
    else
    {
        Spec::ColorRole labelTextColor = Spec::OutlinedTextFieldEnabled::LabelTextColor;
        ImVec4          iconColor      = m3Styles.Colors()[Spec::FilledTextFieldEnabled::LeadingIconColor];
        Spec::ColorRole outlineColor   = Spec::OutlinedTextFieldEnabled::OutlineColor;
        float           outlineWidth   = m3Styles.GetPixels(Spec::OutlinedTextFieldEnabled::OutlineWidth);
        switch (state)
        {
            case Spec::TextFieldState::Hovered: {
                labelTextColor = Spec::OutlinedTextFieldHovered::LabelTextColor;
                outlineColor   = Spec::OutlinedTextFieldHovered::OutlineColor;
                break;
            }
            case Spec::TextFieldState::Focused: {
                labelTextColor = Spec::OutlinedTextFieldFocused::LabelTextColor;
                outlineColor   = Spec::OutlinedTextFieldFocused::OutlineColor;
                outlineWidth   = m3Styles.GetPixels(Spec::OutlinedTextFieldFocused::OutlineWidth);
                break;
            }
            case Spec::TextFieldState::Disabled: {
                labelTextColor = Spec::OutlinedTextFieldDisabled::LabelTextColor;
                outlineColor   = Spec::OutlinedTextFieldDisabled::OutlineColor;
                iconColor.w    = DISABLED_CONTENT_OPACITY;
                break;
            }
            default:
                break;
        }
        // the leading/trailing icon's leading/trailing space always the same.
        const float iconPadding = m3Styles.GetPixels(Spec::TextFieldCommon::IconLeadingSpace);
        const float iconSize    = m3Styles.GetPixels(Spec::TextFieldCommon::LeadingIconSize);
        const float iconOffsetY = HalfDiff(bb.GetHeight(), m3Styles.GetPixels(Spec::TextFieldCommon::LeadingIconSize));
        if (!tfContent.leadingIcon.empty())
        {
            const ImVec2 iconPos = bb.Min + ImVec2(iconPadding, iconOffsetY);
            inputBB.Min.x += iconPadding + iconSize;
            DrawIcon(window->DrawList, iconSize, iconPos, tfContent.leadingIcon, m3Styles, iconColor);
        }
        if (!tfContent.trailingIcon.empty())
        {
            const ImVec2 iconPos(bb.Max.x - iconPadding - iconSize, bb.Min.y + iconOffsetY);
            DrawIcon(window->DrawList, iconSize, iconPos, tfContent.trailingIcon, m3Styles, iconColor);
            inputBB.Max.x -= iconPadding + iconSize;
        }

        if (populated || state == Spec::TextFieldState::Focused)
        {
            ImVec2 labelTextPos(bb.Min.x + labelTextPaddingX, 0.0F);
            float  labelMaxX = labelTextPos.x;
            {
                const auto fontScope = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextPopulatedRole>();
                labelTextPos.y       = bb.Min.y - (m3Styles.GetLastText().currText.textSize * HALF);
                labelMaxX += ImGui::CalcTextSize(TextStart(label), TextEnd(label)).x;
                DrawText(window->DrawList, labelTextPos, label, m3Styles, labelTextColor);
            }

            const auto labelPaddingX = m3Styles.GetPixels(Spec::OutlinedTextFieldEnabled::LabelTextPopulatedContainerPaddingX);
            window->DrawList->PathLineTo(ImVec2{labelMaxX + labelPaddingX, bb.Min.y});
            window->DrawList->PathLineTo(bb.GetTR());
            window->DrawList->PathLineTo(bb.GetBR());
            window->DrawList->PathLineTo(bb.GetBL());
            window->DrawList->PathLineTo(bb.GetTL());
            window->DrawList->PathLineTo({labelTextPos.x - labelPaddingX, bb.Min.y});
            window->DrawList->PathStroke(ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[outlineColor]), 0, outlineWidth);
        }
        else
        {
            const auto   fontScope1  = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextRole>();
            const auto   textOffsetY = HalfDiff(bb.GetHeight(), m3Styles.GetLastText().currText.textSize);
            const ImVec2 textPos(inputBB.Min.x, bb.Min.y + textOffsetY);
            DrawText(window->DrawList, textPos, label, m3Styles, labelTextColor);
        }
    }
}

/**
 * this function support readonly text field (buffer == nullptr) and editable text field (buffer != nullptr). For read
 * only text field, the inputText is used to determine whether the text field is populated, which will affect the label
 * text position.
 */
template <Spec::TextFieldVariant Style>
auto TextField(
    std::string_view label, char *buffer, size_t bufferSize, std::string_view inputText, M3Styles &m3Styles, const TextFieldConfiguration &config
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    const auto idLabel = std::format("##TextField{}", label);
    const auto id      = window->GetID(idLabel.c_str());

    using TfcSpec        = Spec::TextFieldCommon;
    const auto fontScope = m3Styles.UseTextRole<TfcSpec::LabelTextRole>();

    const auto displayLabel = GetDisplayLabel(label);
    float      minWidth     = ImGui::CalcTextSize(TextStart(displayLabel), TextEnd(displayLabel)).x +
                     GetPixels(m3Styles, TfcSpec::LeadingLabelTextSpace, TfcSpec::TrailingLabelTextSpace);
    const auto iconSpace = GetPixels(m3Styles, TfcSpec::IconSpace);
    if (!config.leadingIcon.empty())
    {
        minWidth += iconSpace;
    }
    if (!config.trailingIcon.empty())
    {
        minWidth += iconSpace;
    }
    const auto   height = m3Styles.GetPixels(TfcSpec::Height);
    const ImVec2 size(ImMax(ImGui::GetContentRegionAvail().x, minWidth), height);
    const ImVec2 posMin = GetAlignedCursorPos(window, height);
    const ImRect bb(posMin, posMin + size);
    const bool   editable = buffer != nullptr;

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id, nullptr, editable ? ImGuiItemFlags_Inputable : 0))
    {
        return false;
    }
    const auto          *state       = ImGui::GetInputTextState(id);
    const bool           populated   = editable ? (state != nullptr && state->TextLen > 0) : !inputText.empty();
    bool                 inputActive = editable && ImGui::TempInputIsActive(id);
    Spec::TextFieldState tfState     = Spec::TextFieldState::Enabled;

    if (IsItemDisabled())
    {
        tfState = Spec::TextFieldState::Disabled;
    }
    else
    {
        auto      &g          = *GImGui;
        const bool hovered    = ImGui::ItemHoverable(bb, id, g.LastItemData.ItemFlags);
        const auto clicked    = hovered && ImGui::IsMouseClicked(0, 0, id);
        const bool makeActive = clicked || (g.ActiveId == id);

        // The logic here is somewhat complex.
        // 1. For editable: click or tab(nav) will activate the text field.
        // 2. For non-editable: Click will focus the text field.
        // The "focused" state is only for visual, it won't affect the behavior of text field.
        if (!inputActive)
        {
            if (makeActive && clicked)
            {
                ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
            }
            if (editable)
            {
                if (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput) != 0)
                {
                    inputActive = true;
                }
                if (makeActive && !inputActive)
                {
                    ImGui::SetActiveID(id, window);
                    ImGui::SetFocusID(id, window);
                    ImGui::FocusWindow(window);
                    g.NavActivateId    = id;
                    g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
                    inputActive        = true;
                }
            }
            else if (clicked)
            {
                ImGui::SetFocusID(id, window);
                ImGui::SetNavCursorVisibleAfterMove();
                ImGui::FocusWindow(window);
            }
        }

        if (!hovered && g.IO.MouseClicked[0] && g.NavId == id) // clear NavId when clicked outside
        {
            ImGui::SetNavID(0U, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImRect(g.IO.MousePos, g.IO.MousePos));
        }

        if (g.NavId == id)
        {
            tfState = Spec::TextFieldState::Focused;
        }
        else if (hovered)
        {
            tfState = Spec::TextFieldState::Hovered;
            ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
        }
    }

    ImRect inputBB;
    ImGui::RenderNavCursor(bb, id);
    // Draw outline/active indicator and label text.
    DrawTextField<Style>(displayLabel, tfState, bb, config, m3Styles, populated, inputBB);

    bool       edited            = false;
    const auto nextLineCursorPos = window->DC.CursorPos;
    const auto inputTextOffsetY  = HalfDiff(m3Styles.GetLastText().currText.textSize, inputBB.GetHeight());
    if (inputActive)
    {
        const auto styleGuard = StyleGuard()
                                    .Style<ImGuiStyleVar_FramePadding>({0.0F, inputTextOffsetY})
                                    .Style<ImGuiStyleVar_ItemSpacing>(ImVec2())
                                    .Color<ImGuiCol_FrameBg>(ImVec4{})
                                    .Color<ImGuiCol_Border>(ImVec4{})
                                    .Color<ImGuiCol_NavCursor>(ImVec4{})
                                    .Color<ImGuiCol_InputTextCursor>(m3Styles.Colors()[Spec::TextFieldCommon::CaretColor]);
        const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;
        edited                          = ImGui::TempInputText(inputBB, id, idLabel.c_str(), buffer, static_cast<int>(bufferSize), flags);
        // We not compare buffer with previous value. Keep it simple because TempInputText already has the logic to
        // avoid marking edited when the text is not changed.
    }
    else if (populated)
    {
        const ImVec2 textPos{inputBB.Min.x, inputBB.Min.y + inputTextOffsetY};
        ImVec4       inputTextColor = m3Styles.Colors()[Spec::TextFieldCommon::InputTextColor];
        if (tfState == Spec::TextFieldState::Disabled)
        {
            inputTextColor.w = DISABLED_CONTENT_OPACITY;
        }
        DrawText(window->DrawList, textPos, editable ? buffer : inputText, inputTextColor);
    }
    window->DC.CursorPos = nextLineCursorPos;
    return edited;
}
} // namespace

auto Icon(const std::string_view icon, const Spec::SizeTips sizeTips) -> void
{
    const ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    auto      &m3Styles = Context::GetM3Styles();
    const auto sizing   = Spec::GetIconButtonSizing(sizeTips, Spec::IconButtonWidths::Default);

    const auto   minSize         = m3Styles.GetPixels(Spec::IconButtonCommon::MinLayoutSize);
    const auto   containerHeight = GetPixels(m3Styles, sizing.containerHeight);
    // center align when container height less than min size.
    const auto   offset          = minSize > containerHeight ? HalfDiff(minSize, containerHeight) : 0.0F;
    const auto   height          = std::max(containerHeight, minSize);
    // There may submit a greater item size than bounding box size when container height less than min size,
    // but it won't cause problem because ItemAdd use bounding box for clipping and interaction.
    const ImVec2 size            = {height, height};
    const ImVec2 posMin          = GetAlignedCursorPos(window, height);
    const ImRect bb(posMin + ImVec2(offset, offset), posMin + size - ImVec2(offset, offset));

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0U))
    {
        return;
    }

    ImVec4 iconColor;
    if (IsItemDisabled())
    {
        iconColor   = m3Styles.Colors()[Spec::StandardIconButtonDisabled::IconColor];
        iconColor.w = DISABLED_CONTENT_OPACITY;
    }
    else
    {
        iconColor = m3Styles.Colors()[Spec::StandardIconButtonEnabled::IconColor];
    }

    const auto offsetX  = GetPixels(m3Styles, sizing.leadingSpace);
    const auto iconSize = GetPixels(m3Styles, sizing.iconSize);
    DrawIcon(window->DrawList, iconSize, bb.Min + ImVec2{offsetX, HalfDiff(containerHeight, iconSize)}, icon, m3Styles, iconColor);
}

auto IconButton(std::string_view icon, Spec::SizeTips sizeTips, Spec::IconButtonColorsValues colors, Spec::IconButtonWidths widths) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }

    auto      &m3Styles = Context::GetM3Styles();
    const auto sizing   = Spec::GetIconButtonSizing(sizeTips, widths);

    const ImGuiID id = window->GetID(TextStart(icon), TextEnd(icon));

    const auto   minSize         = m3Styles.GetPixels(Spec::IconButtonCommon::MinLayoutSize);
    const auto   containerHeight = GetPixels(m3Styles, sizing.containerHeight);
    // center align when container height less than min size.
    const auto   offset          = minSize > containerHeight ? HalfDiff(minSize, containerHeight) : 0.0F;
    const auto   height          = std::max(containerHeight, minSize);
    // There may submit a greater item size than bounding box size when container height less than min size,
    // but it won't cause problem because ItemAdd use bounding box for clipping and interaction.
    const ImVec2 size            = {height, height};
    const ImVec2 posMin          = GetAlignedCursorPos(window, height);
    const ImRect bb(posMin + ImVec2(offset, offset), posMin + size - ImVec2(offset, offset));

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    auto &g = *GImGui;

    bool       hovered  = false;
    bool       held     = false;
    const bool pressed  = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    const bool disabled = IsItemDisabled();
    const bool active   = hovered && held;

    ImVec4 bgColor;
    ImVec4 iconColor;
    ImVec4 outlineColor;

    if (disabled)
    {
        bgColor        = m3Styles.Colors()[DISABLED_CONTAINER_COLOR];
        iconColor      = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        outlineColor   = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        bgColor.w      = DISABLED_CONTAINER_OPACITY;
        iconColor.w    = DISABLED_CONTENT_OPACITY;
        outlineColor.w = DISABLED_CONTENT_OPACITY;
    }
    else
    {
        bgColor      = m3Styles.Colors()[colors.containerColor];
        iconColor    = m3Styles.Colors()[colors.iconColor];
        outlineColor = m3Styles.Colors()[colors.outlineColor];
        if (hovered)
        {
            bgColor = ColorUtils::BlendHoveredOrMakeOverlay(bgColor, iconColor);
        }
        else if (active)
        {
            bgColor = ColorUtils::BlendPressedOrMakeOverlay(bgColor, iconColor);
        }
    }

    const auto rounding = m3Styles.GetPixels(active ? sizing.pressedContainerShape : sizing.containerShapeSquare);
    {
        const auto backupFrameRounding = g.Style.FrameRounding;
        g.Style.FrameRounding          = rounding;
        ImGui::RenderNavCursor(bb, id);
        g.Style.FrameRounding = backupFrameRounding;
    }

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), rounding);

    const float outlineWidth = sizing.outlinedOutlineWidth;
    window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(outlineColor), rounding, 0, outlineWidth);

    const auto offsetX  = GetPixels(m3Styles, sizing.leadingSpace);
    const auto iconSize = GetPixels(m3Styles, sizing.iconSize);
    DrawIcon(window->DrawList, iconSize, bb.Min + ImVec2{offsetX, HalfDiff(containerHeight, iconSize)}, icon, m3Styles, iconColor);
    return pressed;
}

auto Fab(std::string_view icon, Spec::SizeTips sizeTips, Spec::FabColors fabColors) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    auto &m3Styles = Context::GetM3Styles();

    const auto    sizing = Spec::GetFabSizingValues(sizeTips);
    const ImGuiID id     = window->GetID(TextStart(icon), TextEnd(icon));
    const auto    height = m3Styles.GetPixels(Spec::FabMedium::ContainerHeight);
    const ImVec2  size   = {height, height};
    const ImVec2  posMin = GetAlignedCursorPos(window, height);
    const ImRect  bb(posMin, posMin + size);

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    auto &g = *GImGui;

    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    const bool active  = hovered && held;

    IM_ASSERT(!IsItemDisabled() && "FAB doesn't have disabled state in spec, IconButton should be used instead when disabled state is needed.");

    const auto   colors    = Spec::GetFabColorsValue(fabColors);
    ImVec4       bgColor   = m3Styles.Colors()[colors.containerColor];
    const ImVec4 iconColor = m3Styles.Colors()[colors.iconColor];

    if (hovered)
    {
        bgColor = ColorUtils::BlendHoveredOrMakeOverlay(bgColor, iconColor);
    }
    else if (active)
    {
        bgColor = ColorUtils::BlendPressedOrMakeOverlay(bgColor, iconColor);
    }

    const auto iconSize = m3Styles.GetPixels(sizing.iconSize);

    const auto rounding = m3Styles.GetPixels(sizing.containerShape);
    {
        const auto backupFrameRounding = g.Style.FrameRounding;
        g.Style.FrameRounding          = rounding;
        ImGui::RenderNavCursor(bb, id);
        g.Style.FrameRounding = backupFrameRounding;
    }

    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), rounding);

    const auto offset = HalfDiff(size.x, iconSize);
    DrawIcon(window->DrawList, iconSize, bb.Min + ImVec2{offset, offset}, icon, m3Styles, iconColor);
    return pressed;
}

auto Button(std::string_view label, const ButtonConfiguration &config) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    auto &m3Styles = Context::GetM3Styles();

    const auto *const      displayLabelEnd = ImGui::FindRenderedTextEnd(TextStart(label), TextEnd(label));
    const std::string_view displayLabel(TextStart(label), displayLabelEnd);

    const auto sizing       = Spec::GetButtonSizing(config.size, config.shape);
    const auto fontScope    = m3Styles.UseTextRole(sizing.labelText);
    float      width        = ImGui::CalcTextSize(TextStart(displayLabel), TextEnd(displayLabel)).x;
    const auto PaddingSpace = m3Styles.GetPixels(sizing.leadingSpace);
    width += PaddingSpace * 2.0F;
    const auto iconSize       = m3Styles.GetPixels(sizing.iconSize);
    const auto iconLabelSpace = m3Styles.GetPixels(sizing.iconLabelSpace);
    if (!config.icon.empty())
    {
        width += iconSize + iconLabelSpace;
    }
    const auto   height = m3Styles.GetPixels(sizing.containerHeight);
    const ImVec2 size(width, height);
    const ImVec2 posMin = GetAlignedCursorPos(window, height);
    const ImRect bb(posMin, posMin + size);

    const auto id = window->GetID(TextStart(label), TextEnd(label));
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }
    auto &g = *GImGui;

    bool       hovered  = false;
    bool       held     = false;
    const bool pressed  = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    const bool active   = hovered && held;
    const bool disabled = IsItemDisabled();

    ImVec4 bgColor;
    ImVec4 textColor;
    ImVec4 iconColor;
    ImVec4 shadowColor;
    if (disabled)
    {
        bgColor     = m3Styles.Colors()[DISABLED_CONTAINER_COLOR];
        textColor   = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        iconColor   = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        bgColor.w   = DISABLED_CONTAINER_OPACITY;
        textColor.w = DISABLED_CONTENT_OPACITY;
        iconColor.w = DISABLED_CONTENT_OPACITY;
    }
    else
    {
        Spec::ButtonVariant variant = Spec::ButtonVariant::Default;
        if (config.toggle)
        {
            variant = config.selected ? Spec::ButtonVariant::ToggleSelected : Spec::ButtonVariant::ToggleUnselected;
        }
        const auto colors = Spec::GetButtonColors(config.colors, variant);
        bgColor           = m3Styles.Colors()[colors.containerColor];
        shadowColor       = m3Styles.Colors()[colors.shadowColor];
        textColor         = m3Styles.Colors()[colors.labelTextColor];
        iconColor         = m3Styles.Colors()[colors.iconColor];
        if (hovered)
        {
            bgColor = ColorUtils::BlendHoveredOrMakeOverlay(bgColor, textColor);
        }
        else if (active)
        {
            bgColor = ColorUtils::BlendPressedOrMakeOverlay(bgColor, textColor);
        }
    }

    const auto rounding = m3Styles.GetPixels(active ? sizing.pressedContainerShape : sizing.containerShape);
    {
        const auto backupFrameRounding = g.Style.FrameRounding;
        g.Style.FrameRounding          = rounding;
        ImGui::RenderNavCursor(bb, id);
        g.Style.FrameRounding = backupFrameRounding;
    }
    window->DrawList->AddRect(bb.Min + ImVec2{-1.0F, 1.0F}, bb.Max + ImVec2{1.0F, 1.0F}, ImGui::ColorConvertFloat4ToU32(shadowColor), rounding);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), rounding);
    if (config.colors == Spec::ButtonColors::outlined)
    {
        const auto outlineColor = m3Styles.Colors()[Spec::OutlinedButtonEnabled::OutlineColor];
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(outlineColor), rounding);
    }

    float labelTextOffsetX = PaddingSpace;
    if (!config.icon.empty())
    {
        const ImVec2 iconPos = bb.Min + ImVec2(labelTextOffsetX, HalfDiff(size.y, iconSize));
        DrawIcon(window->DrawList, iconSize, iconPos, config.icon, m3Styles, iconColor);
        labelTextOffsetX += iconSize + iconLabelSpace;
    }

    const ImVec2 textPos = bb.Min + ImVec2(labelTextOffsetX, HalfDiff(size.y, m3Styles.GetLastText().currText.textSize));
    DrawText(window->DrawList, textPos, displayLabel, textColor);
    return pressed;
}

auto Chip(std::string_view label, const ChipConfiguration &config) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    auto &m3Styles = Context::GetM3Styles();

    const auto displayLabel = GetDisplayLabel(label);

    const auto fontScope     = m3Styles.UseTextRole(Spec::ChipCommon::LabelText);
    float      width         = ImGui::CalcTextSize(TextStart(displayLabel), TextEnd(displayLabel)).x;
    const auto iconSize      = m3Styles.GetPixels(Spec::ChipCommon::IconSize);
    const auto iconSpace     = m3Styles.GetPixels(Spec::ChipCommon::IconLeadingSpace);
    const auto labelSpace    = m3Styles.GetPixels(Spec::ChipCommon::LeadingSpace);
    const auto gap           = m3Styles.GetPixels(Spec::ChipCommon::Gap);
    float      leadingSpace  = 0.0F;
    float      trailingSpace = 0.0F;
    if (!config.icon.empty())
    {
        leadingSpace = iconSpace;
        width += leadingSpace + iconSize + gap;
    }
    else
    {
        leadingSpace = labelSpace;
        width += leadingSpace;
    }
    if (!config.trailingIcon.empty())
    {
        trailingSpace = iconSpace;
        width += trailingSpace + iconSize + gap;
    }
    else
    {
        trailingSpace = labelSpace;
        width += trailingSpace;
    }
    const auto   height = m3Styles.GetPixels(Spec::ChipCommon::ContainerHeight);
    const ImVec2 size(width, height);
    const ImVec2 posMin = GetAlignedCursorPos(window, height);
    const ImRect bb(posMin, posMin + size);

    const auto id = window->GetID(TextStart(label), TextEnd(label));
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }
    auto &g = *GImGui;

    Spec::States states  = Spec::States::Enabled;
    const bool   pressed = GetButtonState(bb, id, states);

    ImVec4 bgColor;
    ImVec4 textColor;
    ImVec4 iconColor;
    ImVec4 outlineColor;
    float  outlineWidth = 1.0F;
    if (states == Spec::States::Disabled)
    {
        bgColor        = m3Styles.Colors()[DISABLED_CONTAINER_COLOR];
        textColor      = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        iconColor      = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        outlineColor   = m3Styles.Colors()[DISABLED_CONTENT_COLOR];
        bgColor.w      = DISABLED_CONTAINER_OPACITY;
        textColor.w    = DISABLED_CONTENT_OPACITY;
        iconColor.w    = DISABLED_CONTENT_OPACITY;
        outlineColor.w = Spec::ChipCommon::DisabledOutlineOpacity;
    }
    else
    {
        Spec::ChipVariant variant = Spec::ChipVariant::Default;
        if (config.colors == Spec::ChipColors::Filter)
        {
            if (config.selected)
            {
                variant      = Spec::ChipVariant::Selected;
                outlineWidth = 0.0F;
            }
            else
            {
                variant = Spec::ChipVariant::Unselected;
            }
        }
        auto colors  = Spec::GetChipsColorsValues(config.colors, states, variant);
        bgColor      = m3Styles.Colors()[colors.containerColor];
        textColor    = m3Styles.Colors()[colors.labelTextColor];
        iconColor    = m3Styles.Colors()[Spec::ChipCommon::IconColor];
        outlineColor = m3Styles.Colors()[colors.outlineColor];
        if (states == Spec::States::Hovered)
        {
            bgColor = ColorUtils::BlendHoveredOrMakeOverlay(bgColor, textColor);
        }
        else if (states == Spec::States::Pressed)
        {
            bgColor = ColorUtils::BlendPressedOrMakeOverlay(bgColor, textColor);
        }
    }

    const auto rounding = m3Styles.GetPixels(Spec::ChipCommon::ContainerShape);
    {
        const auto backupFrameRounding = g.Style.FrameRounding;
        g.Style.FrameRounding          = rounding;
        ImGui::RenderNavCursor(bb, id);
        g.Style.FrameRounding = backupFrameRounding;
    }
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), rounding);
    if (outlineWidth > 0.0F)
    {
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(outlineColor), rounding);
    }

    float labelTextOffsetX = leadingSpace;
    if (!config.icon.empty())
    {
        const ImVec2 iconPos = bb.Min + ImVec2(labelTextOffsetX, HalfDiff(size.y, iconSize));
        DrawIcon(window->DrawList, iconSize, iconPos, config.icon, m3Styles, iconColor);
        labelTextOffsetX += iconSize + gap;
    }
    if (!config.trailingIcon.empty())
    {
        const ImVec2 iconPos = bb.GetTR() - ImVec2(trailingSpace + iconSize, HalfDiff(size.y, iconSize));
        DrawIcon(window->DrawList, iconSize, iconPos, config.icon, m3Styles, iconColor);
    }

    const ImVec2 textPos = bb.Min + ImVec2(labelTextOffsetX, HalfDiff(size.y, m3Styles.GetLastText().currText.textSize));
    DrawText(window->DrawList, textPos, displayLabel, textColor);
    return pressed;
}

void BeginChipGroup()
{
    auto &m3Styles = Context::GetM3Styles();

    const auto lineHeight   = Spec::ChipCommon::LineHeigh;
    const auto chipHeight   = Spec::ChipCommon::ContainerHeight;
    const auto itemSpacingX = m3Styles.GetPixels(Spec::ChipCommon::Margin);
    const auto itemSpacingY = m3Styles.GetPixels(lineHeight - chipHeight);

    // ImGui::GetCurrentWindow()->DC.CurrLineSize.y = m3Styles.GetPixels(lineHeight);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(itemSpacingX, itemSpacingY));
}

void EndChipGroup()
{
    ImGui::PopStyleVar();
}

auto FilledTextField(std::string_view label, char *buffer, size_t bufferSize, const TextFieldConfiguration &config) -> bool
{
    auto &m3Styles = Context::GetM3Styles();
    return TextField<Spec::TextFieldVariant::Filled>(label, buffer, bufferSize, "", m3Styles, config);
}

auto FilledTextField(std::string_view label, std::string_view inputText, const TextFieldConfiguration &config) -> bool
{
    auto &m3Styles = Context::GetM3Styles();
    return TextField<Spec::TextFieldVariant::Filled>(label, nullptr, 0LLU, inputText, m3Styles, config);
}

auto OutlinedTextField(std::string_view label, char *buffer, size_t bufferSize, const TextFieldConfiguration &config) -> bool
{
    auto &m3Styles = Context::GetM3Styles();
    return TextField<Spec::TextFieldVariant::Outlined>(label, buffer, bufferSize, "", m3Styles, config);
}

auto OutlinedTextField(std::string_view label, std::string_view inputText, const TextFieldConfiguration &config) -> bool
{
    auto &m3Styles = Context::GetM3Styles();
    return TextField<Spec::TextFieldVariant::Outlined>(label, nullptr, 0LLU, inputText, m3Styles, config);
}

void ListItem(Func &&contentFunc, const bool plain)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }
    auto      &m3Styles      = Context::GetM3Styles();
    const auto listTextScope = m3Styles.UseTextRole<Spec::List::textRole>();

    const ImVec2 contentOffset = m3Styles.GetPadding<Spec::List>();

    ImRect bb(window->DC.CursorPos, {});
    ImRect contentRect(bb.Min + contentOffset, {});

    window->DrawList->ChannelsSplit(2);

    window->DrawList->ChannelsSetCurrent(CHANNEL_FG);
    ImGui::BeginGroup();
    {
        ImGui::SetCursorScreenPos(contentRect.Min);
        // Submit a Dummy item to apply CursorPos and set the current line min height.
        // Helper components (e.g., ListItemLabel) rely on this value for vertical text centering.
        // Additionally, since ListItems have a minHeight requirement, if contentFunc emits
        // a component exceeding this height, ImGui will dynamically expand DC.CurrLineSize.y,
        // ensuring the layout remains consistent with the actual content.
        const auto minContentHeight = m3Styles.GetPixels(Spec::List::minContentHeight);
        ImGui::Dummy({0.F, minContentHeight});
        ImGui::SameLine(0.F, 0.F);
        contentFunc();
    }
    ImGui::EndGroup();

    contentRect.Max = ImGui::GetItemRectMax();
    window->DrawList->ChannelsSetCurrent(CHANNEL_BG);
    bb.Max = contentRect.Max + contentOffset;
    ImGui::SetCursorScreenPos(bb.Min);

    // Intentional minimal size submission: Prevents layout bloating during AutoFit passes,
    // while ensuring the component remains full-width for rendering and input.
    ImGui::ItemSize(bb);
    if (const auto availX = ImGui::GetContentRegionAvail().x; availX > bb.GetWidth())
    {
        bb.Max.x = bb.Min.x + availX;
    }

    if (ImGui::ItemAdd(bb, 0))
    {
        // only handle hover staus because the click should be handled by inner content.
        if (contentRect.Max.x > contentRect.Min.x)
        {
            auto surfaceColor = m3Styles.Colors()[Spec::ColorRole::surface];
            if (!plain && ImGui::ItemHoverable(bb, 0, GImGui->LastItemData.ItemFlags))
            {
                surfaceColor = m3Styles.Colors().Hovered(Spec::ColorRole::surface, Spec::ColorRole::onSurface);
            }
            window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(surfaceColor));
        }
    }
    window->DrawList->ChannelsMerge();
}

void AlignedLabel(const std::string_view label, const Spec::ColorRole contentRole)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    auto &m3Styles = Context::GetM3Styles();

    // calculate the offset caused by ImGui current layout line with the text line height.
    const auto lineHeight = m3Styles.GetLastText().currText.lineHeight;
    if (lineHeight > 0.F && lineHeight < window->DC.CurrLineSize.y)
    {
        // center align
        const auto offset = HalfDiff(lineHeight, window->DC.CurrLineSize.y);
        window->DC.CursorPos.y += offset;
        window->DC.IsSetPos = true;
    }
    TextUnformatted(label, contentRole);
}

void ListLayoutLeadingColorButton(float height)
{
    const float default_size = ImGui::GetFrameHeight();
    if (height == 0.F)
    {
        height = default_size;
    }

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (height < window->DC.CurrLineSize.y)
    {
        window->DC.CursorPos.y += (window->DC.CurrLineSize.y - height) * HALF;
        window->DC.IsSetPos = true;
    }
}

auto BeginList(const M3Styles &m3Styles, float width, const ChildFlags childFlags) -> ListScope
{
    auto guard = StyleGuard()
                     .Color<ImGuiCol_ChildBg>(m3Styles.Colors()[Spec::ColorRole::surface])
                     .Color<ImGuiCol_Text>(m3Styles.Colors()[Spec::ColorRole::onSurface])
                     .Style<ImGuiStyleVar_ItemSpacing>({m3Styles.GetGap<Spec::List>(), m3Styles.GetPixels(Spec::List::segmentedGap)});
    if (!ImGui::BeginChild("##ListChild", {width, 0.F}, childFlags.AutoResizeY()))
    {
        return {};
    }
    return {.visible = true, .styleGuard = std::move(guard)};
}

void Divider()
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    const ImVec2 size(0.0F, 1.0F);
    const float  x1 = window->DC.CursorPos.x;
    const float  x2 = window->WorkRect.Max.x;
    const ImRect bb({x1, window->DC.CursorPos.y}, {x2, window->DC.CursorPos.y + size.y});

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0))
    {
        return;
    }

    auto &m3Styles = Context::GetM3Styles();
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[Spec::ColorRole::outlineVariant]));
}

auto SearchBar(std::string_view strId, char *buffer, size_t bufferSize, const SearchConfiguration &config) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    auto &m3Styles = Context::GetM3Styles();

    using SearchBarSpec       = Spec::SearchBar;
    const auto innerHideLabel = std::format("##{}", strId);
    const auto id             = window->GetID(TextStart(innerHideLabel), TextEnd(innerHideLabel));
    const auto minWidth       = m3Styles.GetPixels(SearchBarSpec::ContainerMinWidth);
    const auto maxWidth       = m3Styles.GetPixels(SearchBarSpec::ContainerMaxWidth);
    const auto paddingX       = m3Styles.GetPixels(SearchBarSpec::LeadingSpace);
    const auto iconSize       = m3Styles.GetPixels(SearchBarSpec::IconSize);
    const auto iconLabelSpace = m3Styles.GetPixels(SearchBarSpec::LeadingIconLabelSpace);
    const auto height         = m3Styles.GetPixels(SearchBarSpec::ContainerHeight);

    const auto fontScope = m3Styles.UseTextRole(SearchBarSpec::InputText);
    float      width     = config.hintText.empty() ? 0.0F : ImGui::CalcTextSize(TextStart(config.hintText), TextEnd(config.hintText)).x;
    width += paddingX * 2.0F;
    if (!config.icon.empty())
    {
        width += iconSize + iconLabelSpace;
    }
    if (!config.trailingIcon.empty())
    {
        width += iconLabelSpace + iconSize;
    }

    width = std::clamp(width, minWidth, maxWidth);
    const ImVec2 size(width, height);
    const ImVec2 posMin = GetAlignedCursorPos(window, height);
    const ImRect bb(posMin, posMin + size);

    ImGui::ItemSize(size, 0.0F);
    if (!ImGui::ItemAdd(bb, id, nullptr, ImGuiItemFlags_Inputable))
    {
        return false;
    }

    // draw
    IM_ASSERT(!IsItemDisabled() && "SearchBar doesn't have disabled state in spec, TextField should be used instead when disabled state is needed.");

    auto      &g       = *GImGui;
    const bool hover   = ImGui::ItemHoverable(bb, id, g.LastItemData.ItemFlags);
    const auto clicked = hover && ImGui::IsMouseClicked(0, 0, id);
    if (hover)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
    }

    const auto rounding = m3Styles.GetPixels(SearchBarSpec::ContainerShape);
    RenderNavCursor(bb, id, rounding);

    const ImVec4 bgColor = m3Styles.Colors().GetStateColor(Spec::SearchBar::ContainerColor, Spec::SearchBar::InputTextColor, hover, clicked);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), rounding);
    float      cursorX     = bb.Min.x + paddingX;
    const auto iconOffsetY = HalfDiff(size.y, iconSize);
    if (!config.icon.empty())
    {
        const ImVec4 iconColor = m3Styles.Colors()[Spec::SearchBar::LeadingIconColor];
        DrawIcon(window->DrawList, iconSize, ImVec2{cursorX, bb.Min.y + iconOffsetY}, config.icon, m3Styles, iconColor);

        cursorX += iconSize + iconLabelSpace;
    }
    const ImVec2 inputPosMin{cursorX, bb.Min.y + HalfDiff(size.y, m3Styles.GetLastText().currText.textSize)};

    ImRect inputBB(inputPosMin, {bb.Max.x - paddingX, inputPosMin.y + m3Styles.GetLastText().currText.textSize});
    if (!config.trailingIcon.empty())
    {
        inputBB.Max.x -= iconSize;
        const ImVec4 iconColor = m3Styles.Colors()[Spec::SearchBar::TrailingIconColor];
        DrawIcon(window->DrawList, iconSize, ImVec2{inputBB.Max.x, bb.Min.y + iconOffsetY}, config.trailingIcon, m3Styles, iconColor);
        inputBB.Max.x -= iconLabelSpace;
    }

    const bool makeActive  = clicked || (g.ActiveId == id);
    bool       inputActive = ImGui::TempInputIsActive(id);

    if (!inputActive)
    {
        if (makeActive && clicked)
        {
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        }
        if (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput) != 0)
        {
            inputActive = true;
        }
        if (makeActive && !inputActive)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.NavActivateId    = id;
            g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
            inputActive        = true;
        }
    }

    const auto nextLineCursorPos = window->DC.CursorPos;
    bool       edited            = false;
    if (inputActive)
    {
        const auto styleGuard = StyleGuard()
                                    .Style<ImGuiStyleVar_ItemSpacing>(ImVec2())
                                    .Color<ImGuiCol_FrameBg>(ImVec4{})
                                    .Color<ImGuiCol_Border>(ImVec4{})
                                    .Color<ImGuiCol_NavCursor>(ImVec4{})
                                    .Color<ImGuiCol_Text>(m3Styles.Colors()[SearchBarSpec::InputTextColor])
                                    .Color<ImGuiCol_InputTextCursor>(m3Styles.Colors()[SearchBarSpec::FocusIndicatorColor]);
        edited = ImGui::TempInputText(inputBB, id, innerHideLabel.data(), buffer, static_cast<int>(bufferSize), ImGuiInputTextFlags_None);
    }
    else
    {
        const std::string_view bufferSv = buffer;
        DrawText(window->DrawList, inputBB.Min, !bufferSv.empty() ? bufferSv : config.hintText, m3Styles.Colors()[SearchBarSpec::InputTextColor]);
    }
    window->DC.CursorPos = nextLineCursorPos;
    return edited;
}

auto BeginDockedToolbar(const ImVec2 &buttonSize, const uint8_t count, const Spec::ColorRole surfaceRole) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    auto &m3Styles = Context::GetM3Styles();

    const auto margin = m3Styles[Spacing::M];
    const auto size   = ImVec2{ImGui::GetContentRegionAvail().x, buttonSize.y + m3Styles[Spacing::Double_S] + (margin * 2)};
    ImRect     bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0)) return false;

    bb.Min.y += margin;
    bb.Max.y -= margin;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[surfaceRole]));

    const auto minWidth = buttonSize.x * static_cast<float>(count);
    float      gap      = 0.0F;
    if (count > 1U)
    {
        gap = (size.x - minWidth - m3Styles[Spacing::Double_L]) / (count - 1);
        gap = std::max(gap, 0.F);
    }
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, gap);
    ImGui::SetNextItemAllowOverlap();

    window->DC.CursorPos = bb.Min + ImVec2(m3Styles[Spacing::L], m3Styles[Spacing::S]);
    return true;
}

auto EndDockedToolbar() -> void
{
    ImGui::PopStyleVar();
}

auto BeginFloatingToolbar(const char *name, bool *p_open, const Spec::ToolBarColors colors, WindowFlags flags) -> bool
{
    using TbfSpec        = Spec::ToolBar<Spec::ToolBarColors::Standard>;
    using TbfVibrantSpec = Spec::ToolBar<Spec::ToolBarColors::Vibrant>;
    using TbfSizingSpec  = Spec::ToolBarSizing<Spec::ToolBarVariant::Floating>;

    auto      &m3Styles = Context::GetM3Styles();
    const auto paddingX = GetPixels(m3Styles, TbfSizingSpec::ContainerLeadingSpace); // leading == trailing

    const auto bgRole     = colors == Spec::ToolBarColors::Standard ? TbfSpec::ContainerColor : TbfVibrantSpec::ContainerColor;
    const auto styleGuard = StyleGuard()
                                .Style<ImGuiStyleVar_WindowPadding>(ImVec2(paddingX, 0.0F))
                                .Style<ImGuiStyleVar_WindowRounding>(m3Styles.GetPixels(TbfSizingSpec::ContainerShape))
                                .Color<ImGuiCol_WindowBg>(m3Styles.Colors()[bgRole]);

    const auto minWidth = paddingX * 2.0F;
    const auto height   = GetPixels(m3Styles, TbfSizingSpec::HorizontalContainerHeight);
    ImGui::SetNextWindowSizeConstraints({minWidth, height}, {FLT_MAX, height});

    // limit window position
    if (auto *window = ImGui::FindWindowByName(name); window != nullptr)
    {
        const auto vScreenMargin = m3Styles.GetPixels(TbfSizingSpec::VerticalContainerExternalSpace);
        const auto hScreenMargin = m3Styles.GetPixels(TbfSizingSpec::HorizontalContainerExternalSpace);
        const auto displaySize   = ImGui::GetIO().DisplaySize;
        const auto winSize       = window->Size;

        ImVec2 pos = window->Pos;
        pos.x      = std::clamp(pos.x, hScreenMargin, displaySize.x - hScreenMargin - winSize.x);
        pos.y      = std::clamp(pos.y, vScreenMargin, displaySize.y - vScreenMargin - winSize.y);
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(m3Styles.GetPixels(TbfSizingSpec::ContainerBetweenSpace), 0.0F));
    const auto visible = ImGui::Begin(name, p_open, flags.AlwaysAutoResize().NoDecoration().NoResize().NoScrollbar());
    if (!visible)
    {
        ImGui::End();
        ImGui::PopStyleVar();
    }
    ImGui::GetCurrentWindow()->DC.CurrLineSize.y = height;
    // ImGui::Dummy({0.F, height}); // set the current line height.
    // ImGui::SameLine();
    return visible;
}

auto EndFloatingToolbar() -> void
{
    ImGui::End();
    ImGui::PopStyleVar();
}

namespace
{
auto BeginMenu(ImGuiID id, const ImRect &avoidBB, const M3Styles &m3Styles, Spec::MenuColors menuColors, const int32_t maxItemCount) -> bool
{
    if (ImGui::IsPopupOpen(id, ImGuiPopupFlags_None))
    {
        ImVec2 max(FLT_MAX, FLT_MAX);
        if (maxItemCount > 0)
        {
            max.y = m3Styles.GetPixels(Spec::MenuItemSizingStandard::OuterHeightEx) * static_cast<float>(maxItemCount);
        }
        ImGui::SetNextWindowSizeConstraints({avoidBB.GetWidth(), 0.F}, max);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.F, 0.F});

    Spec::ColorRole containerRole = menuColors == Spec::MenuColors::Standard ? Spec::Menus<Spec::MenuColors::Standard>::ContainerColor
                                                                             : Spec::Menus<Spec::MenuColors::Vibrant>::ContainerColor;
    ImGui::PushStyleColor(ImGuiCol_PopupBg, m3Styles.Colors()[containerRole]);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, m3Styles.GetPixels(Spec::Menu::ContainerShape));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
    const auto visible = ImGui::BeginComboPopup(id, avoidBB, 0);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    if (!visible)
    {
        ImGui::PopStyleVar();
    }
    return visible;
}

} // namespace

auto MenuItem(std::string_view label, const bool selected, Spec::MenuColors menuitemColors) -> bool
{
    using MenusSpec    = Spec::MenusSizing<Spec::MenuLayout::Standard>;
    using MenuItemSpec = MenusSpec::Item;

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    // \todo is first/last?
    auto        &m3Styles       = Context::GetM3Styles();
    const auto   labelFontScope = m3Styles.UseTextRole<MenuItemSpec::LabelTextRole>();
    const auto   minWidth       = m3Styles.GetPixels(MenuItemSpec::MinWidthEx);
    const auto   contentWidth   = ImGui::CalcTextSize(TextStart(label), TextEnd(label)).x;
    const auto   width          = ImMax(ImGui::GetContentRegionAvail().x, minWidth + contentWidth);
    const auto   height         = m3Styles.GetPixels(MenusSpec::Item::OuterHeightEx);
    const ImVec2 size(width, height);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const auto   id = window->GetID(TextStart(label), TextEnd(label));
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }
    bool       hovered  = false;
    bool       held     = false;
    bool       pressed  = false;
    const bool focused  = ImGui::IsItemFocused();
    const bool disabled = IsItemDisabled();
    if (!disabled)
    {
        pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    }

    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) != 0 && (GImGui->LastItemData.ItemFlags & ImGuiItemFlags_AutoClosePopups) != 0)
    {
        ImGui::CloseCurrentPopup();
    }

    const ImVec2 contentOffset((m3Styles.GetPixels(MenuItemSpec::OffsetXEx)), m3Styles.GetPixels(MenusSpec::Gap));
    const ImRect contentBB(bb.Min + contentOffset, bb.Max - contentOffset);
    Spec::Unit   rounding = 0U;
    ImVec4       textColor;
    ImVec4       bgColor;
    const auto   colors = Spec::GetMenuItemColors(menuitemColors, disabled);
    if (selected)
    {
        bgColor   = m3Styles.Colors()[colors.selectedContainerColor];
        textColor = m3Styles.Colors()[colors.selectedLabelTextColor];
        rounding  = MenuItemSpec::SelectedShape;
    }
    else
    {
        bgColor   = m3Styles.Colors()[colors.containerColor];
        textColor = m3Styles.Colors()[colors.labelTextColor];
        rounding  = MenuItemSpec::Shape;
    }
    if (disabled)
    {
        if (selected)
        {
            bgColor.w = colors.selectedContainerOpacity;
        }
        textColor.w = DISABLED_CONTENT_OPACITY;
    }
    else
    {
        bgColor = ColorUtils::BlendStateOrMakeOverlay(bgColor, textColor, hovered && held, focused, hovered);
    }

    ImGui::RenderNavCursor(bb, id, ImGuiNavRenderCursorFlags_Compact);
    window->DrawList->AddRectFilled(contentBB.Min, contentBB.Max, ImGui::ColorConvertFloat4ToU32(bgColor), m3Styles.GetPixels(rounding));

    const auto   leadingSpace = m3Styles.GetPixels(MenuItemSpec::LeadingSpace);
    const auto   topSpace     = m3Styles.GetPixels(MenuItemSpec::TopSpaceEx);
    const ImVec2 labelPos     = contentBB.Min + ImVec2(leadingSpace, topSpace + m3Styles.GetLastText().currHalfLineGap);
    DrawText(window->DrawList, labelPos, label, textColor);
    return pressed;
}

auto BeginMenu(std::string_view strId, Spec::MenuColors menuitemColors, const int32_t maxItemCount) -> bool
{
    const auto avoidRect = ImRect(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos());
    const auto popupId   = ImGui::GetID(TextStart(strId), TextEnd(strId));
    auto      &m3Styles  = Context::GetM3Styles();
    return BeginMenu(popupId, avoidRect, m3Styles, menuitemColors, maxItemCount);
}

void EndMenu()
{
    ImGui::EndCombo();
    ImGui::PopStyleVar();
}

auto BeginCombo(std::string_view label, std::string_view previewValue) -> bool
{
    if (ImGui::GetCurrentWindow()->SkipItems)
    {
        return false;
    }

    auto &m3Styles = Context::GetM3Styles();
    // always return false
    (void)TextField<Spec::TextFieldVariant::Outlined>(label, nullptr, 0U, previewValue, m3Styles, {});

    const ImGuiContext &g         = *GImGui;
    const auto          popupId   = ImHashStr("##m3_combo", 0U, g.LastItemData.ID);
    bool                popupOpen = ImGui::IsPopupOpen(popupId, ImGuiPopupFlags_None);
    if (!popupOpen && ImGui::IsItemClicked())
    {
        ImGui::OpenPopupEx(popupId);
        popupOpen = true;
    }
    return popupOpen && BeginMenu(popupId, g.LastItemData.Rect, m3Styles, Spec::MenuColors::Standard, SMALL_MAX_MENU_ITEM_COUNT);
}

void EndCombo()
{
    EndMenu();
}

void SetItemToolTip(const std::string_view text)
{
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) return;

    auto      &m3Styles         = Context::GetM3Styles();
    const auto guard            = StyleGuard().Color<ImGuiCol_PopupBg>(m3Styles.Colors().at(Spec::ColorRole::inverseSurface));
    const auto tooltipFontScope = m3Styles.UseTextRole<Spec::Tooltips::textRole>();
    if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
    {
        TextUnformatted(text, Spec::ColorRole::inverseOnSurface);
        ImGui::EndTooltip();
    }
}

AppBarScope::AppBarScope(Spec::AppBarVariant variant) : m_variant(variant)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    Spec::Unit containerHeight = 0U;
    const auto paddingX        = Spec::AppBarCommon::LeadingSpace;
    switch (variant)
    {
        case Spec::AppBarVariant::Small:
            containerHeight = Spec::SmallAppbar::ContainerHeight;
            break;
        case Spec::AppBarVariant::MediumFlexible:
            containerHeight = Spec::MediumFlexibleAppbar::ContainerHeight;
            break;
        case Spec::AppBarVariant::LargeFlexible:
            containerHeight = Spec::LargeFlexibleAppbar::ContainerHeight;
            break;
    }

    auto        &m3Styles = Context::GetM3Styles();
    const ImVec2 size(ImGui::GetContentRegionAvail().x, m3Styles.GetPixels(containerHeight));
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0))
    {
        return;
    }
    ImGui::BeginGroup();
    // Setting EmitItem = false prevents EndGroup from submitting a layout item for the group bounding box.
    // Without this, EndGroup would call ItemSize/ItemAdd a second time, double-advancing the cursor past the AppBar
    // (ItemSize was already called above to reserve the full AppBar height).
    // The group is retained solely for its cursor/line-state backup+restore behaviour and to terminate
    // any trailing SameLine mode left by the last TrailingIcon call.
    GImGui->GroupStack.back().EmitItem = false;

    m_visible                 = true;
    m_paddingX                = m3Styles.GetPixels(paddingX);
    m_iconLayoutWidth         = m3Styles.GetPixels(Spec::IconButtonCommon::MinLayoutSize);
    m_trailingIconCursorPosX  = bb.Max.x - m_paddingX;
    m_minPos                  = bb.Min;
    window->DC.CursorPos      = bb.Min;
    // For medium/Large AppBar, the title will be placed in the second line.
    window->DC.CurrLineSize.y = m3Styles.GetPixels(Spec::SmallAppbar::ContainerHeight);

    const auto containerColor = m3Styles.Colors()[Spec::AppBarCommon::ContainerColor];
    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(containerColor));
}

AppBarScope::~AppBarScope()
{
    if (m_visible)
    {
        ImGui::EndGroup();
    }
}

auto AppBarScope::LeadingIcon(std::string_view icon) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    // Can't call SameLine in there: the pre-line will be the Appbar's line.
    window->DC.CursorPos.x += m_paddingX;
    m_iconLineItemMaxPosX = window->DC.CursorPos.x + m_iconLayoutWidth;
    const auto visible    = IconButton(icon, Spec::SizeTips::SMALL, {.iconColor = Spec::AppBarCommon::LeadingIconColor});
    ImGui::SameLine(0.F, 0.F);
    return visible;
}

auto AppBarScope::Title(std::string_view title, std::string_view subTitle) -> void
{
    Spec::TypeScaleValue titleText{};
    Spec::TypeScaleValue subtitleText{};
    auto                &m3Styles = Context::GetM3Styles();
    ImGuiWindow         *window   = ImGui::GetCurrentWindow();
    ImVec2               cursor;
    float                titleBottomSpace    = 0.0F; // the space between title and subtitle.
    bool                 needBackupCursorPos = false;
    switch (m_variant)
    {
        case Spec::AppBarVariant::Small: {
            titleText    = Spec::SmallAppbar::TitleText;
            subtitleText = Spec::SmallAppbar::SubtitleText;
            cursor.x     = window->DC.CursorPos.x + m3Styles.GetPixels(Spec::SmallAppbar::TitleLeadingSpace);
            cursor.y     = m_minPos.y + m3Styles.GetPixels(Spec::SmallAppbar::TitleTopSpace);
            break;
        }
        case Spec::AppBarVariant::MediumFlexible: {
            titleText           = Spec::MediumFlexibleAppbar::TitleText;
            subtitleText        = Spec::MediumFlexibleAppbar::SubtitleText;
            titleBottomSpace    = m3Styles.GetPixels(Spec::MediumFlexibleAppbar::TitleBottomSpace);
            cursor.x            = m_minPos.x + m3Styles.GetPixels(Spec::MediumFlexibleAppbar::TitleLeadingSpace);
            cursor.y            = m_minPos.y + m3Styles.GetPixels(Spec::MediumFlexibleAppbar::TitleTopSpace);
            needBackupCursorPos = true;
            break;
        }
        case Spec::AppBarVariant::LargeFlexible: {
            titleText           = Spec::LargeFlexibleAppbar::TitleText;
            subtitleText        = Spec::LargeFlexibleAppbar::SubtitleText;
            titleBottomSpace    = m3Styles.GetPixels(Spec::LargeFlexibleAppbar::TitleBottomSpace);
            cursor.x            = m_minPos.x + m3Styles.GetPixels(Spec::LargeFlexibleAppbar::TitleLeadingSpace);
            cursor.y            = m_minPos.y + m3Styles.GetPixels(Spec::LargeFlexibleAppbar::TitleTopSpace);
            needBackupCursorPos = true;
            break;
        }
    }
    float maxTextWidth = 0.0F; // the max width title/subtitle,
    {
        const auto fontScope = m3Styles.UseTextRole(titleText);
        const auto textColor = m3Styles.Colors()[Spec::AppBarCommon::TitleColor];
        maxTextWidth         = ImGui::CalcTextSize(TextStart(title), TextEnd(title)).x;
        DrawText(window->DrawList, ImVec2(cursor.x, cursor.y + fontScope.CurrTypeScale().currHalfLineGap), title, textColor);
        cursor.y += titleBottomSpace + fontScope.CurrTypeScale().currText.lineHeight;
    }
    if (!subTitle.empty())
    {
        const auto fontScope = m3Styles.UseTextRole(subtitleText);
        const auto textColor = m3Styles.Colors()[Spec::AppBarCommon::SubtitleColor];
        maxTextWidth         = std::max(maxTextWidth, ImGui::CalcTextSize(TextStart(subTitle), TextEnd(subTitle)).x);
        DrawText(window->DrawList, ImVec2(cursor.x, cursor.y + fontScope.CurrTypeScale().currHalfLineGap), subTitle, textColor);
    }
    if (needBackupCursorPos)
    {
        const auto cursorPos = window->DC.CursorPos;
        ImGui::ItemSize({maxTextWidth, 0.0F});
        window->DC.CursorPos = cursorPos;
    }
    else
    {
        ImGui::ItemSize({maxTextWidth, 0.0F});
        ImGui::SameLine(0.F, 0.F);
        m_iconLineItemMaxPosX = window->DC.CursorPos.x;
    }
}

auto AppBarScope::TrailingIcon(std::string_view icon) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();

    m_trailingIconCursorPosX -= m_iconLayoutWidth;
    IM_ASSERT(
        m_iconLineItemMaxPosX <= m_trailingIconCursorPosX &&
        "The leading icon and title have occupied all spaces for trailing icon, please check the layout or reduce the width of leading "
        "icon/title."
    );

    window->DC.CursorPos.x = std::max(m_iconLineItemMaxPosX + m_paddingX, m_trailingIconCursorPosX);

    const auto visible = IconButton(icon, Spec::SizeTips::SMALL, {.iconColor = Spec::AppBarCommon::TrailingIconColor});
    ImGui::SameLine(0.F, 0.F);
    return visible;
}

void SetupDefaultImGuiStyles(ImGuiStyle &style)
{
    using ColorRole = Spec::ColorRole;

    auto       &m3Styles = Context::GetM3Styles();
    const auto &scheme   = m3Styles.Colors();

    style.Colors[ImGuiCol_Text]             = scheme[ColorRole::onSurface];
    style.Colors[ImGuiCol_TitleBg]          = scheme[ColorRole::surfaceContainer];
    style.Colors[ImGuiCol_TitleBgActive]    = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_TitleBgCollapsed] = scheme[ColorRole::surfaceContainer];

    style.Colors[ImGuiCol_WindowBg] = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_ChildBg]  = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_PopupBg]  = scheme[ColorRole::primaryContainer];

    style.Colors[ImGuiCol_FrameBg]        = scheme[ColorRole::secondaryContainer];
    style.Colors[ImGuiCol_FrameBgActive]  = scheme.Pressed(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);
    style.Colors[ImGuiCol_FrameBgHovered] = scheme.Hovered(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);
    style.Colors[ImGuiCol_Border]         = scheme[ColorRole::outlineVariant];
    style.Colors[ImGuiCol_BorderShadow]   = scheme[ColorRole::outlineVariant];

    style.Colors[ImGuiCol_SliderGrab]       = scheme[ColorRole::primary];
    style.Colors[ImGuiCol_SliderGrabActive] = scheme.Pressed(ColorRole::primary, ColorRole::onPrimary);

    style.Colors[ImGuiCol_Button]        = scheme[ColorRole::primary];
    style.Colors[ImGuiCol_ButtonHovered] = scheme.Hovered(ColorRole::primary, ColorRole::onPrimary);
    style.Colors[ImGuiCol_ButtonActive]  = scheme.Pressed(ColorRole::primary, ColorRole::onPrimary);

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = scheme[ColorRole::outline];
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = scheme[ColorRole::outlineVariant];
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = scheme[ColorRole::primary];

    style.Colors[ImGuiCol_MenuBarBg] = scheme[ColorRole::surfaceContainerHigh];

    style.Colors[ImGuiCol_Header]        = scheme[ColorRole::surfaceContainerHigh];
    style.Colors[ImGuiCol_HeaderHovered] = scheme.Hovered(ColorRole::surfaceContainerHigh, ColorRole::onSurface);
    style.Colors[ImGuiCol_HeaderActive]  = scheme.Pressed(ColorRole::surfaceContainerHigh, ColorRole::onSurface);

    style.Colors[ImGuiCol_Separator]        = scheme[ColorRole::secondary];
    style.Colors[ImGuiCol_SeparatorHovered] = scheme.Hovered(ColorRole::secondary, ColorRole::onSecondary);
    style.Colors[ImGuiCol_SeparatorActive]  = scheme.Pressed(ColorRole::secondary, ColorRole::onSecondary);

    style.Colors[ImGuiCol_ResizeGrip]        = scheme[ColorRole::secondaryContainer];
    style.Colors[ImGuiCol_ResizeGripHovered] = scheme.Hovered(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);
    style.Colors[ImGuiCol_ResizeGripActive]  = scheme.Pressed(ColorRole::secondaryContainer, ColorRole::onSecondaryContainer);

    style.Colors[ImGuiCol_InputTextCursor] = scheme[ColorRole::secondary];

    style.Colors[ImGuiCol_Tab]                 = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_TabHovered]          = scheme.Hovered(ColorRole::surface, ColorRole::onSurface);
    style.Colors[ImGuiCol_TabSelected]         = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_TabSelectedOverline] = scheme[ColorRole::primary];

    style.Colors[ImGuiCol_TabDimmed]                 = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_TabDimmedSelected]         = scheme.Pressed(ColorRole::surface, ColorRole::onSurface);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = scheme[ColorRole::outlineVariant];

    style.Colors[ImGuiCol_PlotLines]        = scheme[ColorRole::primary];
    style.Colors[ImGuiCol_PlotLinesHovered] = scheme.Hovered(ColorRole::primary, ColorRole::onPrimary);

    style.Colors[ImGuiCol_PlotHistogram]        = scheme[ColorRole::tertiary];
    style.Colors[ImGuiCol_PlotHistogramHovered] = scheme.Hovered(ColorRole::tertiary, ColorRole::onTertiary);

    style.Colors[ImGuiCol_TableHeaderBg]     = scheme[ColorRole::surfaceContainerHigh];
    style.Colors[ImGuiCol_TableBorderStrong] = scheme[ColorRole::outline];
    style.Colors[ImGuiCol_TableBorderLight]  = scheme[ColorRole::outlineVariant];
    style.Colors[ImGuiCol_TableRowBg]        = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_TableRowBgAlt]     = scheme[ColorRole::surfaceContainerLowest];

    // style.Colors[ImGuiCol_TextLink]     =scheme.surface_container_low; // TODO: set this
    style.Colors[ImGuiCol_TextSelectedBg]   = scheme[ColorRole::primary];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = scheme[ColorRole::onSurface];

    style.Colors[ImGuiCol_DragDropTarget]   = scheme[ColorRole::primary];
    style.Colors[ImGuiCol_DragDropTargetBg] = scheme[ColorRole::surface];

    style.Colors[ImGuiCol_UnsavedMarker]         = scheme[ColorRole::onPrimary];
    style.Colors[ImGuiCol_NavCursor]             = scheme[ColorRole::onSecondary];
    style.Colors[ImGuiCol_NavWindowingHighlight] = scheme[ColorRole::onPrimary];
    style.Colors[ImGuiCol_NavWindowingDimBg]     = scheme[ColorRole::surfaceContainer];

    style.Colors[ImGuiCol_ModalWindowDimBg]   = scheme[ColorRole::surface];
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}
} // namespace ImGuiEx::M3
