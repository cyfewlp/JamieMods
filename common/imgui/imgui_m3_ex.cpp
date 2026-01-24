//
// Created by jamie on 2026/1/24.
//
#include "common/imgui/imgui_m3_ex.h"

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

void AlignText(ImVec2 &posMin, const ImVec2 &align, const ImVec2 &posMax, const ImVec2 &textSize)
{
    // copy from imgui.cpp#3857
    if (align.x > 0.0f) posMin.x = ImMax(posMin.x, posMin.x + (posMax.x - posMin.x - textSize.x) * align.x);
    if (align.y > 0.0f) posMin.y = ImMax(posMin.y, posMin.y + (posMax.y - posMin.y - textSize.y) * align.y);
}

[[nodiscard]] auto TextClip(const ImVec2 &textSize, const ImVec2 &pos, const ImRect &clipRect) -> std::optional<ImVec4>
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
    ImU32 iconColor = colors.on_surface_variant;

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
            if (selected) iconColor = colors.on_secondary_container;
            ImVec4 bgColor;
            if (hovered && held)
            {
                bgColor = Colors::GetStateColor(colors.secondary_container, colors.on_secondary_container, 0.12f);
            }
            else if (hovered)
            {
                bgColor = Colors::GetStateColor(colors.secondary_container, colors.on_secondary_container, 0.08f);
            }
            else
                bgColor = colors.secondary_container;
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
        // modify iconPosMin
        AlignText(labelMin, {CENTER_ALIGN, CENTER_ALIGN}, labelMax, textSize);
        drawList->AddText(
            nullptr,
            0.f,
            labelMin,
            selected ? colors.secondary : colors.on_surface_variant,
            label.data(),
            label.data() + icon.size(),
            0.0f,
            fineClipOpt ? &fineClipOpt.value() : nullptr
        );
    }

    ImGui::PopFont();
    return pressed;
}
}
}