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
#include "ui/Settings.h"

#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
void AppearancePanel::Draw(Settings &settings)
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Style_WindowPadding({m_styles[ImGuiEx::M3::Spacing::L], m_styles[ImGuiEx::M3::Spacing::L]})
        .Style_ItemSpacing({m_styles[ImGuiEx::M3::Spacing::M], m_styles[ImGuiEx::M3::Spacing::Double_M]})
        .Color_Text(m_styles.Colors().at(ImGuiEx::M3::ContentToken::onSurface))
        .Color_ChildBg(m_styles.Colors().at(ImGuiEx::M3::SurfaceToken::surface));
    if (ImGui::BeginChild("##Appearance", {}, ImGuiEx::ChildFlags().AlwaysUseWindowPadding()))
    {
        if (ImGui::BeginTable("CenterAlignTable", 3, ImGuiEx::TableFlags().SizingStretchSame()))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            DrawZoomCombo();
            ImGui::Spacing();
            DrawThemeBuilder();
            DrawLanguagesCombo(settings.appearance);
            ImGui::TableNextColumn();
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void AppearancePanel::DrawZoomCombo() const
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color_Text(m_styles.Colors().at(ImGuiEx::M3::ContentToken::onSurfaceVariant))
        .Style_FrameBorderSize(m_styles[ImGuiEx::M3::Spacing::XS])
        .Color_Border(m_styles.Colors().at(ImGuiEx::M3::SurfaceToken::primary))
        .Color_FrameBg(m_styles.Colors().at(ImGuiEx::M3::SurfaceToken::surface))
        .Color_FrameBgHovered(m_styles.Colors()
                                  .at(ImGuiEx::M3::SurfaceToken::surface)
                                  .Hovered(m_styles.Colors().at(ImGuiEx::M3::ContentToken::onSurfaceVariant)))
        .Color_PopupBg(m_styles.Colors().at(ImGuiEx::M3::SurfaceToken::surfaceContainerLow))
        .Color_HeaderActive(m_styles.Colors().at(ImGuiEx::M3::SurfaceToken::tertiaryContainer))
        .Color_HeaderHovered(m_styles.Colors()
                                 .at(ImGuiEx::M3::SurfaceToken::surfaceContainerLow)
                                 .Hovered(m_styles.Colors().at(ImGuiEx::M3::ContentToken::onSurface)));
    const auto availX = ImGui::GetContentRegionAvail().x;
    if (const auto maxWidth = m_styles.GetSize(ImGuiEx::M3::ComponentSize::MENU_WIDTH); availX > maxWidth)
    {
        ImGui::SetNextItemWidth(maxWidth);
    }
    constexpr uint8_t zoomUnit         = 25;
    static std::array zoomList         = {2, 3, 4, 5, 6, 8};
    static uint8_t    currentZoomIndex = 2; // 100%

    const auto preview = std::format("{}%", zoomList.at(currentZoomIndex) * zoomUnit);
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
                m_styles.UpdateScaling(static_cast<float>(percentage) / 100.f);
                currentZoomIndex = index;
            }
            index++;
        }
        ImGui::EndCombo();
    }
}

void AppearancePanel::DrawThemeBuilder()
{
    const auto &colors    = m_styles.Colors();
    bool        openPopup = false;
    ImGui::PushFont(nullptr, m_styles.LabelText().fontSize);

    constexpr auto colorButtonFlags = ImGuiEx::ColorEditFlags().NoAlpha().NoPicker().NoTooltip();
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_FramePadding({0.f, m_styles[ImGuiEx::M3::Spacing::M]})
            .Color_ChildBg(colors[ImGuiEx::M3::SurfaceToken::surface]);

        openPopup =
            ImGui::ColorButton("##SourceColor", ImGuiEx::M3::ArgbToImVec4(colors.SourceColor()), colorButtonFlags);
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(Translate("Settings.Appearance.ThemeColor").data());
    }
    bool edited = false;
    if (openPopup)
    {
        ImGui::OpenPopup("ThemeBuilder");
        edited = true;
    }
    {
        ImGuiEx::StyleGuard styleGuard;
        styleGuard.Style_FramePadding({0.f, m_styles[ImGuiEx::M3::Spacing::S]})
            .Style_WindowPadding({m_styles[ImGuiEx::M3::Spacing::M], m_styles[ImGuiEx::M3::Spacing::M]})
            .Color_PopupBg(colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh]);
        if (ImGui::BeginPopupModal(
                Translate("Settings.Appearance.ThemeBuilder").data(), nullptr, ImGuiEx::WindowFlags().AlwaysAutoResize()
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
                    .Style_FramePadding({m_styles[ImGuiEx::M3::Spacing::L], m_styles[ImGuiEx::M3::Spacing::M]})
                    .Style_ItemSpacing({m_styles[ImGuiEx::M3::Spacing::L], 0.f})
                    .Style_FrameRounding(m_styles.GetSize(ImGuiEx::M3::ComponentSize::BUTTON_ROUNDING));
                if (ImGui::Button(Translate("Settings.Appearance.Apply").data()))
                {
                    m_styles.RebuildColors(ImGuiEx::M3::ImU32ToArgb(m_colorInThemeBuilder), m_darkModeInThemeBuilder);
                    ApplyM3Theme();
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

            ImGui::SameLine(0, m_styles[ImGuiEx::M3::Spacing::S]);

            ImGui::BeginGroup();
            ImGui::Checkbox(Translate("Settings.Appearance.DarkMode").data(), &m_darkModeInThemeBuilder);
            if (edited)
            {
                m_colorInThemeBuilder    = ImGuiEx::M3::ArgbToImVec4(m_styles.Colors().SourceColor());
                m_darkModeInThemeBuilder = m_styles.Colors().DarkMode();
                scheme                   = std::make_unique<Scheme>(
                    Hct(ImGuiEx::M3::ImU32ToArgb(m_colorInThemeBuilder)), m_darkModeInThemeBuilder, 0.0
                );
            }

            if (scheme)
            {
                ImGuiEx::StyleGuard styleGuard1;
                styleGuard1.Color_Text(colors[ImGuiEx::M3::ContentToken::onSurface])
                    .Style_FramePadding({m_styles[ImGuiEx::M3::Spacing::L], m_styles[ImGuiEx::M3::Spacing::L]})
                    .Style_ItemSpacing({0, m_styles[ImGuiEx::M3::Spacing::M]});

                auto draw_palette = [&colorButtonFlags, this](std::string_view label, const auto &palette) {
                    ImGui::ColorButton(
                        label.data(), ImGuiEx::M3::ArgbToImVec4(palette.get_key_color().ToInt()), colorButtonFlags
                    );
                    ImGui::SameLine(0, m_styles[ImGuiEx::M3::Spacing::S]);
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

void AppearancePanel::DrawLanguagesCombo(Settings::Appearance &appearance)
{
    bool clicked = false;
    if (ImGui::BeginCombo(Translate("Settings.Appearance.Languages").data(), appearance.language.c_str()))
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

void AppearancePanel::ApplySettings(Settings::Appearance &appearance)
{
    appearance.zoom = std::min(ZOOM_MAX, appearance.zoom);
    appearance.zoom = std::max(ZOOM_MIN, appearance.zoom);
    m_styles.UpdateScaling(appearance.zoom);

    const auto dir = CommonUtils::GetInterfacePath() / SIMPLE_IME;
    TranslationLoader::ScanLanguages(dir, m_translateLanguages);

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

void AppearancePanel::ApplyM3Theme()
{
    auto &colors = m_styles.Colors();

    m_colorInThemeBuilder = ImColor(ImGuiEx::M3::ArgbToImVec4(m_styles.Colors().SourceColor()));

    ImGuiStyle &style = ImGui::GetStyle();

    style.Colors[ImGuiCol_Text]             = colors[ImGuiEx::M3::ContentToken::onSurface];
    style.Colors[ImGuiCol_TitleBg]          = colors[ImGuiEx::M3::SurfaceToken::surfaceContainer];
    style.Colors[ImGuiCol_TitleBgActive]    = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TitleBgCollapsed] = colors[ImGuiEx::M3::SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_WindowBg] = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_ChildBg]  = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_PopupBg]  = colors[ImGuiEx::M3::SurfaceToken::primaryContainer];

    style.Colors[ImGuiCol_FrameBg]       = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_FrameBgActive] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Pressed(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );
    style.Colors[ImGuiCol_FrameBgHovered] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Hovered(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );
    style.Colors[ImGuiCol_Border]       = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_BorderShadow] = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_SliderGrab] = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_SliderGrabActive] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Pressed(colors[ImGuiEx::M3::ContentToken::onPrimary]);

    style.Colors[ImGuiCol_Button] = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_ButtonHovered] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Hovered(colors[ImGuiEx::M3::ContentToken::onPrimary]);
    style.Colors[ImGuiCol_ButtonActive] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Pressed(colors[ImGuiEx::M3::ContentToken::onPrimary]);

    style.Colors[ImGuiCol_ScrollbarBg]          = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ScrollbarGrab]        = colors[ImGuiEx::M3::SurfaceToken::outline];
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = colors[ImGuiEx::M3::SurfaceToken::primary];

    style.Colors[ImGuiCol_MenuBarBg] = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh];

    style.Colors[ImGuiCol_Header] = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_HeaderHovered] =
        colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh].Hovered(colors[ImGuiEx::M3::ContentToken::onSurface]);
    style.Colors[ImGuiCol_HeaderActive] =
        colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh].Pressed(colors[ImGuiEx::M3::ContentToken::onSurface]);

    style.Colors[ImGuiCol_Separator] = colors[ImGuiEx::M3::SurfaceToken::secondary];
    style.Colors[ImGuiCol_SeparatorHovered] =
        colors[ImGuiEx::M3::SurfaceToken::secondary].Hovered(colors[ImGuiEx::M3::ContentToken::onSecondary]);
    style.Colors[ImGuiCol_SeparatorActive] =
        colors[ImGuiEx::M3::SurfaceToken::secondary].Pressed(colors[ImGuiEx::M3::ContentToken::onSecondary]);

    style.Colors[ImGuiCol_ResizeGrip]        = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer];
    style.Colors[ImGuiCol_ResizeGripHovered] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Hovered(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );
    style.Colors[ImGuiCol_ResizeGripActive] = colors[ImGuiEx::M3::SurfaceToken::secondaryContainer].Pressed(
        colors[ImGuiEx::M3::ContentToken::onSecondaryContainer]
    );

    style.Colors[ImGuiCol_InputTextCursor] = colors[ImGuiEx::M3::SurfaceToken::secondary];

    style.Colors[ImGuiCol_Tab] = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TabHovered] =
        colors[ImGuiEx::M3::SurfaceToken::surface].Hovered(colors[ImGuiEx::M3::ContentToken::onSurface]);
    style.Colors[ImGuiCol_TabSelected]         = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiEx::M3::SurfaceToken::primary];

    style.Colors[ImGuiCol_TabDimmed] = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TabDimmedSelected] =
        colors[ImGuiEx::M3::SurfaceToken::surface].Pressed(colors[ImGuiEx::M3::ContentToken::onSurface]);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];

    style.Colors[ImGuiCol_PlotLines] = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_PlotLinesHovered] =
        colors[ImGuiEx::M3::SurfaceToken::primary].Hovered(colors[ImGuiEx::M3::ContentToken::onPrimary]);

    style.Colors[ImGuiCol_PlotHistogram] = colors[ImGuiEx::M3::SurfaceToken::tertiary];
    style.Colors[ImGuiCol_PlotHistogramHovered] =
        colors[ImGuiEx::M3::SurfaceToken::tertiary].Hovered(colors[ImGuiEx::M3::ContentToken::onTertiary]);

    style.Colors[ImGuiCol_TableHeaderBg]     = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerHigh];
    style.Colors[ImGuiCol_TableBorderStrong] = colors[ImGuiEx::M3::SurfaceToken::outline];
    style.Colors[ImGuiCol_TableBorderLight]  = colors[ImGuiEx::M3::SurfaceToken::outlineVariant];
    style.Colors[ImGuiCol_TableRowBg]        = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_TableRowBgAlt]     = colors[ImGuiEx::M3::SurfaceToken::surfaceContainerLowest];

    // style.Colors[ImGuiCol_TextLink]     =colors.surface_container_low;
    style.Colors[ImGuiCol_TextSelectedBg]   = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.35f;

    style.Colors[ImGuiCol_TreeLines] = colors[ImGuiEx::M3::ContentToken::onSurface];

    style.Colors[ImGuiCol_DragDropTarget]   = colors[ImGuiEx::M3::SurfaceToken::primary];
    style.Colors[ImGuiCol_DragDropTargetBg] = colors[ImGuiEx::M3::SurfaceToken::surface];

    style.Colors[ImGuiCol_UnsavedMarker]         = colors[ImGuiEx::M3::ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavCursor]             = colors[ImGuiEx::M3::ContentToken::onSecondary];
    style.Colors[ImGuiCol_NavWindowingHighlight] = colors[ImGuiEx::M3::ContentToken::onPrimary];
    style.Colors[ImGuiCol_NavWindowingDimBg]     = colors[ImGuiEx::M3::SurfaceToken::surfaceContainer];

    style.Colors[ImGuiCol_ModalWindowDimBg]   = colors[ImGuiEx::M3::SurfaceToken::surface];
    style.Colors[ImGuiCol_ModalWindowDimBg].w = 0.35f;
}

// FIXME: Handle error
void AppearancePanel::LoadTranslation(std::string_view language) const
{
    if (!m_i18nHandle) return;
    const TranslationLoader loader(CommonUtils::GetInterfacePath() / SIMPLE_IME, "Settings");

    if (auto opt = loader.LoadFrom(language); opt)
    {
        m_i18nHandle->Update(std::move(opt.value()));
    }
}
}
}