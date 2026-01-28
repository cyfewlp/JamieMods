//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/M3ThemeBuilder.h"
#include "common/imgui/Material3.h"

#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
inline auto argbToImVec4(uint32_t a_argb) -> ImVec4
{
    return ImVec4(
        ((a_argb & 0xFF0000) >> 16) / 255.f,
        ((a_argb & 0xFF00) >> 8) / 255.f,
        (a_argb & 0xFF) / 255.f,
        ((a_argb & 0xFF000000) >> 24) / 255.f
    );
};

void AppearancePanel::Draw(const bool appearing)
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Push(ImGuiEx::StyleHolder::WindowPadding(ImGuiEx::M3::CUSTOM_WINDOW_PADDING2))
        .Push(
            ImGuiEx::StyleHolder::ItemSpacing(
                {ImGuiEx::M3::List::STANDARD.gap, ImGuiEx::M3::List::STANDARD.padding.y * 2.0f}
            )
        )
        .Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSurface()))
        .Push(ImGuiEx::ColorHolder::ChildBg(m_styles.colors.Surface()));
    if (ImGui::BeginChild("##Appearance", {}, ImGuiEx::ChildFlags().AlwaysUseWindowPadding()))
    {
        if (appearing)
        {
            m_colorInThemeBuilder = argbToImVec4(m_styles.colors.SeedArgb());
        }
        if (ImGui::BeginTable("CenterAlignTable", 3, ImGuiEx::TableFlags().SizingStretchSame()))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            DrawZoomCombo();
            ImGui::Spacing();
            DrawThemeBuilder();
            ImGui::TableNextColumn();
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void AppearancePanel::DrawZoomCombo() const
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Push(ImGuiEx::ColorHolder::Text(m_styles.colors.OnSurfaceVariant()))
        .Push(ImGuiEx::StyleHolder::FrameBorderSize(4.f))
        .Push(ImGuiEx::ColorHolder::Border(m_styles.colors.Primary()))
        .Push(ImGuiEx::ColorHolder::FrameBg(m_styles.colors.Surface()))
        .Push(
            ImGuiEx::ColorHolder::FrameBgHovered(
                m_styles.colors.Surface().GetHoveredState(m_styles.colors.OnSurfaceVariant())
            )
        )
        .Push(ImGuiEx::ColorHolder::PopupBg(m_styles.colors.SurfaceContainerLow()))
        .Push(ImGuiEx::ColorHolder::HeaderActive(m_styles.colors.TertiaryContainer()))
        .Push(
            ImGuiEx::ColorHolder::HeaderHovered(
                m_styles.colors.SurfaceContainerLow().GetHoveredState(m_styles.colors.OnSurface())
            )
        );
    auto width = ImGui::GetContentRegionAvail().x;
    if (width > ImGuiEx::M3::CUSTOM_STANDARD_MENU_WIDTH)
    {
        ImGui::SetNextItemWidth(ImGuiEx::M3::CUSTOM_STANDARD_MENU_WIDTH);
    }
    if (ImGui::BeginCombo("Zoom", "100%", ImGuiEx::ComboFlags().NoArrowButton()))
    {
        ImGui::Selectable("50%");
        ImGui::Selectable("100%");
        ImGui::Selectable("125%");
        ImGui::Selectable("150%");
        ImGui::Selectable("175%");
        ImGui::Selectable("200%");
        ImGui::EndCombo();
    }
}

void AppearancePanel::DrawThemeBuilder()
{
    using namespace ImGuiEx;
    constexpr auto medium = M3::TextSize::MEDIUM;
    StyleGuard     styleGuard;
    styleGuard.Push(StyleHolder::FramePadding({0.f, medium.GetTextSpacing()}))
        .Push(ColorHolder::ChildBg(m_styles.colors.Surface()));

    ImGui::PushFont(nullptr, medium.fontSize);
    constexpr auto flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
    bool           openPopup = ImGui::ColorButton("##SeedColor", argbToImVec4(m_styles.colors.SeedArgb()), flags);
    ImGui::SameLine();
    ImGui::PushFont(nullptr, M3::TextSize::LARGE.fontSize);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Source color");
    ImGui::PopFont();

    if (openPopup)
    {
        ImGui::OpenPopup("seedColorPicker");
    }
    {
        StyleGuard styleGuard1;
        styleGuard1.Push(StyleHolder::FramePadding({0.f, medium.GetTextSpacing()}))
            .Push(ColorHolder::PopupBg(m_styles.colors.SurfaceContainerHigh()));
        if (ImGui::BeginPopup("seedColorPicker"))
        {
            // setup DragInt/DragScalar styles
            StyleGuard styleGuard2;
            styleGuard2.Push(ColorHolder::Text(m_styles.colors.OnPrimaryContainer()))
                .Push(ColorHolder::FrameBg(m_styles.colors.PrimaryContainer()))
                .Push(
                    ColorHolder::FrameBgHovered(
                        m_styles.colors.PrimaryContainer().GetHoveredState(m_styles.colors.OnPrimaryContainer())
                    )
                )
                .Push(
                    ColorHolder::FrameBgActive(
                        m_styles.colors.PrimaryContainer().GetPressedState(m_styles.colors.OnPrimaryContainer())
                    )
                );
            ImGui::ColorPicker4(
                "##picker",
                reinterpret_cast<float *>(&m_colorInThemeBuilder.Value),
                ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview
            );
            ImGui::SameLine();

            ImGui::BeginGroup();
            {
                styleGuard2.Push(ColorHolder::Text(m_styles.colors.OnSurface()));
                bool darkMode = m_styles.colors.DarkMode();
                ImGui::Checkbox(m_translation["$Theme_DarkMode"], &darkMode);

                constexpr auto buttonSpec = M3::Button::SMALL;
                styleGuard2.Push(ColorHolder::Text(m_styles.colors.OnPrimary()))
                    .Push(StyleHolder::FrameRounding(buttonSpec.rounding))
                    .Push(StyleHolder::FramePadding(buttonSpec.padding));
                if (ImGui::Button(m_translation["$Apply"]))
                {
                    auto imU32ToArgb = [](ImU32 imU32) -> uint32_t {
                        return (imU32 & 0xFF000000) | (imU32 & 0xFF) << 16 | (imU32 & 0xFF00) |
                               (imU32 & 0xFF0000) >> 16;
                    };
                    ApplyM3Theme(imU32ToArgb(m_colorInThemeBuilder), darkMode);
                }

                if (ImGui::Button(m_translation["$Cancel"]))
                {
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndGroup();

            styleGuard2.Pop();
            ImGui::EndPopup();
        }
    }

    ////

    ImGui::ColorButton("##Primary}", argbToImVec4(m_styles.colors.PrimaryPalette()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("Primary");

    ImGui::ColorButton("##Secondary}", argbToImVec4(m_styles.colors.SecondaryPalette()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("Secondary");

    ImGui::ColorButton("##Tertiary}", argbToImVec4(m_styles.colors.TertiaryPalette()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("Tertiary");

    ImGui::ColorButton("##Neutral}", argbToImVec4(m_styles.colors.NeutralPalette()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("Neutral");

    ImGui::ColorButton("##NeutralVariant}", argbToImVec4(m_styles.colors.NeutralVariantPalette()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("NeutralVariant");

    ImGui::ColorButton("##Error}", argbToImVec4(m_styles.colors.ErrorPalette()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("Error");

    ImGui::PopFont();
}

void AppearancePanel::ApplyM3Theme(uint32_t seedArgb, const bool darkMode)
{
    using namespace ImGuiEx::M3;
    auto &colors = m_styles.colors;

    ThemeBuilder::BuildThemeFromSeed(seedArgb, darkMode, colors);
    m_colorInThemeBuilder = ImColor(argbToImVec4(m_styles.colors.SeedArgb()));

    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = colors.OnSurface();

    style.Colors[ImGuiCol_TitleBg]          = colors.SurfaceContainerHighest();
    style.Colors[ImGuiCol_TitleBgActive]    = colors.SurfaceContainerHighest().GetPressedState(colors.OnSurface());
    style.Colors[ImGuiCol_TitleBgCollapsed] = colors.SecondaryContainer();

    style.Colors[ImGuiCol_WindowBg] = colors.Surface();
    style.Colors[ImGuiCol_ChildBg]  = colors.Surface();
    style.Colors[ImGuiCol_PopupBg]  = colors.PrimaryContainer();

    style.Colors[ImGuiCol_FrameBg]        = colors.SecondaryContainer();
    style.Colors[ImGuiCol_FrameBgActive]  = colors.SecondaryContainer().GetPressedState(colors.OnSecondaryContainer());
    style.Colors[ImGuiCol_FrameBgHovered] = colors.SecondaryContainer().GetHoveredState(colors.OnSecondaryContainer());
    style.Colors[ImGuiCol_Border]         = colors.OutlineVariant();
    style.Colors[ImGuiCol_BorderShadow]   = colors.OutlineVariant();

    style.Colors[ImGuiCol_SliderGrab]       = colors.Primary();
    style.Colors[ImGuiCol_SliderGrabActive] = colors.Primary().GetPressedState(colors.OnPrimary());

    style.Colors[ImGuiCol_Button]        = colors.Primary();
    style.Colors[ImGuiCol_ButtonHovered] = colors.Primary().GetHoveredState(colors.OnPrimary());
    style.Colors[ImGuiCol_ButtonActive]  = colors.Primary().GetPressedState(colors.OnPrimary());

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors.Outline();
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors.OutlineVariant();
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors.Primary();

    style.Colors[ImGuiCol_MenuBarBg] = colors.SurfaceContainerHigh();

    style.Colors[ImGuiCol_Header]        = colors.SurfaceContainerHigh();
    style.Colors[ImGuiCol_HeaderHovered] = colors.SurfaceContainerHigh().GetHoveredState(colors.OnSurface());
    style.Colors[ImGuiCol_HeaderActive]  = colors.SurfaceContainerHigh().GetPressedState(colors.OnSurface());

    style.Colors[ImGuiCol_Separator]        = colors.Secondary();
    style.Colors[ImGuiCol_SeparatorHovered] = colors.Secondary().GetHoveredState(colors.OnSecondary());
    style.Colors[ImGuiCol_SeparatorActive]  = colors.Secondary().GetPressedState(colors.OnSecondary());

    style.Colors[ImGuiCol_ResizeGrip] = colors.SecondaryContainer();
    style.Colors[ImGuiCol_ResizeGripHovered] =
        colors.SecondaryContainer().GetHoveredState(colors.OnSecondaryContainer());
    style.Colors[ImGuiCol_ResizeGripActive] =
        colors.SecondaryContainer().GetPressedState(colors.OnSecondaryContainer());

    style.Colors[ImGuiCol_InputTextCursor] = colors.Secondary();

    style.Colors[ImGuiCol_Tab]                 = colors.Surface();
    style.Colors[ImGuiCol_TabHovered]          = colors.Surface().GetHoveredState(colors.OnSurface());
    style.Colors[ImGuiCol_TabSelected]         = colors.Surface();
    style.Colors[ImGuiCol_TabSelectedOverline] = colors.Primary();

    style.Colors[ImGuiCol_TabDimmed]                 = colors.Surface();
    style.Colors[ImGuiCol_TabDimmedSelected]         = colors.Surface().GetPressedState(colors.OnSurface());
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors.OutlineVariant();

    style.Colors[ImGuiCol_PlotLines]        = colors.Primary();
    style.Colors[ImGuiCol_PlotLinesHovered] = colors.Primary().GetHoveredState(colors.OnPrimary());

    style.Colors[ImGuiCol_PlotHistogram]        = colors.Tertiary();
    style.Colors[ImGuiCol_PlotHistogramHovered] = colors.Tertiary().GetHoveredState(colors.OnTertiary());

    style.Colors[ImGuiCol_TableHeaderBg]     = colors.SurfaceContainerHigh();
    style.Colors[ImGuiCol_TableBorderStrong] = colors.Outline();
    style.Colors[ImGuiCol_TableBorderLight]  = colors.OutlineVariant();
    style.Colors[ImGuiCol_TableRowBg]        = colors.Surface();
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors.SurfaceContainerLowest();

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low;
    style.Colors[ImGuiCol_TextSelectedBg]   = colors.Primary();
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors.OnSurface();

    style.Colors[ImGuiCol_DragDropTarget]   = colors.Primary();
    style.Colors[ImGuiCol_DragDropTargetBg] = colors.Surface();

    style.Colors[ImGuiCol_UnsavedMarker]         = colors.OnPrimary();
    style.Colors[ImGuiCol_NavCursor]             = colors.OnSecondary();
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors.OnPrimary();
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors.SurfaceContainer();

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors.Surface();
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}
}
}