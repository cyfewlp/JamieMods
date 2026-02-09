//
// Created by jamie on 2026/2/8.
//
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imguiex_m3_slider.h"

#include "ImGuiEx.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imguiex_m3.h"

namespace ImGuiEx::M3::Slider
{
namespace
{
struct SliderLayout
{
    ImRect outer_bb;
    ImRect frame_bb;
    ImRect grab_bb; // filled by SliderBehavior
    ImVec2 label_size;
};

// Compute bounding rects and do ItemSize/ItemAdd.
// returns false if item was clipped/culled.
auto ComputeLayout(
    const ImGuiWindow *window, const ImGuiID id, const std::string_view label, const detail::Params &params,
    const M3Styles &m3Styles, SliderLayout &out
) -> bool
{
    const auto width  = ImGui::CalcItemWidth();
    const auto height = m3Styles.GetPixels(params.grabHeight);

    out.label_size          = ImGui::CalcTextSize(TextStart(label), TextEnd(label), true);
    out.outer_bb            = ImRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, height));
    const auto frame_offset = (height - m3Styles.GetPixels(params.frameHeight)) * HALF;
    out.frame_bb = ImRect(out.outer_bb.Min + ImVec2(0, frame_offset), out.outer_bb.Max - ImVec2(0, frame_offset));
    const ImRect total_bb = ImRect(
        out.outer_bb.Min,
        out.outer_bb.Max + ImVec2(out.label_size.x > 0.0F ? m3Styles[Spacing::XS] + out.label_size.x : 0.0F, 0.0F)
    );
    ImGui::ItemSize(total_bb);
    return ImGui::ItemAdd(total_bb, id, &out.frame_bb, 0);
}

// Handle activation / focus / nav behavior (keeps parity with original)
void HandleActivation(ImGuiID id, ImGuiWindow *window, const bool hovered)
{
    ImGuiContext &g = *GImGui;
    if (const bool temp_input_is_active = ImGui::TempInputIsActive(id); !temp_input_is_active)
    {
        const bool clicked     = hovered && ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);
        const bool make_active = (clicked || g.NavActivateId == id);
        if (make_active && clicked) ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        if (make_active)
        {
            ImGui::SetActiveID(id, window);
            ImGui::SetFocusID(id, window);
            ImGui::FocusWindow(window);
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }
    }
}

auto DrawFrame(
    ImDrawList *drawList, const SliderLayout &rects, const double value01, const ImVec4 &activeColor,
    const M3Styles &m3Styles
)
{
    const ImU32 frame_col     = ImGui::GetColorU32(m3Styles.Colors()[SurfaceToken::secondaryContainer]);
    const auto  centerX       = (rects.frame_bb.Max.x + rects.frame_bb.Min.x) * HALF;
    const auto  centerGapLeft = centerX - m3Styles[Spacing::S];
    // part1 secondary container
    if (value01 > 0)
    {
        drawList->AddRectFilled(
            rects.frame_bb.Min,
            {std::min(centerGapLeft, rects.grab_bb.Min.x), rects.frame_bb.Max.y},
            frame_col,
            m3Styles[Spacing::S],
            DrawFlags().RoundCornersTopLeft().RoundCornersBottomLeft()
        );
    }
    // part2 primary
    if (rects.grab_bb.Min.x > centerX || rects.grab_bb.Max.x < centerGapLeft)
    {
        drawList->AddRectFilled(
            {std::min(rects.grab_bb.Max.x, centerX), rects.frame_bb.Min.y},
            {std::max(centerGapLeft, rects.grab_bb.Min.x), rects.frame_bb.Max.y},
            ImGui::ColorConvertFloat4ToU32(activeColor),
            m3Styles[Spacing::XS]
        );
    }

    // part3 secondary container
    if (value01 < 1)
    {
        drawList->AddRectFilled(
            {std::max(rects.grab_bb.Max.x, centerX), rects.frame_bb.Min.y},
            rects.frame_bb.Max,
            frame_col,
            m3Styles[Spacing::S],
            DrawFlags().RoundCornersTopRight().RoundCornersBottomRight()
        );
    }
}

} // namespace

auto detail::Draw(std::string_view label, const Params &params, SliderFlags flags) -> bool
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    const auto   &g  = *GImGui;
    const ImGuiID id = window->GetID(TextStart(label), TextEnd(label));

    SliderLayout layout;
    const auto  &m3Styles = params.m3Styles;
    if (!ComputeLayout(window, id, label, params, m3Styles, layout))
    {
        return false;
    }

    const bool hovered = ImGui::ItemHoverable(layout.frame_bb, id, g.LastItemData.ItemFlags);
    HandleActivation(id, window, hovered);

    const auto *const format = ImGui::DataTypeGetInfo(params.dataType)->PrintFmt;

    // Slider behavior
    auto      &style           = ImGui::GetStyle();
    const auto prevGrabMinSize = style.GrabMinSize;
    style.GrabMinSize          = m3Styles.GetPixels(params.grabOuterWidth);
    const bool value_changed   = ImGui::SliderBehavior(
        layout.outer_bb,
        id,
        params.dataType,
        params.pValue,
        params.pMinValue,
        params.pMaxValue,
        format,
        flags,
        &layout.grab_bb
    );
    style.GrabMinSize = prevGrabMinSize;
    if (value_changed)
    {
        ImGui::MarkItemEdited(id);
    }

    ImGui::RenderNavCursor(layout.frame_bb, id);

    const bool activated   = g.ActiveId == id;
    const auto activeColor = activated ? m3Styles.Colors().Pressed(SurfaceToken::primary, ContentToken::onPrimary)
                                       : m3Styles.Colors()[SurfaceToken::primary];

    DrawFrame(window->DrawList, layout, params.value01, activeColor, m3Styles);

    const float grab_margin = m3Styles[Spacing::XS];
    window->DrawList->AddRectFilled(
        {layout.grab_bb.Min.x + grab_margin, layout.grab_bb.Min.y},
        {layout.grab_bb.Max.x - grab_margin, layout.grab_bb.Max.y},
        ImGui::ColorConvertFloat4ToU32(activeColor),
        m3Styles[Spacing::XS] * HALF
    );

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    if (g.LogEnabled) ImGui::LogSetNextTextDecoration("{", "}");
    if (activated)
    {
        std::array<char, 64> value_buf{};
        const auto           bufSize =
            ImGui::DataTypeFormatString(value_buf.data(), value_buf.size(), params.dataType, params.pValue, format);
        if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
        {
            ImGui::TextEx(value_buf.data(), &value_buf[bufSize], ImGuiTextFlags_NoWidthForLargeClippedText);
            ImGui::EndTooltip();
        }
    }

    if (layout.label_size.x > 0.0F)
    {
        ImGui::RenderText(
            ImVec2(layout.frame_bb.Max.x + m3Styles[Spacing::XS], layout.frame_bb.Min.y),
            TextStart(label),
            TextEnd(label)
        );
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(
        id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0)
    );
    return value_changed;
}
} // namespace ImGuiEx::M3::Slider
