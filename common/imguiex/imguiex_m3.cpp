//
// Created by jamie on 2026/1/24.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiex/imguiex_m3.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "m3/facade/menu.h"
#include "m3/facade/nav_rail.h"
#include "m3/facade/text_field.h"
#include "m3/spec/layout.h"
#include "m3/spec/text_field.h"

#include <cmath>

#if IMGUI_VERSION_NUM != 19259
    #error "ImGui version changed! imguiex_m3 only supports v1.92.6WIP"
#endif

namespace ImGuiEx::M3
{

namespace
{

inline auto IsItemDisabled() -> bool
{
    return (ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
}

inline auto IsFocusedAndNavVisible(ImGuiID id) -> bool
{
    const ImGuiContext &g      = *GImGui;
    const ImGuiWindow  *window = g.CurrentWindow;
    return (g.NavId == id) && g.NavCursorVisible && !window->DC.NavHideHighlightOneFrame;
}

struct LayoutInfo
{
    ImVec2 size;
    ImVec2 contentOffset;
    ImRect bb;
};

//! Always use the max width. deduced from [min, max] and `GetContentRegionAvail.x`
template <typename Component>
auto ComputeLayout(std::pair<float, float> widthRange, const M3Styles &m3Styles) -> LayoutInfo
{
    const ImGuiWindow *window = ImGui::GetCurrentWindow();

    const ImVec2 contentOffset = m3Styles.GetPadding<Component>();
    const float  height        = m3Styles.GetHeight<Component>();
    float        minWidth      = widthRange.first;
    float        maxWidth      = widthRange.second;
    if (minWidth <= 0.0F)
    {
        minWidth += ImGui::GetContentRegionAvail().x;
    }
    if (maxWidth <= 0.0F)
    {
        maxWidth += ImGui::GetContentRegionAvail().x;
    }
    const ImVec2 size((ImMax(minWidth, maxWidth)), height);
    return LayoutInfo{
        .size = size, .contentOffset = contentOffset, .bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + size)
    };
}

//! @brief Try to make the text field active and return whether the input is active.
auto TryActiveTextField(const ImGuiID id, ImGuiWindow *window, const bool allowInput) -> bool
{
    ImGuiContext &g = *GImGui;

    const auto hovered     = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) != 0;
    const auto clicked     = hovered && ImGui::IsMouseClicked(0, 0, id);
    const bool makeActive  = clicked || (g.ActiveId == id);
    bool       inputActive = allowInput && ImGui::TempInputIsActive(id);
    if (!inputActive)
    {
        if (makeActive && clicked)
        {
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        }
        if (makeActive && !inputActive)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            if (allowInput)
            {
                g.NavActivateId    = id;
                g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
            }
        }
        if (allowInput &&
            (clicked || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput) != 0)))
        {
            inputActive = true;
        };
    }

    return inputActive;
}

} // namespace

void TextUnformatted(const std::string_view &text, const M3Styles &m3Styles, const Spec::ColorRole contentRole)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    const ImGuiContext &g = *GImGui;

    const float wrap_pos_x   = window->DC.TextWrapPos;
    const bool  wrap_enabled = (wrap_pos_x >= 0.0F);
    if (text.size() <= 2000 || wrap_enabled)
    {
        // Common case
        const float wrap_width    = wrap_enabled ? ImGui::CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0F;
        const auto *const textEnd = TextEnd(text);
        const ImVec2      text_size = ImGui::CalcTextSize(TextStart(text), textEnd, false, wrap_width);

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

inline void DrawIcon(
    ImDrawList *drawList, float iconSize, const ImVec2 &iconPos, std::string_view icon, const M3Styles &m3Styles,
    const Spec::ColorRole contentRole
)
{
    drawList->AddText(
        m3Styles.IconFont(),
        iconSize,
        iconPos,
        ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[contentRole]),
        TextStart(icon),
        TextEnd(icon)
    );
}

inline void DrawText(ImDrawList *drawList, const ImVec2 &textPos, std::string_view text, const ImVec4 &contentColor)
{
    drawList->AddText(
        nullptr, 0.F, textPos, ImGui::ColorConvertFloat4ToU32(contentColor), TextStart(text), TextEnd(text)
    );
}

inline void DrawText(
    ImDrawList *drawList, const ImVec2 &textPos, std::string_view text, const M3Styles &m3Styles,
    const Spec::ColorRole contentRole
)
{
    DrawText(drawList, textPos, text, m3Styles.Colors()[contentRole]);
}

//! Internal used by NavItem when the nav rail is expanded.
auto HoriazontalNavItem(std::string_view label, bool selected, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    using NavItemSpecH = Spec::NavRailItemHorizontal;

    ImGuiWindow *window   = ImGui::GetCurrentWindow();
    const auto   textSize = ImGui::CalcTextSize(TextStart(label), TextEnd(label));
    const auto   width    = textSize.x + m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorMinWidthEx);
    const ImVec2 size(width, m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorHeight));

    window->DC.CursorPos.x += m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorOffsetXEx);
    const auto   id = ImGui::GetID(TextStart(label), TextEnd(label));
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ImGui::ItemSize(size, 0.0F);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }
    const auto &colors  = m3Styles.Colors();
    bool        hovered = false;
    bool        held    = false;
    const bool  pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    const auto  bgColorRole =
        selected ? Spec::ExpandedNavRail::ActiveIndicatorColor : Spec::ExpandedNavRail::ContainerColor;
    if (selected || hovered || held)
    {
        const ImVec4 bgColor = colors.GetStateColor(bgColorRole, Spec::ExpandedNavRail::ActiveIconColor, hovered, held);
        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(bgColor), false, size.y * HALF);
    }
    const ImVec2 iconPos = bb.Min + ImVec2(
                                        m3Styles.GetPixels(NavItemSpecH::ActiveIndicatorLeadingSpace),
                                        m3Styles.GetPixels(NavItemSpecH::FullWidthLeadingSpace)
                                    );
    const auto iconSize = m3Styles.GetPixels(NavItemSpecH::IconSize);
    const auto iconRole = selected ? Spec::ExpandedNavRail::ActiveIconColor : Spec::ExpandedNavRail::InactiveIconColor;
    DrawIcon(window->DrawList, iconSize, iconPos, icon, m3Styles, iconRole);
    const auto   labelScope = m3Styles.UseTextRole<NavItemSpecH::LabelTextRole>();
    const ImVec2 textPos(
        iconPos.x + iconSize + m3Styles.GetPixels(Spec::NavRailItemHorizontal::IconLabelSpace),
        bb.Min.y + HalfDiff(size.y, m3Styles.GetLastText().currText.textSize)
    );
    const auto &textRole =
        selected ? Spec::ExpandedNavRail::ActiveLabelTextColor : Spec::ExpandedNavRail::InactiveLabelTextColor;
    DrawText(window->DrawList, textPos, label, m3Styles, textRole);
    return pressed;
}
} // namespace

auto NavItem(std::string_view label, bool selected, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    using NavItemSpecV = Spec::NavRailItemVertical;

    const ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    const auto availX   = ImGui::GetContentRegionAvail().x;
    const bool expanded = availX > m3Styles.GetPixels(Spec::CollapsedNavRail::ContainerWidth);
    if (expanded)
    {
        return HoriazontalNavItem(label, selected, icon, m3Styles);
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

    const ImVec2 ActiveIndicatorSize(
        m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorWidth), m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorHeight)
    );

    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    const ImVec2 ActiveIndicatorOffset(m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorOffsetXEx), itemVerticalSpace);
    const ImVec2 ActiveIndicatorMin = bb.Min + ActiveIndicatorOffset;
    const ImVec2 ActiveIndicatorMax = ActiveIndicatorMin + ActiveIndicatorSize;

    const auto &colors = m3Styles.Colors();
    {
        const Spec::ColorRole bgColorRole =
            selected ? Spec::CollapsedNavRail::ActiveIndicatorColor : Spec::CollapsedNavRail::ContainerColor;
        // icon bg rect
        if (selected || hovered || held)
        {
            const ImVec4 bgColor =
                colors.GetStateColor(bgColorRole, Spec::ExpandedNavRail::ActiveIconColor, hovered, held);
            ImGui::RenderFrame(
                ActiveIndicatorMin,
                ActiveIndicatorMax,
                ImGui::ColorConvertFloat4ToU32(bgColor),
                false,
                size.y * HALF // full shape
            );
        }
        const ImVec2 iconPos = ActiveIndicatorMin +
                               ImVec2(m3Styles.GetPixels(NavItemSpecV::ActiveIndicatorLeadingSpace), itemVerticalSpace);
        const Spec::ColorRole iconRole =
            selected ? Spec::CollapsedNavRail::ActiveIconColor : Spec::CollapsedNavRail::InactiveIconColor;
        DrawIcon(drawList, m3Styles.GetPixels(NavItemSpecV::IconSize), iconPos, icon, m3Styles, iconRole);
    }

    const auto labelTextRole = m3Styles.UseTextRole<NavItemSpecV::LabelTextRole>();
    {
        const auto textSize = ImGui::CalcTextSize(TextStart(label), TextEnd(label));
        // align left when text is too long, avoid it looks weird when center aligned with ellipsis;
        ImVec2 labelPos(bb.Min.x, ActiveIndicatorMax.y + itemVerticalSpace + m3Styles.GetLastText().currHalfLineGap);
        if (textSize.x <= size.x)
        {
            labelPos.x = bb.Min.x + HalfDiff(size.x, textSize.x); // horizontal center align.
        }

        const auto &labelColorRole =
            selected ? Spec::CollapsedNavRail::ActiveLabelTextColor : Spec::CollapsedNavRail::InactiveLabelTextColor;
        DrawText(drawList, labelPos, label, m3Styles, labelColorRole);
    }
    return pressed;
}

auto BeginNavRail(std::string_view strId, const M3Styles &m3Styles, const bool expanded) -> bool
{
    ImGui::PushStyleColor(
        ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[Spec::CollapsedNavRail::ContainerColor])
    );
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

auto BeginNavRail(std::string_view strId, const M3Styles &m3Styles) -> bool
{
    const bool expanded = ImGui::GetContentRegionAvail().x > m3Styles.GetPixels(Spec::Layout::breakpointLarge);
    return BeginNavRail(strId, m3Styles, expanded);
}

auto EndNavRail() -> void
{
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

auto detail::Icon(
    const std::string_view icon, const M3Styles &m3Styles, const IconLayout &layout, const Spec::ColorRole contentRole
) -> void
{
    const ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    const ImVec2 pos   = window->DC.CursorPos;
    const auto   width = layout.size + (layout.margin * 2.0F);
    const ImVec2 size  = {width, width};
    const ImRect bb(pos, pos + size);

    const auto padding       = (layout.size - layout.iconSize) * 0.5F;
    const auto contentOffset = padding + layout.margin;
    ImGui::ItemSize(size, contentOffset);
    if (!ImGui::ItemAdd(bb, 0U))
    {
        return;
    }

    const auto textColor = m3Styles.Colors().at(contentRole);
    window->DrawList->AddText(
        m3Styles.IconFont(),
        layout.iconSize,
        bb.Min + ImVec2(contentOffset, contentOffset),
        ImGui::ColorConvertFloat4ToU32(textColor),
        TextStart(icon),
        TextEnd(icon)
    );
}

auto detail::IconButton(
    const std::string_view icon, const M3Styles &m3Styles, const IconLayout &layout, const Spec::ColorRole surfaceRole,
    const Spec::ColorRole contentRole
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const ImGuiID id    = window->GetID(TextStart(icon));
    const ImVec2  pos   = window->DC.CursorPos;
    const auto    width = layout.size + (layout.margin * 2.0F);
    const ImVec2  size  = {width, width};
    const ImRect  bb(pos, pos + size);

    const auto padding       = (layout.size - layout.iconSize) * 0.5F;
    const auto contentOffset = padding + layout.margin;
    ImGui::ItemSize(size, contentOffset);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    const auto containerColor = m3Styles.Colors().at(surfaceRole);
    auto       textColor      = m3Styles.Colors().at(contentRole);

    ImVec4 frameColor;
    if (IsItemDisabled())
    {
        textColor.w  = DISABLED_CONTENT;
        frameColor.w = 0.10F;
    }
    else if (hovered && held)
    {
        frameColor = m3Styles.Colors().Pressed(surfaceRole, contentRole);
    }
    else if (hovered)
    {
        frameColor = m3Styles.Colors().Hovered(surfaceRole, contentRole);
    }
    else
    {
        frameColor = containerColor;
    }

    const ImVec2 contentMin = bb.Min + ImVec2{layout.margin, layout.margin};
    const ImVec2 contentMax = bb.Max - ImVec2{layout.margin, layout.margin};
    ImGui::RenderNavCursor(bb, id);
    ImGui::RenderFrame(contentMin, contentMax, ImGui::ColorConvertFloat4ToU32(frameColor), true, layout.rounding);
    window->DrawList->AddText(
        m3Styles.IconFont(),
        layout.iconSize,
        contentMin + ImVec2(padding, padding),
        ImGui::ColorConvertFloat4ToU32(textColor),
        TextStart(icon),
        TextEnd(icon)
    );
    return pressed;
}

namespace
{
template <Spec::TextFieldVariant Style>
auto DrawTextField(
    Spec::TextFieldState state, const ImRect &bb, std::string_view label, const M3Styles &m3Styles,
    const bool populated, __out ImRect &inputBB
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();

    const ImVec2 contentOffset(
        m3Styles.GetPixels(Spec::TextFieldCommon::PaddingX), m3Styles.GetPixels(Spec::TextFieldCommon::PaddingY)
    );
    const ImRect contentBB(bb.Min + contentOffset, bb.Max - contentOffset);

    if constexpr (Style == Spec::TextFieldVariant::Filled)
    {
        ImVec4       labelTextColor     = m3Styles.Colors()[Spec::FilledTextFieldEnabled::LabelTextColor];
        ImVec4       indicatorColor     = m3Styles.Colors()[Spec::FilledTextFieldEnabled::ActiveIndicatorColor];
        float        indicatorHeight    = m3Styles.GetPixels(Spec::FilledTextFieldEnabled::ActiveIndicatorHeight);
        float        indicatorThickness = m3Styles.GetPixels(Spec::FilledTextFieldEnabled::ActiveIndicatorThickness);
        ImVec4       bgColor            = m3Styles.Colors()[Spec::FilledTextFieldEnabled::ContainerColor];
        const ImVec4 inputTextColor     = m3Styles.Colors()[Spec::FilledTextFieldEnabled::InputTextColor];
        switch (state)
        {
            case Spec::TextFieldState::Hovered:
                indicatorColor = m3Styles.Colors()[Spec::FilledTextFieldHovered::ActiveIndicatorColor];
                bgColor        = ColorUtils::BlendHovered(bgColor, inputTextColor);
                break;
            case Spec::TextFieldState::Disabled: {
                bgColor          = m3Styles.Colors()[Spec::FilledTextFieldDisabled::ContainerColor];
                indicatorColor   = m3Styles.Colors()[Spec::FilledTextFieldDisabled::ActiveIndicatorColor];
                bgColor.w        = Spec::FilledTextFieldDisabled::ContainerOpacity;
                labelTextColor.w = DISABLED_CONTENT;
                indicatorColor.w = DISABLED_CONTENT;
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
        if (populated || state == Spec::TextFieldState::Focused)
        {
            float labelTextMaxY = contentBB.Min.y;
            {
                const auto   fontScope1 = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextPopulatedRole>();
                const ImVec2 textPos{contentBB.Min.x, contentBB.Min.y + m3Styles.GetLastText().currHalfLineGap};
                labelTextMaxY = textPos.y + m3Styles.GetLastText().currText.textSize;
                DrawText(window->DrawList, textPos, label, labelTextColor);
            }
            inputBB.Min = ImVec2{contentBB.Min.x, labelTextMaxY};
            inputBB.Max = contentBB.Max;
        }
        else
        {
            const auto   fontScope1 = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextRole>();
            const auto   textOffset = HalfDiff(contentBB.GetHeight(), m3Styles.GetLastText().currText.textSize);
            const ImVec2 textPos{contentBB.Min.x, contentBB.Min.y + textOffset};
            DrawText(window->DrawList, textPos, label, labelTextColor);
        }

        window->DrawList->PathLineTo(bb.GetBL());
        window->DrawList->PathLineTo({bb.Max.x, bb.Max.y + indicatorHeight});
        window->DrawList->PathStroke(ImGui::ColorConvertFloat4ToU32(indicatorColor), 0, indicatorThickness);
    }
    else
    {
        Spec::ColorRole labelTextColor = Spec::OutlinedTextFieldEnabled::LabelTextColor;
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
                break;
            }
            default:
                break;
        }

        if (populated || state == Spec::TextFieldState::Focused)
        {
            float labelMaxX = contentBB.Min.x;
            {
                const auto   fontScope1 = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextPopulatedRole>();
                const ImVec2 textPos{contentBB.Min.x, bb.Min.y - (m3Styles.GetLastText().currText.textSize * HALF)};
                labelMaxX += ImGui::CalcTextSize(TextStart(label), TextEnd(label)).x;
                DrawText(window->DrawList, textPos, label, m3Styles, labelTextColor);
            }
            inputBB = contentBB;

            const auto labelPaddingX =
                m3Styles.GetPixels(Spec::OutlinedTextFieldEnabled::LabelTextPopulatedContainerPaddingX);
            window->DrawList->PathLineTo(ImVec2{labelMaxX + labelPaddingX, bb.Min.y});
            window->DrawList->PathLineTo(bb.GetTR());
            window->DrawList->PathLineTo(bb.GetBR());
            window->DrawList->PathLineTo(bb.GetBL());
            window->DrawList->PathLineTo(bb.GetTL());
            window->DrawList->PathLineTo({contentBB.Min.x - labelPaddingX, bb.Min.y});
            window->DrawList->PathStroke(
                ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[outlineColor]), 0, outlineWidth
            );
        }
        else
        {
            const auto   fontScope1 = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextRole>();
            const auto   textOffset = HalfDiff(contentBB.GetHeight(), m3Styles.GetLastText().currText.textSize);
            const ImVec2 textPos{contentBB.Min.x, contentBB.Min.y + textOffset};
            DrawText(window->DrawList, textPos, label, m3Styles, labelTextColor);
        }
    }
    return true;
}

template <Spec::TextFieldVariant Style>
auto TextField(std::string_view label, char *buffer, size_t bufferSize, const M3Styles &m3Styles) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    const auto idLabel = std::format("##TextField{}", label);
    const auto id      = window->GetID(idLabel.c_str());

    const auto   fontScope    = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextRole>();
    const auto   minWidth     = m3Styles.GetPixels(Spec::TextFieldCommon::MinWidth);
    const auto   contentWidth = ImGui::CalcTextSize(TextStart(label), TextEnd(label)).x;
    const auto   height       = m3Styles.GetPixels(Spec::TextFieldCommon::Height);
    const ImVec2 size(ImMax(ImGui::GetContentRegionAvail().x, minWidth + contentWidth), height);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id, nullptr, ImGuiItemFlags_Inputable))
    {
        return false;
    }
    const auto          *state        = ImGui::GetInputTextState(id);
    const bool           inputIsEmpty = state == nullptr || state->TextLen <= 0;
    const bool           populated    = !inputIsEmpty;
    Spec::TextFieldState tfState      = Spec::TextFieldState::Enabled;

    if (IsItemDisabled())
    {
        tfState = Spec::TextFieldState::Disabled;
    }
    else if (TryActiveTextField(id, window, true) || IsFocusedAndNavVisible(id))
    {
        tfState = Spec::TextFieldState::Focused;
    }
    else if (ImGui::ItemHoverable(bb, id, 0))
    {
        tfState = Spec::TextFieldState::Hovered;
    }

    ImRect inputBB;
    ImGui::RenderNavCursor(bb, id);
    const bool visible = DrawTextField<Style>(tfState, bb, label, m3Styles, populated, inputBB);
    if (!visible)
    {
        return visible;
    }

    bool       edited            = false;
    const auto nextLineCursorPos = window->DC.CursorPos;
    if (populated)
    {
        const auto inputTextOffsetY = HalfDiff(m3Styles.GetLastText().currText.textSize, inputBB.GetHeight());
        const auto styleGuard       = StyleGuard()
                                    .Style<ImGuiStyleVar_FramePadding>({0.0F, inputTextOffsetY})
                                    .Style<ImGuiStyleVar_ItemSpacing>(ImVec2())
                                    .Color<ImGuiCol_FrameBg>(ImVec4{})
                                    .Color<ImGuiCol_Border>(ImVec4{})
                                    .Color<ImGuiCol_NavCursor>(ImVec4{})
                                    .Color<ImGuiCol_InputTextCursor>(m3Styles.Colors()[Spec::ColorRole::primary]);
        const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;
        edited = ImGui::TempInputText(inputBB, id, idLabel.c_str(), buffer, static_cast<int>(bufferSize), flags);
        // We not compare buffer with previous value. Keep it simple because TempInputText already has the logic to
        // avoid marking edited when the text is not changed.
    }
    window->DC.CursorPos = nextLineCursorPos;
    return edited;
}

template <Spec::TextFieldVariant Style>
auto TextField(std::string_view label, std::string_view inputText, const M3Styles &m3Styles) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    const auto idLabel = std::format("##TextField{}", label);
    const auto id      = window->GetID(idLabel.c_str());

    const auto   fontScope    = m3Styles.UseTextRole<Spec::TextFieldCommon::LabelTextRole>();
    const auto   minWidth     = m3Styles.GetPixels(Spec::TextFieldCommon::MinWidth);
    const auto   contentWidth = ImGui::CalcTextSize(TextStart(label), TextEnd(label)).x;
    const auto   height       = m3Styles.GetPixels(Spec::TextFieldCommon::Height);
    const ImVec2 size(ImMax(ImGui::GetContentRegionAvail().x, minWidth + contentWidth), height);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);

    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    const bool           populated = !inputText.empty();
    Spec::TextFieldState tfState   = Spec::TextFieldState::Enabled;
    const ImGuiContext  &g         = *GImGui;

    const auto hovered    = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) != 0;
    const auto clicked    = hovered && ImGui::IsMouseClicked(0, 0, id);
    const bool makeActive = clicked || (g.ActiveId == id);

    if (IsItemDisabled())
    {
        tfState = Spec::TextFieldState::Disabled;
    }
    else
    {
        if (makeActive && clicked)
        {
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        }
        else if (makeActive)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            tfState = Spec::TextFieldState::Focused;
        }
        else if (ImGui::ItemHoverable(bb, id, 0))
        {
            tfState = Spec::TextFieldState::Hovered;
        }
    }

    ImRect inputBB;
    ImGui::RenderNavCursor(bb, id);
    const bool visible = DrawTextField<Style>(tfState, bb, label, m3Styles, populated, inputBB);

    if (ImGui::IsMouseClicked(0, 0, id) && !hovered)
    {
        ImGui::ClearActiveID();
    }

    if (!visible)
    {
        return visible;
    }

    if (populated)
    {
        const std::string str(inputText);
        const auto        inputTextOffsetY = HalfDiff(m3Styles.GetLastText().currText.textSize, inputBB.GetHeight());
        const ImVec2      textPos{inputBB.Min.x, inputBB.Min.y + inputTextOffsetY};
        ImVec4            inputTextColor = m3Styles.Colors()[Spec::TextFieldCommon::InputTextColor];
        if (tfState == Spec::TextFieldState::Disabled)
        {
            inputTextColor.w = DISABLED_CONTENT;
        }
        DrawText(window->DrawList, textPos, str, inputTextColor);
    }
    return false;
}
} // namespace

auto FilledTextField(std::string_view label, char *buffer, size_t bufferSize, const M3Styles &m3Styles) -> bool
{
    return TextField<Spec::TextFieldVariant::Filled>(label, buffer, bufferSize, m3Styles);
}

auto FilledTextField(std::string_view label, std::string_view inputText, const M3Styles &m3Styles) -> bool
{
    return TextField<Spec::TextFieldVariant::Filled>(label, inputText, m3Styles);
}

auto OutlinedTextField(std::string_view label, char *buffer, size_t bufferSize, const M3Styles &m3Styles) -> bool
{
    return TextField<Spec::TextFieldVariant::Outlined>(label, buffer, bufferSize, m3Styles);
}

auto OutlinedTextField(std::string_view label, std::string_view inputText, const M3Styles &m3Styles) -> bool
{
    return TextField<Spec::TextFieldVariant::Outlined>(label, inputText, m3Styles);
}

void ListItem(const std::string_view strId, const M3Styles &m3Styles, Func &&contentFunc, const bool plain)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return;
    }

    const auto listTextScope = m3Styles.UseTextRole<Spec::List::textRole>();

    const ImVec2 contentOffset = m3Styles.GetPadding<Spec::List>();

    ImRect bb(window->DC.CursorPos, {});
    ImRect contentRect(bb.Min + contentOffset, {});

    window->DrawList->ChannelsSplit(2);
    const auto id = window->GetID(TextStart(strId), TextEnd(strId));

    window->DrawList->ChannelsSetCurrent(CHANNEL_FG);
    ImGui::PushID(static_cast<int>(id)); // avoid id conflict in `contentFunc`.
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
    ImGui::PopID();

    contentRect.Max = ImGui::GetItemRectMax();
    window->DrawList->ChannelsSetCurrent(CHANNEL_BG);
    bb.Max = contentRect.Max + contentOffset;
    ImGui::SetCursorScreenPos(bb.Min);

    if (const auto availX = ImGui::GetContentRegionAvail().x; availX > bb.GetWidth())
    {
        bb.Max.x = bb.Min.x + availX;
    }

    ImGui::ItemSize(bb);
    if (ImGui::ItemAdd(bb, id))
    {
        // only handle hover staus because the click should be handled by inner content.
        if (contentRect.Max.x > contentRect.Min.x)
        {
            auto      &g       = *GImGui;
            const bool hovered = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) != 0;

            auto surfaceColor = m3Styles.Colors()[Spec::ColorRole::surface];
            if (!plain && hovered)
            {
                surfaceColor = m3Styles.Colors().Hovered(Spec::ColorRole::surface, Spec::ColorRole::onSurface);
            }
            window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(surfaceColor));
        }
    }
    window->DrawList->ChannelsMerge();
}

void AlignedLabel(const std::string_view label, const M3Styles &m3Styles, const Spec::ColorRole contentRole)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    // calculate the offset caused by ImGui current layout line with the text line height.
    const auto lineHeight = m3Styles.GetLastText().currText.lineHeight;
    if (lineHeight > 0.F && lineHeight < window->DC.CurrLineSize.y)
    {
        // center align
        const auto offset = HalfDiff(lineHeight, window->DC.CurrLineSize.y);
        window->DC.CursorPos.y += offset;
        window->DC.IsSetPos = true;
    }
    TextUnformatted(label, m3Styles, contentRole);
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
                     .Style<ImGuiStyleVar_ItemSpacing>(
                         {m3Styles.GetGap<Spec::List>(), m3Styles.GetPixels(Spec::List::segmentedGap)}
                     );
    if (!ImGui::BeginChild("##ListChild", {width, 0.F}, childFlags.AutoResizeY()))
    {
        return {};
    }
    return ListScope{true, std::move(guard)};
}

auto BeginDockedToolbar(
    const ImVec2 &buttonSize, const uint8_t count, const Spec::ColorRole surfaceRole, const M3Styles &m3Styles
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const auto margin = m3Styles[Spacing::M];
    const auto size =
        ImVec2{ImGui::GetContentRegionAvail().x, buttonSize.y + m3Styles[Spacing::Double_S] + (margin * 2)};
    ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
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

namespace
{
template <Spec::MenuVariant Variant>
auto MenuItem(std::string_view label, const bool selected, const M3Styles &m3Styles) -> bool
{
    using MenusSpec    = Spec::Menus<Spec::MenuLayout::Standard, Variant>;
    using MenuItemSpec = MenusSpec::Item;

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    // is first
    // is last
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

    const ImVec2 contentOffset((m3Styles.GetPixels(MenuItemSpec::OffsetXEx)), m3Styles.GetPixels(MenusSpec::Gap));
    const ImRect contentBB(bb.Min + contentOffset, bb.Max - contentOffset);
    Spec::Unit   rounding = 0U;
    ImVec4       textColor;
    ImVec4       bgColor;
    if (selected)
    {
        bgColor   = m3Styles.Colors()[MenuItemSpec::SelectedContainerColor];
        textColor = m3Styles.Colors()[MenuItemSpec::SelectedLabelTextColor];
        rounding  = MenuItemSpec::SelectedShape;
    }
    else
    {
        bgColor   = m3Styles.Colors()[MenuItemSpec::ContainerColor];
        textColor = m3Styles.Colors()[MenuItemSpec::LabelTextColor];
        rounding  = MenuItemSpec::Shape;
    }
    if (disabled)
    {
        bgColor.w   = MenuItemSpec::DisabledContainerOpacity;
        textColor.w = DISABLED_CONTENT;
    }
    else
    {
        bgColor = ColorUtils::BlendState(bgColor, textColor, hovered && held, focused, hovered);
    }

    ImGui::RenderNavCursor(bb, id, ImGuiNavRenderCursorFlags_Compact);
    window->DrawList->AddRectFilled(
        contentBB.Min, contentBB.Max, ImGui::ColorConvertFloat4ToU32(bgColor), m3Styles.GetPixels(rounding)
    );

    const auto   leadingSpace = m3Styles.GetPixels(MenuItemSpec::LeadingSpace);
    const auto   topSpace     = m3Styles.GetPixels(MenuItemSpec::TopSpaceEx);
    const ImVec2 labelPos     = contentBB.Min + ImVec2(leadingSpace, topSpace + m3Styles.GetLastText().currHalfLineGap);
    DrawText(window->DrawList, labelPos, label, textColor);
    return pressed;
}

auto BeginMenu(std::string_view strId, const ImRect &avoidBB, const M3Styles &m3Styles, const int32_t maxItemCount)
    -> bool
{
    const auto popupId = ImGui::GetID(TextStart(strId), TextEnd(strId));
    if (ImGui::IsPopupOpen(popupId, ImGuiPopupFlags_None))
    {
        ImVec2 max(FLT_MAX, FLT_MAX);
        if (maxItemCount > 0)
        {
            max.y = m3Styles.GetPixels(Spec::MenuItemStandard::OuterHeightEx) * static_cast<float>(maxItemCount);
        }
        ImGui::SetNextWindowSizeConstraints({avoidBB.GetWidth(), 0.F}, max);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.F, 0.F});

    ImGui::PushStyleColor(ImGuiCol_PopupBg, m3Styles.Colors()[Spec::Menu::ContainerColor]);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, m3Styles.GetPixels(Spec::Menu::ContainerShape));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
    const auto visible = ImGui::BeginComboPopup(popupId, avoidBB, 0);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    if (!visible)
    {
        ImGui::PopStyleVar();
    }
    return visible;
}

} // namespace

auto MenuItem(std::string_view label, const bool selected, const M3Styles &m3Styles, const bool vibrant) -> bool
{
    if (vibrant)
    {
        return MenuItem<Spec::MenuVariant::Vibrant>(label, selected, m3Styles);
    }
    return MenuItem<Spec::MenuVariant::Standard>(label, selected, m3Styles);
}

auto BeginMenu(std::string_view strId, const M3Styles &m3Styles, const int32_t maxItemCount) -> bool
{
    const auto avoidRect = ImRect(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos());
    return BeginMenu(strId, avoidRect, m3Styles, maxItemCount);
}

void EndMenu()
{
    ImGui::EndCombo();
    ImGui::PopStyleVar();
}

auto BeginCombo(std::string_view label, std::string_view previewValue, const M3Styles &m3Styles)
{
    // TextField();
}

void SetItemToolTip(const std::string_view text, const M3Styles &m3Styles)
{
    const auto guard = StyleGuard()
                           .Color<ImGuiCol_PopupBg>(m3Styles.Colors().at(Spec::ColorRole::inverseSurface))
                           .Style<ImGuiStyleVar_WindowPadding>(m3Styles.GetPadding<Spec::Tooltips>());

    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) return;

    const auto tooltipFontScope = m3Styles.UseTextRole<Spec::Tooltips::textRole>();
    ImGui::PushFont(nullptr, m3Styles.GetLastText().currText.textSize);
    if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
    {
        TextUnformatted(text, m3Styles, Spec::ColorRole::inverseOnSurface);
        ImGui::EndTooltip();
    }
    ImGui::PopFont();
}
} // namespace ImGuiEx::M3
