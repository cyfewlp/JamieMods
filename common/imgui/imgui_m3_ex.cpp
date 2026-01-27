//
// Created by jamie on 2026/1/24.
//

#define IMGUI_DEFINE_MATH_OPERATORS
#include "common/imgui/imgui_m3_ex.h"

#include "common/imgui/Material3.h"
#include "imgui.h"
#include "imgui_internal.h"

#if IMGUI_VERSION_NUM != 19259 // 假设你当前基于 1.91.5 开发
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
void DrawNavMenu(std::string_view icon)
{

    ImGui::Dummy({0.f, NavigationRail::Standard.padding});
    ImGui::TextAligned(CENTER_ALIGN, -FLT_MIN, "%s", icon.data());
    ImGui::Dummy({0.f, NavigationRail::Standard.padding});
}

auto DrawNavItem(
    const std::string_view label, const bool selected, const std::string_view icon, const M3Styles &m3Styles
) -> bool
{
    StyleGuard     styleGuard;
    constexpr auto rail = NavigationRail::Standard;
    styleGuard.Push(StyleHolder::ItemSpacing(rail.spacing));
    const auto      regionLT   = ImGui::GetCursorScreenPos();
    const float     itemWidth  = ImGui::GetContentRegionAvail().x;
    constexpr float itemHeight = rail.fontSize + rail.iconSize + rail.spacing.y * 4;
    const ImRect    bb(regionLT, {regionLT.x + itemWidth, regionLT.y + itemHeight});
    const auto      id = ImGui::GetID(label.data());
    ImGui::ItemSize({itemWidth, itemHeight}, rail.spacing.y);
    if (!ImGui::ItemAdd(bb, id))
    {
        return false;
    }

    bool       hovered, held;
    const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    // ImGui::RenderFrame(bb.Min, bb.Max, m3Styles.colors.surface_container);

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    auto &colors    = m3Styles.colors;
    ImU32 iconColor = colors.onSurfaceVariant;

    ImVec2       iconPosMin{bb.Min.x, bb.Min.y + rail.spacing.y};
    const ImVec2 iconPosMax{bb.Max.x, iconPosMin.y + rail.iconSize};
    {
        constexpr ImVec2 iconSize{rail.iconSize, rail.iconSize};
        auto             fineClipOpt = TextClip(iconSize, iconPosMin, bb);
        // modify iconPosMin
        AlignText(iconPosMin, {CENTER_ALIGN, CENTER_ALIGN}, iconPosMax, iconSize);
        // icon bg rect
        if (selected || hovered || held)
        {
            auto iconBgRect = ImRect(
                ImVec2{iconPosMin.x - 16.f, iconPosMin.y - rail.spacing.y},
                ImVec2(iconPosMin.x + iconSize.x + 16.f, iconPosMin.y + iconSize.y + rail.spacing.y)
            );
            if (selected) iconColor = colors.onPrimary;
            ImVec4 bgColor;
            if (hovered && held)
            {
                bgColor = Colors::GetActiveColor(colors.primary, colors.onPrimary);
            }
            else if (hovered)
            {
                bgColor = Colors::GetHoveredColor(colors.secondaryContainer, colors.onSecondaryContainer);
            }
            else
            {
                bgColor = selected ? colors.primary : colors.secondaryContainer;
            }
            ImGui::RenderFrame(iconBgRect.Min, iconBgRect.Max, ImGui::GetColorU32(bgColor), true, 16.f);
        }
        drawList->AddText(
            m3Styles.iconFont,
            GetFontSize(rail.iconSize),
            iconPosMin,
            iconColor,
            icon.data(),
            icon.data() + icon.size(),
            0.0f,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PushFont(nullptr, rail.fontSize);
    ImVec2       labelMin{bb.Min.x, iconPosMax.y + rail.spacing.y * 2.f};
    const ImVec2 labelMax{bb.Max.x, labelMin.y + rail.fontSize};
    const auto   labelEnd = label.data() + label.size();
    const auto   textSize = ImGui::CalcTextSize(label.data(), labelEnd);
    ImGui::RenderTextClipped(labelMin, labelMax, label.data(), labelEnd, &textSize, {CENTER_ALIGN, CENTER_ALIGN}, &bb);
    {
        auto fineClipOpt = TextClip(textSize, labelMin, bb);
        // modify labelMin
        AlignText(labelMin, {CENTER_ALIGN, CENTER_ALIGN}, labelMax, textSize);
        drawList->AddText(
            nullptr,
            0.f,
            labelMin,
            selected ? colors.secondary : colors.onSurfaceVariant,
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
    const std::string_view icon, const ImU32 &containerColor, const ImU32 &textColor, ImFont *iconFont,
    const ButtonSpec &spec
) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext     &g        = *GImGui;
    const ImGuiStyle &style    = g.Style;
    const ImGuiID     id       = window->GetID(icon.data());
    const ImVec2      iconSize = {spec.fontSize, spec.fontSize};

    ImVec2       pos  = window->DC.CursorPos;
    const ImVec2 size = iconSize + spec.padding * 2 + spec.spacing * 2;

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Render
    ImU32 frameColor = containerColor;
    if (hovered && held)
    {
        frameColor = Colors::GetActiveColor(frameColor, textColor);
    }
    else if (hovered)
    {
        frameColor = Colors::GetHoveredColor(frameColor, textColor);
    }

    const ImVec2 contentMin = bb.Min + spec.spacing;
    const ImVec2 contentMax = bb.Max - spec.spacing;
    ImGui::RenderNavCursor(bb, id);
    ImGui::RenderFrame(contentMin, contentMax, frameColor, true, spec.rounding);

    ImVec2 posMin = contentMin;
    AlignText(posMin, {CENTER_ALIGN, CENTER_ALIGN}, contentMax, iconSize);
    window->DrawList->AddText(
        iconFont, GetFontSize(spec.fontSize), posMin, textColor, icon.data(), icon.data() + icon.size()
    );
    return pressed;
}

auto DrawIconButton(std::string_view icon, const ButtonSpec &spec, const M3Styles &m3Styles) -> bool
{
    return DrawIconButton(icon, m3Styles.colors.primary, m3Styles.colors.onPrimary, m3Styles.iconFont, spec);
}

auto BeginDockedToolbar(const ImVec2 &buttonSize, const uint8_t count, const ImU32 bgColor) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const auto   size = ImVec2{ImGui::GetContentRegionAvail().x, buttonSize.y + Toolbar::DOCKED.padding.y * 2};
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0)) return false;

    window->DrawList->AddRectFilled(bb.Min, bb.Max, bgColor);

    const auto minWidth = buttonSize.x * count;
    auto       gap      = (size.x - minWidth - Toolbar::DOCKED.padding.x * 2.f) / (count - 1);
    gap                 = std::max(gap, 0.f);
    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, gap);
    ImGui::SetNextItemAllowOverlap();

    window->DC.CursorPos = bb.Min + Toolbar::DOCKED.padding;
    return true;
}

auto BeginDockedToolbar(float buttonSize, const uint8_t count, const ImU32 bgColor) -> bool
{
    return BeginDockedToolbar(ImVec2{buttonSize, buttonSize}, count, bgColor);
}

auto EndDockedToolbar() -> void
{
    ImGui::PopStyleVar();
}

void SetItemToolTip(std::string_view text, const Colors &colors)
{
    StyleGuard styleGuard;
    styleGuard.Push(ColorHolder::Text(colors.inverseOnSurface))
        .Push(ColorHolder::PopupBg(colors.inverseSurface))
        .Push(StyleHolder::WindowPadding(Tooltip::PLAIN.GetFullPadding()));
    ImGui::PushFont(nullptr, Tooltip::PLAIN.textSize.fontSize);
    ImGui::SetItemTooltip("%s", text.data());
    ImGui::PopFont();
}
}
}