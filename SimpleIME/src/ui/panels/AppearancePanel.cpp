//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "cpp/scheme/scheme_tonal_spot.h"
#include "i18n/TranslationLoader.h"
#include "i18n/TranslatorHolder.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/imguiex_m3_slider.h"
#include "imguiex/m3/facade/button.h"
#include "imguiex/m3/spec/others.h"
#include "path_utils.h"
#include "ui/Settings.h"
#include "ui/imgui_system.h"

#include <imgui.h>

namespace Ime
{
void AppearancePanel::Draw(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard
        .Style<ImGuiStyleVar_WindowPadding>({m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::L]})
        .Style<ImGuiStyleVar_ItemSpacing>({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::Double_M]})
        .Color<ImGuiCol_Text>(m3Styles.Colors().at(ImGuiEx::M3::ContentToken::onSurface))
        .Color<ImGuiCol_ChildBg>(m3Styles.Colors().at(ImGuiEx::M3::SurfaceToken::surface));
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
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    const auto _ = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();

    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color<ImGuiCol_Text>(m3Styles.Colors().at(ContentToken::onSurfaceVariant))
        .Style<ImGuiStyleVar_FrameBorderSize>(m3Styles[ImGuiEx::M3::Spacing::XS])
        .Style<ImGuiStyleVar_WindowPadding>(
            {m3Styles.GetPixels(M3Spec::Menu::paddingX), m3Styles.GetPixels(M3Spec::Menu::paddingY)}
        )
        .Color<ImGuiCol_Border>(m3Styles.Colors().at(SurfaceToken::primary))
        .Color<ImGuiCol_FrameBg>(m3Styles.Colors().at(SurfaceToken::surface))
        .Color<ImGuiCol_FrameBgHovered>(
            m3Styles.Colors().Hovered(SurfaceToken::surface, ContentToken::onSurfaceVariant)
        )
        .Color<ImGuiCol_PopupBg>(m3Styles.Colors().at(SurfaceToken::surfaceContainerLow))
        .Color<ImGuiCol_HeaderActive>(m3Styles.Colors().at(SurfaceToken::tertiaryContainer))
        .Color<ImGuiCol_HeaderHovered>(
            m3Styles.Colors().Hovered(SurfaceToken::surfaceContainerLow, ContentToken::onSurface)
        );
    const auto availX = ImGui::GetContentRegionAvail().x;
    if (const auto maxWidth = m3Styles.GetPixels(M3Spec::Menu::width); availX > maxWidth)
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
        ImGuiEx::M3::BeginMenu(m3Styles);
        const auto itemHeight = m3Styles.GetPixels(M3Spec::Menu::itemHeight);
        for (const int &zoom : zoomList)
        {
            const auto percentage = zoom * zoomUnit;
            if (const bool selected = index == currentZoomIndex;
                ImGui::Selectable(std::format("{}%", zoom * zoomUnit).c_str(), selected, 0, {0, itemHeight}) &&
                !selected)
            {
                m3Styles.UpdateScaling(static_cast<float>(percentage) / 100.f);
                currentZoomIndex = index;
            }
            index++;
        }
        ImGuiEx::M3::EndMenu(m3Styles);
        ImGui::EndCombo();
    }
}

void AppearancePanel::DrawThemeBuilder(ImGuiEx::M3::M3Styles &m3Styles)
{
    using ContentToken = ImGuiEx::M3::ContentToken;
    using SurfaceToken = ImGuiEx::M3::SurfaceToken;

    const auto &colors       = m3Styles.Colors();
    const auto &schemeConfig = m3Styles.Colors().GetSchemeConfig();
    bool        openPopup    = false;
    const auto  _            = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();

    constexpr auto colorButtonFlags = ImGuiEx::ColorEditFlags().NoAlpha().NoPicker().NoTooltip();
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style<ImGuiStyleVar_FramePadding>({0.f, m3Styles[ImGuiEx::M3::Spacing::M]})
            .Color<ImGuiCol_ChildBg>(colors[SurfaceToken::surface]);

        openPopup =
            ImGui::ColorButton("##SourceColor", ImGuiEx::M3::ArgbToImVec4(schemeConfig.sourceColor), colorButtonFlags);
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(Translate("Settings.Appearance.ThemeColor"));
    }
    bool edited = false;
    if (openPopup)
    {
        ImGui::OpenPopup("ThemeBuilder");
        m_ImColorTemp  = ImGuiEx::M3::ArgbToImVec4(schemeConfig.sourceColor);
        m_darkModeTemp = schemeConfig.darkMode;
        edited         = true;
    }
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style<ImGuiStyleVar_FramePadding>({0.f, m3Styles[ImGuiEx::M3::Spacing::S]})
            .Style<ImGuiStyleVar_WindowPadding>({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::M]})
            .Color<ImGuiCol_PopupBg>(colors[SurfaceToken::surfaceContainerHigh]);
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
                styleGuard1.Color<ImGuiCol_Text>(colors[ContentToken::onPrimaryContainer])
                    .Color<ImGuiCol_FrameBg>(colors[SurfaceToken::primaryContainer])
                    .Color<ImGuiCol_FrameBgHovered>(
                        colors.Hovered(SurfaceToken::primaryContainer, ContentToken::onPrimaryContainer)
                    )

                    .Color<ImGuiCol_FrameBgActive>(
                        colors.Pressed(SurfaceToken::primaryContainer, ContentToken::onPrimaryContainer)
                    );
                if (ImGui::ColorPicker3(
                        "##picker",
                        reinterpret_cast<float *>(&m_ImColorTemp.Value),
                        ImGuiEx::ColorEditFlags().NoSmallPreview().NoSidePreview()
                    ))
                {
                    edited = true;
                }
                styleGuard1.Color<ImGuiCol_Text>(colors[ContentToken::onPrimary])
                    .Style<ImGuiStyleVar_FramePadding>(
                        {m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::M]}
                    )
                    .Style<ImGuiStyleVar_ItemSpacing>({m3Styles[ImGuiEx::M3::Spacing::L], 0.f})
                    .Style<ImGuiStyleVar_FrameRounding>(m3Styles.GetPixels(M3Spec::SmallButton::rounding));
                if (ImGui::Button(Translate("Settings.Appearance.Apply")))
                {
                    m3Styles.RebuildColors(
                        {m_contrastLevelTemp, ImGuiEx::M3::ImU32ToArgb(m_ImColorTemp), m_darkModeTemp}
                    );
                    UI::ApplyM3Theme(m3Styles);
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
            ImGui::Checkbox(Translate("Settings.Appearance.DarkMode"), &m_darkModeTemp);
            if (edited)
            {
                scheme = std::make_unique<Scheme>(
                    Hct(ImGuiEx::M3::ImU32ToArgb(m_ImColorTemp)), m_darkModeTemp, m_contrastLevelTemp
                );
            }
            ImGuiEx::M3::Slider::Draw(
                Translate("Settings.Appearance.ContrastLevel"),
                ImGuiEx::M3::Slider::Params{
                    m_contrastLevelTemp, ImGuiEx::M3::CONTRAST_MIN, ImGuiEx::M3::CONTRAST_MAX, m3Styles
                }
            );

            if (scheme)
            {
                ImGuiEx::StyleGuard styleGuard1;
                styleGuard1.Color<ImGuiCol_Text>(colors[ContentToken::onSurface])
                    .Style<ImGuiStyleVar_FramePadding>(
                        {m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::L]}
                    )
                    .Style<ImGuiStyleVar_ItemSpacing>({0, m3Styles[ImGuiEx::M3::Spacing::M]});

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
    UI::ApplyM3Theme(m3Styles);

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
