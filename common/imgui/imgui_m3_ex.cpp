//
// Created by jamie on 2026/1/24.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "common/imgui/imgui_m3_ex.h"

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/Material3.h"
#include "imgui.h"
#include "imgui_internal.h"

#if IMGUI_VERSION_NUM != 19259
    #error "ImGui version changed! imgui_m3_ex only supports v1.92.6WIP"
#endif

namespace ImGuiEx::M3
{

constexpr float CENTER_ALIGN = 0.5f;

static void AlignText(ImVec2 &posMin, const ImVec2 &align, const ImVec2 &posMax, const ImVec2 &textSize)
{
    // copy from imgui.cpp#3857
    if (align.x > 0.0f) posMin.x = ImMax(posMin.x, posMin.x + (posMax.x - posMin.x - textSize.x) * align.x);
    if (align.y > 0.0f) posMin.y = ImMax(posMin.y, posMin.y + (posMax.y - posMin.y - textSize.y) * align.y);
}

[[nodiscard]] static auto TextClip(const ImVec2 &textSize, const ImVec2 &pos, const ImRect &clipRect)
    -> std::optional<ImVec4>
{
    // copy from imgui.cpp#3847
    // Perform CPU side clipping for single clipped element to avoid using scissor state
    bool need_clipping = pos.x + textSize.x >= clipRect.Max.x || pos.y + textSize.y >= clipRect.Max.y;
    need_clipping |= pos.x < clipRect.Min.x || pos.y < clipRect.Min.y;
    if (need_clipping)
    {
        return ImVec4(clipRect.Min.x, clipRect.Min.y, clipRect.Max.x, clipRect.Max.y);
    }
    return std::nullopt;
}

void LineTextUnformatted(const std::string_view &text, const float lineHeight)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    auto &g = *GImGui;
    if (auto offset = lineHeight - g.FontSize; offset <= 0)
    {
        ImGui::TextUnformatted(TextStart(text), TextEnd(text));
    }
    else
    {
        auto backup                       = window->DC.CurrLineTextBaseOffset;
        window->DC.CurrLineTextBaseOffset = offset * HALF;
        ImGui::TextUnformatted(TextStart(text), TextEnd(text));
        window->DC.PrevLineTextBaseOffset = backup;
    }
}

/**
 * Since the width of the navigation track is locked, a simple centered layout is all that's needed.
 */
void DrawNavMenu(std::string_view icon, const M3Styles &m3Styles)
{
    ImGui::Dummy({0.f, m3Styles[Spacing::L]});
    ImGui::TextAligned(CENTER_ALIGN, -FLT_MIN, "%s", TextStart(icon));
    ImGui::Dummy({0.f, m3Styles[Spacing::L]});
}

auto DrawNavItem(
    const std::string_view label, const bool selected, const std::string_view icon, const M3Styles &m3Styles
) -> bool
{
    StyleGuard styleGuard;
    styleGuard.Style_ItemSpacing({0.f, m3Styles[Spacing::XS]});
    const auto   regionLT   = ImGui::GetCursorScreenPos();
    const float  itemWidth  = ImGui::GetContentRegionAvail().x;
    const auto   iconSize   = ImVec2{m3Styles.IconSize(), m3Styles.IconSize()};
    const float  itemHeight = m3Styles.LabelText().fontSize + iconSize.y + m3Styles[Spacing::XS] * 4;
    const ImRect bb(regionLT, {regionLT.x + itemWidth, regionLT.y + itemHeight});
    const auto   id = ImGui::GetID(TextStart(label));
    ImGui::ItemSize({itemWidth, itemHeight}, m3Styles[Spacing::XS]);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    bool       hovered = false, held = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    auto &colors    = m3Styles.Colors();
    auto  iconColor = colors[ContentToken::onSurfaceVariant];

    ImVec2 iconPosMin{bb.Min.x, bb.Min.y + m3Styles[Spacing::XS]};
    ImVec2 iconPosMax{bb.Max.x, iconPosMin.y + iconSize.y};
    {
        auto fineClipOpt = TextClip(iconSize, iconPosMin, bb);
        // modify iconPosMin
        AlignText(iconPosMin, {CENTER_ALIGN, CENTER_ALIGN}, iconPosMax, iconSize);
        iconPosMax = iconPosMin + iconSize;
        // icon bg rect
        if (selected || hovered || held)
        {
            const auto offset     = ImVec2(m3Styles[Spacing::L], m3Styles[Spacing::XS]);
            const auto iconBgRect = ImRect(iconPosMin - offset, iconPosMin + iconSize + offset);
            if (selected) iconColor = colors[ContentToken::onPrimary];
            SurfaceColor bgColor;
            if (hovered && held)
            {
                bgColor = colors.Pressed(SurfaceToken::primary, ContentToken::onPrimary);
            }
            else if (hovered)
            {
                bgColor = colors.Hovered(SurfaceToken::secondaryContainer, ContentToken::onSecondaryContainer);
            }
            else
            {
                bgColor = selected ? colors[SurfaceToken::primary] : colors[SurfaceToken::secondaryContainer];
            }
            ImGui::RenderFrame(
                iconBgRect.Min, iconBgRect.Max, ImGui::ColorConvertFloat4ToU32(bgColor), true, iconSize.x
            );
        }
        drawList->AddText(
            m3Styles.IconFont(),
            m3Styles.IconSize(),
            iconPosMin,
            static_cast<ImU32>(iconColor),
            TextStart(icon),
            TextEnd(icon),
            0.0f,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PushFont(nullptr, m3Styles.LabelText().fontSize);
    {
        ImVec2       labelMin{bb.Min.x, iconPosMax.y + m3Styles[Spacing::Double_XS]};
        const ImVec2 labelMax{bb.Max.x, labelMin.y + m3Styles.LabelText().fontSize};
        const auto   textSize    = ImGui::CalcTextSize(TextStart(label), TextEnd(label));
        const auto   fineClipOpt = TextClip(textSize, labelMin, bb);
        // modify labelMin
        AlignText(labelMin, {CENTER_ALIGN, CENTER_ALIGN}, labelMax, textSize);
        const ColorBase &textColor = selected ? static_cast<const ColorBase &>(colors[SurfaceToken::secondary])
                                              : static_cast<const ColorBase &>(colors[ContentToken::onSurfaceVariant]);
        drawList->AddText(
            nullptr,
            0.f,
            labelMin,
            ImGui::ColorConvertFloat4ToU32(textColor),
            TextStart(label),
            TextEnd(label),
            0.0f,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PopFont();
    return pressed;
}

auto DrawIconButton(
    const std::string_view icon, const M3Styles &m3Styles, SurfaceToken surfaceColorToken,
    ContentToken contentColorToken, const SizeTips sizeTips
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    auto padding  = m3Styles[Spacing::XS];
    auto margin   = 0.f;
    auto iconSize = 0.f;
    auto rounding = 0.f;
    switch (sizeTips)
    {
        case SizeTips::XSMALL:
            margin   = m3Styles[Spacing::S];
            iconSize = m3Styles[Spacing::L];
            rounding = m3Styles.GetUnit(2);
            break;
        case SizeTips::SMALL:
            margin   = m3Styles[Spacing::XS];
            padding  = m3Styles[Spacing::S];
            iconSize = m3Styles[Spacing::XL];
            rounding = m3Styles.GetUnit(2);
            break;
        case SizeTips::MEDIUM:
            padding  = m3Styles[Spacing::L];
            iconSize = m3Styles[Spacing::XL];
            rounding = m3Styles.GetUnit(3);
            break;
        case SizeTips::LARGE:
            padding  = m3Styles[Spacing::XXL];
            iconSize = m3Styles[Spacing::XXL];
            rounding = m3Styles.GetUnit(4);
            break;
        case SizeTips::XLARGE:
            padding  = m3Styles[Spacing::PADDING_XL];
            iconSize = m3Styles.GetUnit(10);
            rounding = m3Styles[Spacing::L];
            break;
    }

    const ImGuiID id    = window->GetID(TextStart(icon));
    const ImVec2  pos   = window->DC.CursorPos;
    const auto    width = iconSize + padding * 2 + margin * 2;
    const ImVec2  size  = {width, width};

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, padding + margin);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool       hovered = false, held = false;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Render
    auto containerColor = m3Styles.Colors().at(surfaceColorToken);
    auto textColor      = m3Styles.Colors().at(contentColorToken);

    ImVec4 frameColor;
    if (const ImGuiContext &g = *GImGui; g.CurrentItemFlags & ImGuiItemFlags_Disabled)
    {
        textColor  = m3Styles.Colors().at(ContentToken::onSurface);
        frameColor = m3Styles.Colors().at(ContentToken::onSurface) * SurfaceColor::DISABLED_OPACITY;
        textColor  = ContentColor(textColor * ContentColor::DISABLED_OPACITY);
    }
    else if (hovered && held)
    {
        frameColor = containerColor.Pressed(textColor);
    }
    else if (hovered)
    {
        frameColor = containerColor.Hovered(textColor);
    }
    else
    {
        frameColor = containerColor;
    }

    const ImVec2 contentMin = bb.Min + ImVec2{margin, margin};
    const ImVec2 contentMax = bb.Max - ImVec2{margin, margin};
    ImGui::RenderNavCursor(bb, id);
    ImGui::RenderFrame(contentMin, contentMax, ImGui::ColorConvertFloat4ToU32(frameColor), true, rounding);

    ImVec2 posMin = contentMin;
    AlignText(posMin, {CENTER_ALIGN, CENTER_ALIGN}, contentMax, ImVec2(iconSize, iconSize));
    window->DrawList->AddText(
        m3Styles.IconFont(), iconSize, posMin, ImGui::ColorConvertFloat4ToU32(textColor), TextStart(icon), TextEnd(icon)
    );
    return pressed;
}

auto BeginDockedToolbar(
    const ImVec2 &buttonSize, const uint8_t count, const SurfaceToken surfaceToken, const M3Styles &m3Styles
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const auto margin = m3Styles[Spacing::M];
    const auto size = ImVec2{ImGui::GetContentRegionAvail().x, buttonSize.y + m3Styles[Spacing::Double_S] + margin * 2};
    ImRect     bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0)) return false;

    bb.Min.y += margin;
    bb.Max.y -= margin;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, static_cast<ImU32>(m3Styles.Colors().at(surfaceToken)));

    const auto minWidth = buttonSize.x * static_cast<float>(count);
    auto       gap      = (size.x - minWidth - m3Styles[Spacing::Double_L]) / (count - 1);
    gap                 = std::max(gap, 0.f);
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, gap);
    ImGui::SetNextItemAllowOverlap();

    window->DC.CursorPos = bb.Min + ImVec2(m3Styles[Spacing::L], m3Styles[Spacing::S]);
    return true;
}

auto EndDockedToolbar() -> void
{
    ImGui::PopStyleVar();
}

void SetItemToolTip(std::string_view text, const M3Styles &m3Styles)
{
    const auto &labelText = m3Styles.LabelText();
    StyleGuard  styleGuard;
    styleGuard.Color_Text(m3Styles.Colors().at(ContentToken::inverseOnSurface))
        .Color_PopupBg(m3Styles.Colors().at(SurfaceToken::inverseSurface))
        .Style_WindowPadding({m3Styles[Spacing::S], m3Styles[Spacing::XS] + HalfLineGap(labelText)});
    ImGui::PushFont(nullptr, labelText.fontSize);
    ImGui::SetItemTooltip("%s", TextStart(text));
    ImGui::PopFont();
}
} // namespace ImGuiEx::M3
