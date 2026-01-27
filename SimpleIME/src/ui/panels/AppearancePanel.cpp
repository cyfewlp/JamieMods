//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/M3ThemeBuilder.h"
#include "common/imgui/Material3.h"
#include "ui/Settings.h"

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
    ImGui::DragFloat(
        m_translation["$Font_Size_Scale"],
        &ImGui::GetStyle().FontScaleMain,
        0.05,
        Settings::MIN_FONT_SIZE_SCALE,
        Settings::MAX_FONT_SIZE_SCALE,
        "%.3f",
        ImGuiSliderFlags_NoInput
    );

    if (appearing)
    {
        m_colorInThemeBuilder = argbToImVec4(m_styles.colors.SeedArgb());
    }
    if (ImGui::BeginTable("CenterAlignTable", 3, ImGuiEx::TableFlags().SizingStretchSame()))
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        DrawThemeBuilder();
        ImGui::TableNextColumn();
        ImGui::EndTable();
    }
}

void AppearancePanel::DrawThemeBuilder()
{
    auto imU32ToArgb = [](ImU32 imU32) -> uint32_t {
        return (imU32 & 0xFF000000) | (imU32 & 0xFF) << 16 | (imU32 & 0xFF00) | (imU32 & 0xFF0000) >> 16;
    };
    constexpr auto flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
    bool           openPopup = ImGui::ColorButton("##SeedColor", argbToImVec4(m_styles.colors.SeedArgb()), flags);
    ImGui::SameLine();
    ImGui::TextUnformatted("Source color");

    if (openPopup)
    {
        ImGui::OpenPopup("seedColorPicker");
    }
    if (ImGui::BeginPopup("seedColorPicker"))
    {
        ImGui::ColorPicker4(
            "##picker",
            reinterpret_cast<float *>(&m_colorInThemeBuilder.Value),
            ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview
        );
        ImGui::SameLine();

        ImGui::BeginGroup();
        {
            static bool darkMode = false;
            ImGui::Checkbox(m_translation["$Theme_DarkMode"], &darkMode);

            if (ImGui::Button(m_translation["$Apply"]))
            {
                ImGuiEx::M3::ThemeBuilder::BuildThemeFromSeed(
                    imU32ToArgb(m_colorInThemeBuilder), darkMode, m_styles.colors
                );
                m_colorInThemeBuilder = ImColor(argbToImVec4(m_styles.colors.SeedArgb()));
            }

            if (ImGui::Button(m_translation["$Cancel"]))
            {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
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
}

}
}