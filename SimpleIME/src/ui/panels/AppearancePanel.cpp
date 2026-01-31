//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/M3ThemeBuilder.h"
#include "common/imgui/Material3.h"
#include "cpp/scheme/scheme_tonal_spot.h"
#include "i18n/TranslatorHolder.h"

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
}

void AppearancePanel::Draw(const bool appearing)
{
    using namespace ImGuiEx::M3;
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Push(ImGuiEx::StyleHolder::WindowPadding({m_styles[Spacing::L], m_styles[Spacing::L]}))
        .Push(ImGuiEx::StyleHolder::ItemSpacing({m_styles[Spacing::M], m_styles[Spacing::Double_M]}))
        .Push(ImGuiEx::ColorHolder::Text(m_styles.colors[ContentToken::onSurface]))
        .Push(ImGuiEx::ColorHolder::ChildBg(m_styles.colors[SurfaceToken::surface]));
    if (ImGui::BeginChild("##Appearance", {}, ImGuiEx::ChildFlags().AlwaysUseWindowPadding()))
    {
        if (appearing)
        {
            m_colorInThemeBuilder    = argbToImVec4(m_styles.colors.SeedArgb());
            m_darkModeInThemeBuilder = m_styles.colors.DarkMode();
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
    styleGuard.Push(ImGuiEx::ColorHolder::Text(m_styles.colors[ImGuiEx::M3::ContentToken::onSurfaceVariant]))
        .Push(ImGuiEx::StyleHolder::FrameBorderSize(4.f))
        .Push(ImGuiEx::ColorHolder::Border(m_styles.colors[ImGuiEx::M3::SurfaceToken::primary]))
        .Push(ImGuiEx::ColorHolder::FrameBg(m_styles.colors[ImGuiEx::M3::SurfaceToken::surface]))
        .Push(
            ImGuiEx::ColorHolder::FrameBgHovered(m_styles.colors[ImGuiEx::M3::SurfaceToken::surface].Hovered(
                m_styles.colors[ImGuiEx::M3::ContentToken::onSurfaceVariant]
            ))
        )
        .Push(ImGuiEx::ColorHolder::PopupBg(m_styles.colors[ImGuiEx::M3::SurfaceToken::surfaceContainerLow]))
        .Push(ImGuiEx::ColorHolder::HeaderActive(m_styles.colors[ImGuiEx::M3::SurfaceToken::tertiaryContainer]))
        .Push(
            ImGuiEx::ColorHolder::HeaderHovered(m_styles.colors[ImGuiEx::M3::SurfaceToken::surfaceContainerLow].Hovered(
                m_styles.colors[ImGuiEx::M3::ContentToken::onSurface]
            ))
        );
    const auto availX = ImGui::GetContentRegionAvail().x;
    if (const auto maxWidth = m_styles.GetSize(ImGuiEx::M3::ComponentSize::MENU_WIDTH); availX > maxWidth)
    {
        ImGui::SetNextItemWidth(maxWidth);
    }
    constexpr uint8_t             zoomUnit         = 25;
    static std::array<uint8_t, 6> zoomList         = {2, 3, 4, 5, 6, 8};
    static uint8_t                currentZoomIndex = 2; // 100%

    const auto preview = std::format("{}%", zoomList[currentZoomIndex] * zoomUnit);
    if (ImGui::BeginCombo(
            Translate("Settings.Appearance.Zoom").data(), preview.c_str(), ImGuiEx::ComboFlags().NoArrowButton()
        ))
    {
        uint8_t index = 0;
        for (const uint8_t &zoom : zoomList)
        {
            const auto percentage = zoom * zoomUnit;
            if (const bool selected = index == currentZoomIndex;
                ImGui::Selectable(std::format("{}%", zoom * zoomUnit).c_str(), selected) && !selected)
            {
                m_styles.UpdateScaling(percentage / 100.f);
                currentZoomIndex = index;
            }
            index++;
        }
        ImGui::EndCombo();
    }
}

void AppearancePanel::DrawThemeBuilder()
{
    using namespace ImGuiEx;
    const auto &medium    = m_styles.GetMediumText();
    const auto &colors    = m_styles.colors;
    bool        openPopup = false;
    ImGui::PushFont(nullptr, medium.fontSize);

    constexpr auto colorButtonFlags = ColorEditFlags().NoAlpha().NoPicker().NoTooltip();
    {
        StyleGuard styleGuard;
        styleGuard.Push(StyleHolder::FramePadding({0.f, m_styles[M3::Spacing::M]}))
            .Push(ColorHolder::ChildBg(colors[M3::SurfaceToken::surface]));

        openPopup = ImGui::ColorButton("##SeedColor", argbToImVec4(colors.SeedArgb()), colorButtonFlags);
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(Translate("Settings.Appearance.ThemeColor").data());
    }
    auto imU32ToArgb = [](ImU32 imU32) -> uint32_t {
        return (imU32 & 0xFF000000) | (imU32 & 0xFF) << 16 | (imU32 & 0xFF00) | (imU32 & 0xFF0000) >> 16;
    };

    bool edited = false;
    if (openPopup)
    {
        ImGui::OpenPopup("ThemeBuilder");
        edited = true;
    }
    {
        StyleGuard styleGuard;
        styleGuard.Push(StyleHolder::FramePadding({0.f, m_styles[M3::Spacing::S]}))
            .Push(StyleHolder::WindowPadding({m_styles[M3::Spacing::M], m_styles[M3::Spacing::M]}))
            .Push(ColorHolder::PopupBg(colors[M3::SurfaceToken::surfaceContainerHigh]));
        if (ImGui::BeginPopupModal(
                Translate("Settings.Appearance.ThemeBuilder").data(), nullptr, WindowFlags().AlwaysAutoResize()
            ))
        {
            using namespace material_color_utilities;
            static std::unique_ptr<SchemeTonalSpot> scheme = nullptr;

            ImGui::BeginGroup();
            {
                StyleGuard styleGuard1;
                styleGuard1.Push(ColorHolder::Text(colors[M3::ContentToken::onPrimaryContainer]))
                    .Push(ColorHolder::FrameBg(colors[M3::SurfaceToken::primaryContainer]))
                    .Push(
                        ColorHolder::FrameBgHovered(
                            colors[M3::SurfaceToken::primaryContainer].Hovered(
                                colors[M3::ContentToken::onPrimaryContainer]
                            )
                        )
                    )
                    .Push(
                        ColorHolder::FrameBgActive(
                            colors[M3::SurfaceToken::primaryContainer].Pressed(
                                colors[M3::ContentToken::onPrimaryContainer]
                            )
                        )
                    );
                if (ImGui::ColorPicker3(
                        "##picker",
                        reinterpret_cast<float *>(&m_colorInThemeBuilder.Value),
                        ColorEditFlags().NoSmallPreview().NoSidePreview()
                    ))
                {
                    edited = true;
                }
                styleGuard1.Push(ColorHolder::Text(colors[M3::ContentToken::onPrimary]))
                    .Push(StyleHolder::FramePadding({m_styles[M3::Spacing::M], m_styles[M3::Spacing::S]}))
                    .Push(StyleHolder::ItemSpacing({m_styles[M3::Spacing::L], 0.f}))
                    .Push(StyleHolder::FrameRounding(m_styles[M3::Spacing::M]));
                if (ImGui::Button(Translate("Settings.Appearance.Apply").data()))
                {
                    ApplyM3Theme(imU32ToArgb(m_colorInThemeBuilder), m_darkModeInThemeBuilder);
                    scheme.reset();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button(Translate("Settings.Appearance.Cancel").data()))
                {
                    scheme.reset();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndGroup();

            ImGui::SameLine(0, m_styles[M3::Spacing::S]);

            ImGui::BeginGroup();
            ImGui::Checkbox(Translate("Settings.Appearance.DarkMode").data(), &m_darkModeInThemeBuilder);
            if (edited)
            {
                scheme = std::make_unique<SchemeTonalSpot>(
                    Hct(imU32ToArgb(m_colorInThemeBuilder)), m_darkModeInThemeBuilder, 0.0
                );
            }

            if (scheme)
            {
                StyleGuard styleGuard1;
                styleGuard1.Push(ColorHolder::Text(colors[M3::ContentToken::onSurface]))
                    .Push(StyleHolder::FramePadding({m_styles[M3::Spacing::L], m_styles[M3::Spacing::L]}))
                    .Push(StyleHolder::ItemSpacing({0, m_styles[M3::Spacing::M]}));

                auto draw_palette = [&colorButtonFlags, this](std::string_view label, const TonalPalette &palette) {
                    ImGui::ColorButton(label.data(), argbToImVec4(palette.get_key_color().ToInt()), colorButtonFlags);
                    ImGui::SameLine(0, m_styles[M3::Spacing::S]);
                    ImGui::TextUnformatted(label.data());
                };
                draw_palette("Primary", scheme->primary_palette);
                draw_palette("Secondary", scheme->secondary_palette);
                draw_palette("Tertiary", scheme->tertiary_palette);
                draw_palette("Neutral", scheme->neutral_palette);
                draw_palette("NeutralVariant", scheme->neutral_variant_palette);
                draw_palette("Error", scheme->error_palette);
            }
            ImGui::EndGroup();

            ImGui::EndPopup();
        }
    }
    ImGui::PopFont();
}

void AppearancePanel::ApplyM3Theme(uint32_t seedArgb, const bool darkMode)
{
    using namespace ImGuiEx::M3;
    auto &colors = m_styles.colors;

    ThemeBuilder::BuildThemeFromSeed(seedArgb, darkMode, colors);
    m_colorInThemeBuilder = ImColor(argbToImVec4(m_styles.colors.SeedArgb()));

    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text] = colors[ContentToken::onSurface];

    style.Colors[ImGuiCol_TitleBg]          = colors[SurfaceToken::surfaceContainer];
    style.Colors[ImGuiCol_TitleBgActive]    = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TitleBgCollapsed] = colors[SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_WindowBg] = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_ChildBg]  = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_PopupBg]  = colors[SurfaceToken::primaryContainer];

    style.Colors[ImGuiCol_FrameBg] = colors[SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_FrameBgActive] =
        colors[SurfaceToken::secondaryContainer].Pressed(colors[ContentToken::onSecondaryContainer]);
    style.Colors[ImGuiCol_FrameBgHovered] =
        colors[SurfaceToken::secondaryContainer].Hovered(colors[ContentToken::onSecondaryContainer]);
    style.Colors[ImGuiCol_Border]       = colors[SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_BorderShadow] = colors[SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_SliderGrab]       = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_SliderGrabActive] = colors[SurfaceToken::primary].Pressed(colors[ContentToken::onPrimary]);

    style.Colors[ImGuiCol_Button]        = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_ButtonHovered] = colors[SurfaceToken::primary].Hovered(colors[ContentToken::onPrimary]);
    style.Colors[ImGuiCol_ButtonActive]  = colors[SurfaceToken::primary].Pressed(colors[ContentToken::onPrimary]);

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors[SurfaceToken::outline];
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors[SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors[SurfaceToken::primary];

    style.Colors[ImGuiCol_MenuBarBg] = colors[SurfaceToken::surfaceContainerHigh];

    style.Colors[ImGuiCol_Header] = colors[SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_HeaderHovered] =
        colors[SurfaceToken::surfaceContainerHigh].Hovered(colors[ContentToken::onSurface]);
    style.Colors[ImGuiCol_HeaderActive] =
        colors[SurfaceToken::surfaceContainerHigh].Pressed(colors[ContentToken::onSurface]);

    style.Colors[ImGuiCol_Separator] = colors[SurfaceToken::secondary];
    style.Colors[ImGuiCol_SeparatorHovered] =
        colors[SurfaceToken::secondary].Hovered(colors[ContentToken::onSecondary]);
    style.Colors[ImGuiCol_SeparatorActive] = colors[SurfaceToken::secondary].Pressed(colors[ContentToken::onSecondary]);

    style.Colors[ImGuiCol_ResizeGrip] = colors[SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_ResizeGripHovered] =
        colors[SurfaceToken::secondaryContainer].Hovered(colors[ContentToken::onSecondaryContainer]);
    style.Colors[ImGuiCol_ResizeGripActive] =
        colors[SurfaceToken::secondaryContainer].Pressed(colors[ContentToken::onSecondaryContainer]);

    style.Colors[ImGuiCol_InputTextCursor] = colors[SurfaceToken::secondary];

    style.Colors[ImGuiCol_Tab]                 = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TabHovered]          = colors[SurfaceToken::surface].Hovered(colors[ContentToken::onSurface]);
    style.Colors[ImGuiCol_TabSelected]         = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TabSelectedOverline] = colors[SurfaceToken::primary];

    style.Colors[ImGuiCol_TabDimmed]         = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TabDimmedSelected] = colors[SurfaceToken::surface].Pressed(colors[ContentToken::onSurface]);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors[SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_PlotLines]        = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_PlotLinesHovered] = colors[SurfaceToken::primary].Hovered(colors[ContentToken::onPrimary]);

    style.Colors[ImGuiCol_PlotHistogram] = colors[SurfaceToken::tertiary];
    style.Colors[ImGuiCol_PlotHistogramHovered] =
        colors[SurfaceToken::tertiary].Hovered(colors[ContentToken::onTertiary]);

    style.Colors[ImGuiCol_TableHeaderBg]     = colors[SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_TableBorderStrong] = colors[SurfaceToken::outline];
    style.Colors[ImGuiCol_TableBorderLight]  = colors[SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_TableRowBg]        = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors[SurfaceToken::surfaceContainerLowest];

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low;
    style.Colors[ImGuiCol_TextSelectedBg]   = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors[ContentToken::onSurface];

    style.Colors[ImGuiCol_DragDropTarget]   = colors[SurfaceToken::primary];
    style.Colors[ImGuiCol_DragDropTargetBg] = colors[SurfaceToken::surface];

    style.Colors[ImGuiCol_UnsavedMarker]         = colors[ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavCursor]             = colors[ContentToken::onSecondary];
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors[ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors[SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors[SurfaceToken::surface];
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}
}
}