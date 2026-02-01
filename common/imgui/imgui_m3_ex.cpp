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

namespace LIBC_NAMESPACE_DECL
{
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

inline auto GetFontSize(float fontSizeBase) -> float
{
    auto &style = ImGui::GetStyle();
    return fontSizeBase * style.FontScaleMain * style.FontScaleDpi;
}

void LineTextUnformatted(const std::string_view &text, const float lineHeight)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    auto &g = *GImGui;
    if (auto offset = lineHeight - g.FontSize; offset <= 0)
    {
        ImGui::TextUnformatted(text.data(), text.data() + text.size());
    }
    else
    {
        auto backup                       = window->DC.CurrLineTextBaseOffset;
        window->DC.CurrLineTextBaseOffset = offset * 0.5f;
        ImGui::TextUnformatted(text.data(), text.data() + text.size());
        window->DC.PrevLineTextBaseOffset = backup;
    }
}

/**
 * Since the width of the navigation track is locked, a simple centered layout is all that's needed.
 */
void DrawNavMenu(std::string_view icon, const M3Styles &m3Styles)
{
    ImGui::Dummy({0.f, m3Styles[Spacing::L]});
    ImGui::TextAligned(CENTER_ALIGN, -FLT_MIN, "%s", icon.data());
    ImGui::Dummy({0.f, m3Styles[Spacing::L]});
}

auto DrawNavItem(
    const std::string_view label, const bool selected, const std::string_view icon, const M3Styles &m3Styles
) -> bool
{
    StyleGuard styleGuard;
    styleGuard.Push(StyleHolder::ItemSpacing({0.f, m3Styles[Spacing::XS]}));
    const auto   regionLT   = ImGui::GetCursorScreenPos();
    const float  itemWidth  = ImGui::GetContentRegionAvail().x;
    const float  itemHeight = m3Styles.GetMediumText().fontSize + m3Styles.GetIconSize() + m3Styles[Spacing::XS] * 4;
    const ImRect bb(regionLT, {regionLT.x + itemWidth, regionLT.y + itemHeight});
    const auto   id = ImGui::GetID(label.data());
    ImGui::ItemSize({itemWidth, itemHeight}, m3Styles[Spacing::XS]);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    bool       hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    auto &colors    = m3Styles.colors;
    ImU32 iconColor = colors.Get(ContentToken::onSurfaceVariant);

    ImVec2       iconPosMin{bb.Min.x, bb.Min.y + m3Styles[Spacing::XS]};
    const ImVec2 iconPosMax{bb.Max.x, iconPosMin.y + m3Styles.GetIconSize()};
    {
        const ImVec2 iconSize{m3Styles.GetIconSize(), m3Styles.GetIconSize()};
        auto         fineClipOpt = TextClip(iconSize, iconPosMin, bb);
        // modify iconPosMin
        AlignText(iconPosMin, {CENTER_ALIGN, CENTER_ALIGN}, iconPosMax, iconSize);
        // icon bg rect
        if (selected || hovered || held)
        {
            const auto offset     = ImVec2(m3Styles[Spacing::L], m3Styles[Spacing::XS]);
            auto       iconBgRect = ImRect(iconPosMin - offset, iconPosMin + iconSize + offset);
            if (selected) iconColor = colors[ContentToken::onPrimary];
            ImVec4 bgColor;
            if (hovered && held)
            {
                bgColor = colors[SurfaceToken::primary].Pressed(colors[ContentToken::onPrimary]);
            }
            else if (hovered)
            {
                bgColor = colors[SurfaceToken::secondaryContainer].Hovered(colors[ContentToken::onSecondaryContainer]);
            }
            else
            {
                bgColor = selected ? colors[SurfaceToken::primary] : colors[SurfaceToken::secondaryContainer];
            }
            ImGui::RenderFrame(iconBgRect.Min, iconBgRect.Max, ImGui::GetColorU32(bgColor), true, 16.f);
        }
        drawList->AddText(
            m3Styles.iconFont,
            m3Styles.GetIconSize(),
            iconPosMin,
            iconColor,
            icon.data(),
            icon.data() + icon.size(),
            0.0f,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PushFont(nullptr, m3Styles.GetMediumText().fontSize);
    ImVec2       labelMin{bb.Min.x, iconPosMax.y + m3Styles[Spacing::Double_XS]};
    const ImVec2 labelMax{bb.Max.x, labelMin.y + m3Styles.GetMediumText().fontSize};
    const auto   labelEnd = label.data() + label.size();
    const auto   textSize = ImGui::CalcTextSize(label.data(), labelEnd);
    {
        auto fineClipOpt = TextClip(textSize, labelMin, bb);
        // modify labelMin
        AlignText(labelMin, {CENTER_ALIGN, CENTER_ALIGN}, labelMax, textSize);
        drawList->AddText(
            nullptr,
            0.f,
            labelMin,
            selected ? colors[SurfaceToken::secondary] : colors[ContentToken::onSurfaceVariant],
            label.data(),
            label.data() + label.size(),
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
            padding  = m3Styles.GetUnit(12);
            iconSize = m3Styles.GetUnit(10);
            rounding = m3Styles.GetUnit(4);
            break;
    }

    const ImGuiID id    = window->GetID(icon.data());
    const ImVec2  pos   = window->DC.CursorPos;
    const auto    width = iconSize + padding * 2 + margin * 2;
    const ImVec2  size  = {width, width};

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, padding + margin);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool       hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Render
    auto containerColor = m3Styles.colors.Get(surfaceColorToken);
    auto textColor      = m3Styles.colors.Get(contentColorToken);

    ImGuiContext &g = *GImGui;
    ImU32         frameColor;
    if (g.CurrentItemFlags & ImGuiItemFlags_Disabled)
    {
        textColor     = m3Styles.colors.Get(ContentToken::onSurface);
        textColor.raw = textColor.raw * ContentColor::DISABLED_OPACITY;
        frameColor    = ImGui::ColorConvertFloat4ToU32(textColor.raw * SurfaceColor::DISABLED_OPACITY);
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
    ImGui::RenderFrame(contentMin, contentMax, frameColor, true, rounding);

    ImVec2 posMin = contentMin;
    AlignText(posMin, {CENTER_ALIGN, CENTER_ALIGN}, contentMax, ImVec2(iconSize, iconSize));
    window->DrawList->AddText(m3Styles.iconFont, iconSize, posMin, textColor, icon.data(), icon.data() + icon.size());
    return pressed;
}

auto BeginDockedToolbar(
    const ImVec2 &buttonSize, uint8_t count, const SurfaceToken surfaceToken, const M3Styles &m3Styles
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
    window->DrawList->AddRectFilled(bb.Min, bb.Max, m3Styles.colors[surfaceToken]);

    const auto minWidth = buttonSize.x * count;
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
    const auto smallText = m3Styles.GetSmallText();
    StyleGuard styleGuard;
    styleGuard.Push(ColorHolder::Text(m3Styles.colors[ContentToken::inverseOnSurface]))
        .Push(ColorHolder::PopupBg(m3Styles.colors[SurfaceToken::inverseSurface]))
        .Push(StyleHolder::WindowPadding({m3Styles[Spacing::S], m3Styles[Spacing::XS] + smallText.GetTextSpacing()}));
    ImGui::PushFont(nullptr, smallText.fontSize);
    ImGui::SetItemTooltip("%s", text.data());
    ImGui::PopFont();
}
}
}