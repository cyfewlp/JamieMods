//
// Created by jamie on 2026/1/24.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiex/imguiex_m3.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "m3/facade/nav_rail.h"
#include "m3/facade/text_field.h"

#if IMGUI_VERSION_NUM != 19259
    #error "ImGui version changed! imguiex_m3 only supports v1.92.6WIP"
#endif

namespace ImGuiEx::M3
{

namespace
{

// \todo remove it and replace by M3::TextUnformated or AligendLabel
void AlignText(ImVec2 &posMin, const ImVec2 &align, const ImVec2 &posMax, const ImVec2 &textSize)
{
    // copy from imgui.cpp#3857
    if (align.x > 0.0F) posMin.x = ImMax(posMin.x, posMin.x + ((posMax.x - posMin.x - textSize.x) * align.x));
    if (align.y > 0.0F) posMin.y = ImMax(posMin.y, posMin.y + ((posMax.y - posMin.y - textSize.y) * align.y));
}

[[nodiscard]] auto TextClip(const ImVec2 &textSize, const ImVec2 &pos, const ImRect &clipRect) -> std::optional<ImVec4>
{
    // copy from imgui.cpp#3847
    // Perform CPU side clipping for single clipped element to avoid using scissor state
    bool need_clipping = pos.x + textSize.x >= clipRect.Max.x || pos.y + textSize.y >= clipRect.Max.y;
    need_clipping      = need_clipping || pos.x < clipRect.Min.x || pos.y < clipRect.Min.y;
    if (need_clipping)
    {
        return ImVec4(clipRect.Min.x, clipRect.Min.y, clipRect.Max.x, clipRect.Max.y);
    }
    return std::nullopt;
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
auto TryActiveTextField(const ImGuiID id, ImGuiWindow *window) -> bool
{
    ImGuiContext &g = *GImGui;

    const auto hovered     = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) != 0;
    const auto clicked     = hovered && ImGui::IsMouseClicked(0, 0, id);
    const bool makeActive  = clicked || (g.ActiveId == id);
    bool       inputActive = ImGui::TempInputIsActive(id);
    if (makeActive && clicked)
    {
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
    }
    if (makeActive && !inputActive)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);

        g.NavActivateId    = id;
        g.NavActivateFlags = ImGuiActivateFlags_PreferInput;
    }
    if (clicked || (g.NavActivateId == id && (g.NavActivateFlags & ImGuiActivateFlags_PreferInput) != 0))
    {
        inputActive = true;
    }

    bool const focused = (g.NavId == id) && g.NavCursorVisible && !window->DC.NavHideHighlightOneFrame;
    return inputActive || focused;
}

} // namespace

void TextUnformatted(const std::string_view &text, const M3Styles &m3Styles, const Spec::ColorRole contentRole)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    const ImGuiContext &g = *GImGui;

    const float wrap_pos_x   = window->DC.TextWrapPos;
    const bool  wrap_enabled = (wrap_pos_x >= 0.0f);
    if (text.size() <= 2000 || wrap_enabled)
    {
        // Common case
        const float wrap_width    = wrap_enabled ? ImGui::CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
        const auto *const textEnd = TextEnd(text);
        const ImVec2      text_size = ImGui::CalcTextSize(TextStart(text), textEnd, false, wrap_width);

        const auto   halfGap = ImMax(0.0F, m3Styles.GetLastText().currHalfLineGap);
        const auto   size    = text_size + ImVec2(0, halfGap * 2.0F);
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

auto NavItem(std::string_view label, bool selected, std::string_view icon, const M3Styles &m3Styles) -> bool
{
    const float itemVerticalSpace = m3Styles.GetPixels(Spec::CollapsedNavRail::ItemVerticalSpace);
    const auto  styleGuard        = StyleGuard().Style<ImGuiStyleVar_ItemSpacing>({0.F, itemVerticalSpace});

    ImGuiWindow *window = ImGui::GetCurrentWindow();

    const ImVec2 size(ImGui::GetContentRegionAvail().x, m3Styles.GetPixels(Spec::NavRailItem::ContainerHeight));
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const auto   id = ImGui::GetID(TextStart(label), TextEnd(label));
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }
    const auto nextLineCursorPos = window->DC.CursorPos;
    window->DC.CursorPos         = bb.Min;

    const ImVec2 ActiveIndicatorSize(
        m3Styles.GetPixels(Spec::NavRailItemVertical::ActiveIndicatorWidth),
        m3Styles.GetPixels(Spec::NavRailItemVertical::ActiveIndicatorHeight)
    );

    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    const ImVec2 ActiveIndicatorOffset = ImVec2(HalfDiff(ActiveIndicatorSize.x, size.x), itemVerticalSpace);
    window->DC.CursorPos += ActiveIndicatorOffset;

    const ImRect ActiveIndicatorBB(window->DC.CursorPos, window->DC.CursorPos + ActiveIndicatorSize);

    const auto &colors = m3Styles.Colors();
    {
        const Spec::ColorRole bgColorRole =
            selected ? Spec::NavRailCommon::ActiveIndicatorColor : Spec::ColorRole::none;
        // icon bg rect
        if (selected || hovered || held)
        {
            ImVec4 bgColor;
            if (hovered && held)
            {
                bgColor = colors.Pressed(bgColorRole, Spec::NavRailCommon::ActiveIconColor);
            }
            else if (hovered)
            {
                bgColor = colors.Hovered(bgColorRole, Spec::NavRailCommon::ActiveIconColor);
            }
            else
            {
                bgColor = colors[bgColorRole];
            }
            ImGui::RenderFrame(
                ActiveIndicatorBB.Min,
                ActiveIndicatorBB.Max,
                ImGui::ColorConvertFloat4ToU32(bgColor),
                true,
                m3Styles.GetPixels(Spec::NavRailItem::ActiveIndicatorLeadingSpace) // full shape
            );
        }
        window->DC.CursorPos.x += m3Styles.GetPixels(Spec::NavRailItem::ActiveIndicatorLeadingSpace);
        window->DC.CursorPos.y += itemVerticalSpace;
        const Spec::ColorRole iconRole =
            selected ? Spec::NavRailCommon::ActiveIconColor : Spec::NavRailCommon::InactiveIconColor;
        drawList->AddText(
            m3Styles.IconFont(),
            m3Styles.GetPixels(Spec::NavRailItem::IconSize),
            window->DC.CursorPos,
            ImGui::ColorConvertFloat4ToU32(colors[iconRole]),
            TextStart(icon),
            TextEnd(icon)
        );
    }

    window->DC.CursorPos.y = ActiveIndicatorBB.Max.y + itemVerticalSpace;
    const auto labelRole   = m3Styles.UseTextRole<Spec::NavRailItemVertical::LabelTextRole>();
    {
        const auto textSize = ImGui::CalcTextSize(TextStart(label), TextEnd(label));
        if (textSize.x > size.x)
        {
            window->DC.CursorPos.x =
                bb.Min.x; // align left when text is too long, avoid it looks weird when center aligned with ellipsis;
        }
        else
        {
            window->DC.CursorPos.x = bb.Min.x + HalfDiff(size.x, textSize.x); // horizontal center align.
        }
        // need add half line gap because we not call our TextUnformatted function to render text,
        // which will add half line gap as vertical padding.
        window->DC.CursorPos.y += m3Styles.GetLastText().currHalfLineGap;

        const auto &textColor = selected ? colors[Spec::NavRailCommon::ActiveLabelTextColor]
                                         : colors[Spec::NavRailCommon::InactiveLabelTextColor];
        drawList->AddText(
            nullptr,
            0.F,
            window->DC.CursorPos,
            ImGui::ColorConvertFloat4ToU32(textColor),
            TextStart(label),
            TextEnd(label),
            0.0F
        );
    }
    window->DC.CursorPos = nextLineCursorPos; // restore cursor pos.
    return pressed;
}

auto BeginNavRail(
    std::string_view strId, const M3Styles &m3Styles, ImGuiEx::ChildFlags childFlags, Spec::ColorRole containerColor
) -> bool
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[containerColor]));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.F, 0.F});
    const bool visible = ImGui::BeginChild(
        TextStart(strId), {m3Styles.GetPixels(Spec::CollapsedNavRail::ContainerWidth), 0.0F}, childFlags
    );
    ImGui::Dummy({0.F, m3Styles.GetPixels(Spec::CollapsedNavRail::TopSpace)});
    return visible;
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
    const auto   width = layout.size + (layout.margin * 2);
    const ImVec2 size  = {width, width};
    const ImRect bb(pos, pos + size);

    const auto padding       = (layout.size - layout.iconSize) * 0.5F;
    const auto contentOffset = padding + layout.margin;
    ImGui::ItemSize(size, contentOffset);
    if (!ImGui::ItemAdd(bb, 0))
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
    const auto    width = layout.size + (layout.margin * 2);
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
    if (const ImGuiContext &g = *GImGui; g.CurrentItemFlags & ImGuiItemFlags_Disabled)
    {
        textColor  = m3Styles.Colors().DisabledContent(contentRole);
        frameColor = m3Styles.Colors().DisabledSurface(surfaceRole);
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

auto detail::ResolveItemWidth(float width, float defaultWidth, const M3Styles &m3Styles) -> float
{
    width             = m3Styles.GetPixels(width);
    // ignore avail width when width greater than zero.
    const auto availX = width > 0.0F ? 0.0F : ImGui::GetContentRegionAvail().x;
    if (width < 0.0F)
    {
        width = ImMax(0.0F, width + availX);
    }
    else
    {
        width = ImMax(availX, width);
    }
    return width;
}

namespace
{
void DrawFocusedLabelAndIndicator(
    ImGuiWindow *window, std::string_view label, const ImRect &bb, const M3Styles &m3Styles
)
{
    const auto backUpCursorPosX = window->DC.CursorPos.x;
    {
        const auto supportFontScope = m3Styles.UseTextRole<Spec::TextFieldBase::supportTextRole>();
        ImGuiEx::M3::TextUnformatted(label, m3Styles, Spec::ColorRole::primary);
    }
    const auto indicatorHeight = m3Styles.GetPixels(M3Spec::FilledTextFieldFocused::activeIndicatorHeight);
    const auto thickness       = m3Styles.GetPixels(M3Spec::FilledTextFieldFocused::activeIndicatorThickness);
    window->DrawList->PathLineTo({bb.Min.x, bb.Max.y});
    window->DrawList->PathLineTo({bb.Max.x, bb.Max.y + indicatorHeight});
    window->DrawList->PathStroke(
        ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[M3Spec::ColorRole::primary]), 0, thickness
    );
    window->DC.CursorPos.x = backUpCursorPosX;
}

void DrawFocusedLabelAndOutline(
    ImGuiWindow *window, std::string_view label, const LayoutInfo &layoutInfo, const M3Styles &m3Styles
)
{
    const auto &bb               = layoutInfo.bb;
    const auto  backUpCursorPosX = window->DC.CursorPos.x;
    const auto  labelPaddingX    = m3Styles.GetPixels(Spec::OutlinedTextFieldFocused::labelPaddingX);
    {
        const auto supportFontScope = m3Styles.UseTextRole<M3Spec::TextFieldBase::supportTextRole>();
        window->DC.CursorPos.y      = bb.Min.y - (m3Styles.GetLastText().currText.lineHeight * HALF);
        ImGuiEx::M3::TextUnformatted(label, m3Styles, M3Spec::ColorRole::primary);
    }

    window->DrawList->PathLineTo(ImVec2{ImGui::GetItemRectMax().x + labelPaddingX, bb.Min.y});
    window->DrawList->PathLineTo(bb.GetTR());
    window->DrawList->PathLineTo(bb.GetBR());
    window->DrawList->PathLineTo(bb.GetBL());
    window->DrawList->PathLineTo(bb.GetTL());
    window->DrawList->PathLineTo({backUpCursorPosX - labelPaddingX, bb.Min.y});
    window->DrawList->PathStroke(
        ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[Spec::ColorRole::primary]),
        0,
        Spec::OutlinedTextFieldFocused::outlineWidth
    );
    window->DC.CursorPos.x = backUpCursorPosX;
}
} // namespace

auto TextField(
    std::string_view label, Spec::TextFieldStyle tfStyle, char *buffer, size_t bufferSize, const M3Styles &m3Styles,
    float width
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
    {
        return false;
    }
    const auto idLabel = std::format("##TextField{}", label);

    const auto id = window->GetID(idLabel.c_str());

    const auto layoutInfo = ComputeLayout<Spec::TextFieldBase>({m3Styles.GetPixels(width), 0.0F}, m3Styles);

    const auto bb            = layoutInfo.bb;
    const auto contentOffset = layoutInfo.contentOffset;

    bool edited = false;
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, id, nullptr, ImGuiItemFlags_Inputable))
    {
        return edited;
    }

    const auto nextLineCursorPos = ImGui::GetCursorScreenPos();

    const bool focused = TryActiveTextField(id, window);

    window->DrawList->AddRectFilled(
        bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[Spec::ColorRole::surfaceContainerHighest])
    );

    window->DC.CursorPos = bb.Min + contentOffset;
    if (focused)
    {
        if (tfStyle == Spec::TextFieldStyle::Filled)
        {
            DrawFocusedLabelAndIndicator(window, label, bb, m3Styles);
        }
        else
        {
            DrawFocusedLabelAndOutline(window, label, layoutInfo, m3Styles);
        }

        const ImRect contentBB(window->DC.CursorPos, bb.Max - contentOffset);
        const auto   styleGuard =
            StyleGuard()
                .Style<ImGuiStyleVar_FramePadding>(
                    {0.0F, HalfDiff(m3Styles.GetLastText().currText.textSize, contentBB.GetHeight())}
                )
                .Style<ImGuiStyleVar_ItemSpacing>(ImVec2())
                .Color<ImGuiCol_FrameBg>(ImVec4{})
                .Color<ImGuiCol_Border>(ImVec4{})
                .Color<ImGuiCol_NavCursor>(ImVec4{})
                .Color<ImGuiCol_InputTextCursor>(m3Styles.Colors()[Spec::ColorRole::primary]);
        const ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;
        edited = ImGui::TempInputText(contentBB, id, idLabel.c_str(), buffer, static_cast<int>(bufferSize), flags);
        // We not compare buffer with previous value. Keep it simple because TempInputText already has the logic to
        // avoid marking edited when the text is not changed.
    }
    else
    {
        if (tfStyle == Spec::TextFieldStyle::Filled)
        {
            const auto indicatorHeight = m3Styles.GetPixels(Spec::FilledTextFieldEnabled::activeIndicatorHeight);
            window->DrawList->AddLine(
                bb.GetBL(),
                bb.Max,
                ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[M3Spec::ColorRole::outline]),
                indicatorHeight
            );
        }
        else
        {
            window->DrawList->AddRect(
                bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(m3Styles.Colors()[M3Spec::ColorRole::outline])
            );
        }

        const ImGuiInputTextState *state = ImGui::GetInputTextState(id);
        ImGui::Dummy({0.0F, m3Styles.GetPixels(Spec::TextFieldBase::contentHeight)});
        ImGui::SameLine(0.0F, 0.0F);
        const bool inputIsEmpty = state == nullptr || state->TextLen <= 0;
        ImGuiEx::M3::AlignedLabel(
            inputIsEmpty ? label : std::string_view(buffer, static_cast<size_t>(state->TextLen)), m3Styles
        );
    }
    // reset cursor.
    ImGui::SetCursorScreenPos(nextLineCursorPos);
    return edited;
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
