//
// Created by jamie on 2026/1/24.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiex/imguiex_m3.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"

#if IMGUI_VERSION_NUM != 19259
    #error "ImGui version changed! imguiex_m3 only supports v1.92.6WIP"
#endif

namespace ImGuiEx::M3
{

namespace
{

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
        ImGui::ItemSize(size, 0.0f);
        if (!ImGui::ItemAdd(bb, 0)) return;

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

/**
 * Since the width of the navigation track is locked, a simple centered layout is all that's needed.
 */
void DrawNavMenu(const std::string_view icon, const M3Styles &m3Styles) // TODO: maybe replace with Icon()
{
    ImGui::Dummy({0.F, m3Styles[Spacing::L]});
    ImGui::TextAligned(ALIGN_CENTER, -FLT_MIN, "%s", TextStart(icon));
    ImGui::Dummy({0.F, m3Styles[Spacing::L]});
}

auto DrawNavItem(
    const std::string_view label, const bool selected, const std::string_view icon, const M3Styles &m3Styles
) -> bool
{
    StyleGuard styleGuard;
    styleGuard.Style<ImGuiStyleVar_ItemSpacing>({0.F, m3Styles[Spacing::XS]});
    const auto   regionLT   = ImGui::GetCursorScreenPos();
    const float  itemWidth  = ImGui::GetContentRegionAvail().x;
    const auto   iconSize   = ImVec2{m3Styles.IconSize(), m3Styles.IconSize()};
    const float  itemHeight = m3Styles.LabelText().textSize + iconSize.y + (m3Styles[Spacing::XS] * 4);
    const ImRect bb(regionLT, {regionLT.x + itemWidth, regionLT.y + itemHeight});
    const auto   id = ImGui::GetID(TextStart(label));
    ImGui::ItemSize({itemWidth, itemHeight}, m3Styles[Spacing::XS]);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    bool       hovered = false;
    bool       held    = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    const auto &colors    = m3Styles.Colors();
    auto        iconColor = colors[Spec::ColorRole::onSurfaceVariant];

    ImVec2 iconPosMin{bb.Min.x, bb.Min.y + m3Styles[Spacing::XS]};
    ImVec2 iconPosMax{bb.Max.x, iconPosMin.y + iconSize.y};
    {
        const auto fineClipOpt = TextClip(iconSize, iconPosMin, bb);
        // modify iconPosMin
        AlignText(iconPosMin, {ALIGN_CENTER, ALIGN_CENTER}, iconPosMax, iconSize);
        iconPosMax = iconPosMin + iconSize;
        // icon bg rect
        if (selected || hovered || held)
        {
            const auto offset     = ImVec2(m3Styles[Spacing::L], m3Styles[Spacing::XS]);
            const auto iconBgRect = ImRect(iconPosMin - offset, iconPosMin + iconSize + offset);
            if (selected) iconColor = colors[Spec::ColorRole::onPrimary];
            ImVec4 bgColor;
            if (hovered && held)
            {
                bgColor = colors.Pressed(Spec::ColorRole::primary, Spec::ColorRole::onPrimary);
            }
            else if (hovered)
            {
                bgColor = colors.Hovered(Spec::ColorRole::secondaryContainer, Spec::ColorRole::onSecondaryContainer);
            }
            else
            {
                bgColor = selected ? colors[Spec::ColorRole::primary] : colors[Spec::ColorRole::secondaryContainer];
            }
            ImGui::RenderFrame(
                iconBgRect.Min, iconBgRect.Max, ImGui::ColorConvertFloat4ToU32(bgColor), true, iconSize.x
            );
        }
        drawList->AddText(
            m3Styles.IconFont(),
            m3Styles.IconSize(),
            iconPosMin,
            ImGui::ColorConvertFloat4ToU32(iconColor),
            TextStart(icon),
            TextEnd(icon),
            0.0F,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PushFont(nullptr, m3Styles.LabelText().textSize);
    {
        ImVec2       labelMin{bb.Min.x, iconPosMax.y + m3Styles[Spacing::Double_XS]};
        const ImVec2 labelMax{bb.Max.x, labelMin.y + m3Styles.LabelText().textSize};
        const auto   textSize    = ImGui::CalcTextSize(TextStart(label), TextEnd(label));
        const auto   fineClipOpt = TextClip(textSize, labelMin, bb);
        // modify labelMin
        AlignText(labelMin, {ALIGN_CENTER, ALIGN_CENTER}, labelMax, textSize);
        const auto &textColor =
            selected ? colors[Spec::ColorRole::secondary] : colors[Spec::ColorRole::onSurfaceVariant];
        drawList->AddText(
            nullptr,
            0.F,
            labelMin,
            ImGui::ColorConvertFloat4ToU32(textColor),
            TextStart(label),
            TextEnd(label),
            0.0F,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PopFont();
    return pressed;
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
        const auto minContentHeight = m3Styles.GetPixels(Spec::List::minHeight) - (contentOffset.y * 2);
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
        // only handle hover staus because the click should handled by inner content.
        if (contentRect.Max.x > contentRect.Min.x)
        {
            auto &g       = *GImGui;
            bool  hovered = (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) != 0;

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
        const auto offset = HalfLineGap({.textSize = lineHeight, .lineHeight = window->DC.CurrLineSize.y});
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
