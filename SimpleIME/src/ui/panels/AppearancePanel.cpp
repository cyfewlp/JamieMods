//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "common/imgui/ImGuiEx.h"
#include "common/imgui/M3ThemeBuilder.h"
#include "common/imgui/Material3.h"
#include "common/utils.h"
#include "cpp/scheme/scheme_tonal_spot.h"
#include "i18n/TranslationLoader.h"
#include "i18n/TranslatorHolder.h"
#include "ui/ImGuiManager.h"
#include "ui/Settings.h"

#include <imgui.h>

namespace Ime
{
void AppearancePanel::Draw(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Style_WindowPadding({m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::L]})
        .Style_ItemSpacing({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::Double_M]})
        .Color_Text(m3Styles.Colors().at(ImGuiEx::M3::ContentToken::onSurface))
        .Color_ChildBg(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::surface));
    if (ImGui::BeginChild("##Appearance", {}, ImGuiEx::ChildFlags().AlwaysUseWindowPadding()))
    {
        if (ImGui::BeginTable("CenterAlignTable", 3, ImGuiEx::TableFlags().SizingStretchSame()))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            DrawZoomCombo(m3Styles);
            ImGui::Spacing();
            DrawThemeBuilder(m3Styles);
            DrawLanguagesCombo(settings.appearance);
            ImGui::TableNextColumn();
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void AppearancePanel::DrawZoomCombo(ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color_Text(m3Styles.Colors().at(ImGuiEx::M3::ContentToken::onSurfaceVariant))
        .Style_FrameBorderSize(m3Styles[ImGuiEx::M3::Spacing::XS])
        .Color_Border(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::primary))
        .Color_FrameBg(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::surface))
        .Color_FrameBgHovered(m3Styles.Colors()
                                  .at(ImGuiEx::M3::SurfaceToken::surface)
                                  .Hovered(m3Styles.Colors().at(ImGuiEx::M3::ContentToken::onSurfaceVariant)))
        .Color_PopupBg(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::surfaceContainerLow))
        .Color_HeaderActive(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::tertiaryContainer))
        .Color_HeaderHovered(m3Styles.Colors()
                                 .at(ImGuiEx::M3::SurfaceToken::surfaceContainerLow)
                                 .Hovered(m3Styles.Colors().at(ImGuiEx::M3::ContentToken::onSurface)));
    const auto availX = ImGui::GetContentRegionAvail().x;
    if (const auto maxWidth = m3Styles.GetSize(ImGuiEx::M3::ComponentSize::MENU_WIDTH); availX > maxWidth)
    {
        ImGui::SetNextItemWidth(maxWidth);
    }
    constexpr uint8_t zoomUnit         = 25;
    static std::array zoomList         = {2, 3, 4, 5, 6, 8};
    static uint8_t    currentZoomIndex = 2; // 100%

    const auto preview = std::format("{}%", zoomList.at(currentZoomIndex) * zoomUnit);
    if (ImGui::BeginCombo(
            Translate("Settings.Appearance.Zoom"), preview.c_str(), ImGuiEx::ComboFlags().NoArrowButton()
        ))
    {
        uint8_t index = 0;
        for (const int &zoom : zoomList)
        {
            const auto percentage = zoom * zoomUnit;
            if (const bool selected = index == currentZoomIndex;
                ImGui::Selectable(std::format("{}%", zoom * zoomUnit).c_str(), selected) && !selected)
            {
                m3Styles.UpdateScaling(static_cast<float>(percentage) / 100.f);
                currentZoomIndex = index;
            }
            index++;
        }
        ImGui::EndCombo();
    }
}

void AppearancePanel::DrawThemeBuilder(ImGuiEx::M3::M3Styles &m3Styles)
{
    const auto &colors    = m3Styles.Colors();
    bool        openPopup = false;
    ImGui::PushFont(nullptr, m3Styles.LabelText().fontSize);

    constexpr auto colorButtonFlags = ImGuiEx::ColorEditFlags().NoAlpha().NoPicker().NoTooltip();
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_FramePadding({0.f, m3Styles[ImGuiEx::M3::Spacing::M]})
            .Color_ChildBg(colors[ImGuiEx::M3::SurfaceToken::surface]);

        openPopup =
            ImGui::ColorButton("##SourceColor", ImGuiEx::M3::ArgbToImVec4(colors.SourceColor()), colorButtonFlags);
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(Translate("Settings.Appearance.ThemeColor"));
    }
    bool edited = false;
    if (openPopup)
    {
        ImGui::OpenPopup("ThemeBuilder");
        edited = true;
    }
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_FramePadding({0.f, m3Styles[ImGuiEx::M3::Spacing::S]})
            .Style_WindowPadding({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::M]})
            .Color_PopupBg(colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh]);
        if (ImGui::BeginPopupModal(
                Translate("Settings.Appearance.ThemeBuilder"), nullptr, ImGuiEx::WindowFlags().AlwaysAutoResize()
            ))
        {
            using Scheme = material_color_utilities::SchemeTonalSpot;
            using Hct    = material_color_utilities::Hct;

            static std::unique_ptr<Scheme> scheme = nullptr;

            ImGui::BeginGroup();
            {
                ImGuiEx::StyleGuard styleGuard1;
                styleGuard1.Color_Text(colors[ImGuiEx::M3::ContentToken::onPrimaryContainer])
                    .Color_FrameBg(colors[ImGuiEx::M3::SurfaceToken::primaryContainer])
                    .Color_FrameBgHovered(
                        colors[ImGuiEx::M3::SurfaceToken::primaryContainer].Hovered(
                            colors[ImGuiEx::M3::ContentToken::onPrimaryContainer]
                        )
                    )

                    .Color_FrameBgActive(
                        colors[ImGuiEx::M3::SurfaceToken::primaryContainer].Pressed(
                            colors[ImGuiEx::M3::ContentToken::onPrimaryContainer]
                        )
                    );
                if (ImGui::ColorPicker3(
                        "##picker",
                        reinterpret_cast<float *>(&m_colorInThemeBuilder.Value),
                        ImGuiEx::ColorEditFlags().NoSmallPreview().NoSidePreview()
                    ))
                {
                    edited = true;
                }
                styleGuard1.Color_Text(colors[ImGuiEx::M3::ContentToken::onPrimary])
                    .Style_FramePadding({m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::M]})
                    .Style_ItemSpacing({m3Styles[ImGuiEx::M3::Spacing::L], 0.f})
                    .Style_FrameRounding(m3Styles.GetSize(ImGuiEx::M3::ComponentSize::BUTTON_ROUNDING));
                if (ImGui::Button(Translate("Settings.Appearance.Apply")))
                {
                    m3Styles.RebuildColors(ImGuiEx::M3::ImU32ToArgb(m_colorInThemeBuilder), m_darkModeInThemeBuilder);
                    ImGuiManager::ApplyM3Theme(m3Styles);
                    scheme.reset();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button(Translate("Settings.Appearance.Cancel")))
                {
                    scheme.reset();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndGroup();

            ImGui::SameLine(0, m3Styles[ImGuiEx::M3::Spacing::S]);

            ImGui::BeginGroup();
            ImGui::Checkbox(Translate("Settings.Appearance.DarkMode"), &m_darkModeInThemeBuilder);
            if (edited)
            {
                m_colorInThemeBuilder    = ImGuiEx::M3::ArgbToImVec4(m3Styles.Colors().SourceColor());
                m_darkModeInThemeBuilder = m3Styles.Colors().DarkMode();
                scheme                   = std::make_unique<Scheme>(
                    Hct(ImGuiEx::M3::ImU32ToArgb(m_colorInThemeBuilder)), m_darkModeInThemeBuilder, 0.0
                );
            }

            if (scheme)
            {
                ImGuiEx::StyleGuard styleGuard1;
                styleGuard1.Color_Text(colors[ImGuiEx::M3::ContentToken::onSurface])
                    .Style_FramePadding({m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::L]})
                    .Style_ItemSpacing({0, m3Styles[ImGuiEx::M3::Spacing::M]});

                auto draw_palette = [&colorButtonFlags, &m3Styles](std::string_view label, const auto &palette) {
                    ImGui::ColorButton(
                        label.data(), ImGuiEx::M3::ArgbToImVec4(palette.get_key_color().ToInt()), colorButtonFlags
                    );
                    ImGui::SameLine(0, m3Styles[ImGuiEx::M3::Spacing::S]);
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

void AppearancePanel::DrawLanguagesCombo(Settings::Appearance &appearance) const
{
    bool clicked = false;
    if (ImGui::BeginCombo(Translate("Settings.Appearance.Languages"), appearance.language.c_str()))
    {
        int32_t idx = 0;
        for (const auto &language : m_translateLanguages)
        {
            ImGui::PushID(idx);
            const bool isSelected = appearance.language == language;
            if (ImGui::Selectable(language.c_str(), isSelected) && !isSelected)
            {
                appearance.language = language;
                clicked             = true;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
            idx++;
        }
        ImGui::EndCombo();
    }

    if (clicked)
    {
        LoadTranslation(appearance.language);
    }
}

void AppearancePanel::ApplySettings(Settings::Appearance &appearance, ImGuiEx::M3::M3Styles &m3Styles)
{
    appearance.zoom = std::min(ZOOM_MAX, appearance.zoom);
    appearance.zoom = std::max(ZOOM_MIN, appearance.zoom);
    m3Styles.UpdateScaling(appearance.zoom);
    ImGuiManager::ApplyM3Theme(m3Styles);

    i18n::ScanLanguages(utils::GetInterfacePath() / SIMPLE_IME, m_translateLanguages);

    if (const auto langIt = std::ranges::find(m_translateLanguages, appearance.language);
        langIt == m_translateLanguages.end())
    {
        appearance.language = "english";
    }

    if (auto accessorOpt = TranslatorHolder::RequestUpdateHandle())
    {
        m_i18nHandle.emplace(accessorOpt.value());
    }
    else
    {
        throw SimpleIMEException("Already initialized TranslatorHolder! TranslatorHolder should init by ImeUI!");
    }
    LoadTranslation(appearance.language);
}

// FIXME: Handle error
void AppearancePanel::LoadTranslation(std::string_view language) const
{
    if (!m_i18nHandle) return;
    if (auto opt = i18n::LoadTranslation(language, utils::GetInterfacePath() / SIMPLE_IME); opt)
    {
        m_i18nHandle->Update(std::move(opt.value()));
    }
}
} // namespace Ime
