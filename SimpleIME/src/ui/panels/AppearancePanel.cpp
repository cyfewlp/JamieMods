//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "cpp/scheme/scheme_tonal_spot.h"
#include "i18n/translator_manager.h"
#include "icons.h"
#include "imguiex/ImGuiEx.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "imguiex/imguiex_m3_slider.h"
#include "imguiex/m3/facade/button_groups.h"
#include "imguiex/m3/facade/slider.h"
#include "imguiex/m3/spec/layout.h"
#include "imguiex/m3/spec/text_field.h"
#include "path_utils.h"
#include "ui/Settings.h"
#include "ui/imgui_system.h"

#include <cassert>
#include <imgui.h>

namespace Ime
{

using ColorRole = M3Spec::ColorRole;

namespace
{
using Hct = material_color_utilities::Hct;

constexpr float kToneDefault = 50.F;
constexpr ImU32 COL_WHITE    = 0xFFFFFFFF;

void HandlerPickerCursor(ImDrawList *drawList, const char *strId, float &value, float maxValue, const ImVec2 &pickerPos, const ImVec2 &size)
{
    const float cursor_x = pickerPos.x + ((value / maxValue) * size.x);
    const float radius   = size.y * 0.5F;
    drawList->AddCircleFilled(ImVec2(cursor_x, pickerPos.y + radius), radius, COL_WHITE);

    const auto ratio = (ImGui::GetIO().MousePos.x - pickerPos.x) / size.x;

    (void)ImGui::InvisibleButton(strId, size);
    if (ImGui::IsItemActive())
    {
        value = maxValue * std::clamp(ratio, 0.0F, 1.0F);
    }
}

void DrawColorBar(ImDrawList *drawList, const ImVec2 &pos, const ImVec2 &size, const Hct &col1, const Hct &col2)
{
    drawList->AddRectFilledMultiColor(
        pos,
        ImVec2(pos.x + size.x, pos.y + size.y),
        ImGuiEx::M3::ArgbToImU32(col1.ToInt()),
        ImGuiEx::M3::ArgbToImU32(col2.ToInt()),
        ImGuiEx::M3::ArgbToImU32(col2.ToInt()),
        ImGuiEx::M3::ArgbToImU32(col1.ToInt())
    );
}

void DrawHuePicker(float &hue, float chroma, const ImVec2 &pickerSize)
{
    constexpr float kHueMax = 360.0F;

    const std::array col_hues = {
        Hct(0xFFFF0000), Hct(0xFFFFFF00), Hct(0xFF00FF00), Hct(0xFF00FFFF), Hct(0xFF0000FF), Hct(0xFFFF00FF), Hct(0xFFFF0000)
    };
    const auto kHueSegments = col_hues.size() - 1;

    auto *draw_list = ImGui::GetWindowDrawList();

    const ImVec2 picker_pos   = ImGui::GetCursorScreenPos();
    const float  segment_w    = pickerSize.x / static_cast<float>(kHueSegments);
    float        segment_minX = picker_pos.x;
    for (size_t i = 0; i < kHueSegments; ++i)
    {
        const auto col1 = Hct(col_hues[i].get_hue(), chroma, kToneDefault);
        const auto col2 = Hct(col_hues[i + 1].get_hue(), chroma, kToneDefault);

        DrawColorBar(draw_list, ImVec2(segment_minX, picker_pos.y), ImVec2(segment_w, pickerSize.y), col1, col2);

        segment_minX += segment_w;
    }

    HandlerPickerCursor(draw_list, "hue", hue, kHueMax, picker_pos, pickerSize);
}

void DrawChromaPicker(const float hue, float &chroma, const ImVec2 &pickerSize)
{
    constexpr float kChromaMin = 0.0F;
    constexpr float kChromaMax = 150.0F;

    auto *draw_list = ImGui::GetWindowDrawList();

    const ImVec2 picker_pos = ImGui::GetCursorScreenPos();

    const auto col1 = Hct(hue, kChromaMin, kToneDefault);
    const auto col2 = Hct(hue, kChromaMax, kToneDefault);
    DrawColorBar(draw_list, picker_pos, pickerSize, col1, col2);
    HandlerPickerCursor(draw_list, "chroma", chroma, kChromaMax, picker_pos, pickerSize);
}

void DrawTonePicker(const float hue, const float chroma, float &tone, const ImVec2 &pickerSize)
{
    constexpr float kToneMin = 0.0F;
    constexpr float kToneMax = 100.0F;
    constexpr int   segments = 3;
    constexpr float toneStep = kToneMax / static_cast<float>(segments);

    auto *draw_list = ImGui::GetWindowDrawList();

    const ImVec2 picker_pos = ImGui::GetCursorScreenPos();
    const float  segment_w  = pickerSize.x / static_cast<float>(segments);

    float segment_minX = picker_pos.x;
    float tone0        = kToneMin;
    for (int i = 0; i < segments; ++i)
    {
        const auto col1 = Hct(hue, chroma, tone0);
        const auto col2 = Hct(hue, chroma, tone0 + toneStep);

        DrawColorBar(draw_list, {segment_minX, picker_pos.y}, {segment_w, pickerSize.y}, col1, col2);

        tone0 += toneStep;
        segment_minX += segment_w;
    }

    HandlerPickerCursor(draw_list, "tone", tone, kToneMax, picker_pos, pickerSize);
}

void HexRgbInputText(AppearancePanel::HctCache &hctCache)
{
    const Hct   hct(hctCache.hue, hctCache.chroma, hctCache.tone);
    std::string buffer = std::format("#{:06X}", hct.ToInt() & 0xFFFFFFU);

    constexpr size_t BUFFER_SIZE = 64U;
    buffer.reserve(BUFFER_SIZE);

    if (ImGuiEx::M3::OutlinedTextField("RGB", buffer.data(), buffer.capacity()))
    {
        std::string_view view = buffer;
        for (const auto &c : view)
        {
            if (c == '#' || std::isspace(c) != 0)
            {
                view.remove_prefix(1U);
                continue;
            }
            break;
        }

        std::array<int, 3U> color{};
        constexpr size_t    col_r_idx = 0U;
        constexpr size_t    col_g_idx = 0U;
        constexpr size_t    col_b_idx = 0U;

        const char *pHexColor = view.data();
        size_t      j         = 0U;
        while (j < color.size())
        {
            auto [p, err] = std::from_chars(pHexColor, pHexColor + 2, color[j++], 16);
            if (*p == '\0' || err != std::errc())
            {
                break;
            }
            pHexColor = p;
        }
        if (j == color.size())
        {
            auto new_Hct    = Hct(material_color_utilities::ArgbFromRgb(color[col_r_idx], color[col_g_idx], color[col_b_idx]));
            hctCache.hue    = static_cast<float>(new_Hct.get_hue());
            hctCache.chroma = static_cast<float>(new_Hct.get_chroma());
            hctCache.tone   = static_cast<float>(new_Hct.get_tone());
        }
    }
}

auto HctPickerPopup(const char *strId, AppearancePanel::HctCache &hctCache) -> bool
{
    bool applied = false;

    auto &m3Styles = ImGuiEx::M3::Context::GetM3Styles();
    if (auto dialog = ImGuiEx::M3::DialogModal(strId); dialog)
    {
        HexRgbInputText(hctCache);

        const ImVec2 pickerSSize(ImGui::GetContentRegionAvail().x, m3Styles.GetPixels(M3Spec::SmallSlider::frameHeight));

        dialog.SupportingText(std::format("Hue: {:.3f}", hctCache.hue));
        DrawHuePicker(hctCache.hue, hctCache.chroma, pickerSSize);

        dialog.SupportingText(std::format("Chroma: {:.3f}", hctCache.chroma));
        DrawChromaPicker(hctCache.hue, hctCache.chroma, pickerSSize);

        dialog.SupportingText(std::format("Tone: {:.3f}", hctCache.tone));
        DrawTonePicker(hctCache.hue, hctCache.chroma, hctCache.tone, pickerSSize);

        if (dialog.ActionButton(Translate("Settings.Apply"), ICON_CHECK))
        {
            applied = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (dialog.ActionButton(Translate("Settings.Cancel"), ICON_X))
        {
            ImGui::CloseCurrentPopup();
        }
    }
    return applied;
}

} // namespace

AppearancePanel::AppearancePanel()
{
    i18n::ScanLanguages(utils::GetInterfacePath() / SIMPLE_IME, m_translateLanguages);
    // Translator lifecycle is managed by ToolWindow. See docs/adr/0001-toolwindow-translator-lifecycle.md
}

void AppearancePanel::Draw(Settings &settings)
{
    auto      &m3Styles   = ImGuiEx::M3::Context::GetM3Styles();
    const auto styleGuard = ImGuiEx::StyleGuard().Color<ImGuiCol_ChildBg>(m3Styles.Colors()[M3Spec::ColorRole::surfaceContainerLowest]);
    if (ImGui::BeginChild("##Appearance", {}, ImGuiEx::ChildFlags().AlwaysUseWindowPadding()))
    {
        if (ImGui::BeginTable("CenterAlignTable", 3, ImGuiEx::TableFlags().SizingStretchSame()))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            if (ImGui::TableNextColumn())
            {
                DrawZoomCombo(settings);
                ImGui::Spacing();
                DrawThemeBuilder(settings);
                DrawLanguagesCombo(settings.appearance);
                ImGuiEx::M3::Checkbox(Translate("Settings.Appearance.VerticalCandidateList"), settings.appearance.verticalCandidateList, ICON_CHECK);
                ImGuiEx::M3::Checkbox(Translate("Settings.Appearance.AutoToggleLanguageBar"), settings.appearance.autoToggleLanguageBar, ICON_CHECK);
            }

            ImGui::TableNextColumn();
            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void AppearancePanel::DrawZoomCombo(Settings &settings)
{
    if (ImGuiEx::M3::BeginCombo(Translate("Settings.Appearance.Zoom"), std::format("{}%", m_currentZoomPercent)))
    {
        for (uint32_t zoom = ZOOM_MIN_PERCENT; zoom <= ZOOM_MAX_PERCENT; zoom += ZOOM_STEP_PERCENT)
        {
            if (const bool selected = (zoom == m_currentZoomPercent); //
                ImGuiEx::M3::MenuItem(std::format("{}%", zoom), selected) && !selected)
            {
                const float uiScale = static_cast<float>(zoom) / 100.F;
                ImGuiEx::M3::Context::GetM3Styles().UpdateScaling(uiScale);
                settings.appearance.zoom = uiScale;
                m_currentZoomPercent     = zoom;
            }
        }
        ImGuiEx::M3::EndCombo();
    }
}

void AppearancePanel::DrawThemeBuilder(Settings &settings)
{
    auto      &m3Styles  = ImGuiEx::M3::Context::GetM3Styles();
    const auto fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();

    bool           edited           = false;
    constexpr auto colorButtonFlags = ImGuiEx::ColorEditFlags().NoAlpha().AlphaOpaque().NoPicker().NoTooltip();
    {
        const auto &schemeConfig = m3Styles.Colors().GetSchemeConfig();
        bool        openPopup    = false;
        ImGuiEx::M3::ListItem([&] -> void {
            const auto size = ImGuiEx::M3::ListLeadingImageSize();
            openPopup       = ImGui::ColorButton("##SourceColor", ImGuiEx::M3::ArgbToImVec4(schemeConfig.sourceColor), colorButtonFlags, size);
            ImGui::SameLine();
            ImGuiEx::M3::AlignedLabel(Translate("Settings.Appearance.ThemeColor"));
        });
        if (openPopup)
        {
            ImGui::OpenPopup("ThemeBuilder");
            const Hct hct(schemeConfig.sourceColor);
            m_configuredHct.hue    = static_cast<float>(hct.get_hue());
            m_configuredHct.chroma = static_cast<float>(hct.get_chroma());
            m_configuredHct.tone   = static_cast<float>(hct.get_tone());

            m_configuredDarkMode = schemeConfig.darkMode;
            edited               = true;
        }
    }
    constexpr auto HCT_PICKER_POPUP = "##HctPickerPopup";

    if (auto dialog = ImGuiEx::M3::DialogModal(Translate("Settings.Appearance.ThemeBuilder")); dialog)
    {
        const auto hctPickerPopupId = ImGui::GetID(HCT_PICKER_POPUP);

        edited = HctPickerPopup(HCT_PICKER_POPUP, m_configuredHct) || edited;
        edited = ImGuiEx::M3::Checkbox(Translate("Settings.Appearance.DarkMode"), m_configuredDarkMode, ICON_CHECK) || edited;
        edited = ImGuiEx::M3::Slider::Draw(
                     Translate("Settings.Appearance.ContrastLevel"),
                     ImGuiEx::M3::Slider::Params{
                         .value = m_configuredContrastLevel, .minValue = ImGuiEx::M3::CONTRAST_MIN, .maxValue = ImGuiEx::M3::CONTRAST_MAX
                     }
                 ) ||
                 edited;
        if (edited)
        {
            m_configuredScheme = std::make_unique<Scheme>(
                Hct(m_configuredHct.hue, m_configuredHct.chroma, m_configuredHct.tone), m_configuredDarkMode, m_configuredContrastLevel
            );
        }

        if (m_configuredScheme)
        {
            const auto paletteSize = ImGuiEx::M3::ListLeadingImageSize();

            auto draw_palette = [&](std::string_view label, const auto &palette) -> void {
                ImGuiEx::M3::ListItemPlain([&] -> void {
                    const auto cursorPos = ImGui::GetCursorScreenPos();
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        cursorPos,
                        {cursorPos.x + paletteSize.x, cursorPos.y + paletteSize.y},
                        ImGuiEx::M3::ArgbToImU32(palette.get_key_color().ToInt())
                    );
                    ImGui::Dummy(paletteSize);
                    ImGui::SameLine();
                    ImGuiEx::M3::AlignedLabel(label);
                });
            };

            ImGuiEx::M3::ListItem([&] -> void {
                if (ImGui::ColorButton(
                        "Primary",
                        ImGuiEx::M3::ArgbToImVec4(m_configuredScheme->primary_palette.get_key_color().ToInt()),
                        colorButtonFlags,
                        paletteSize
                    ))
                {
                    ImGui::OpenPopup(hctPickerPopupId);
                }
                ImGui::SameLine();
                ImGuiEx::M3::AlignedLabel("Primary");
            });
            draw_palette("Secondary", m_configuredScheme->secondary_palette);
            draw_palette("Tertiary", m_configuredScheme->tertiary_palette);
            draw_palette("Neutral", m_configuredScheme->neutral_palette);
            draw_palette("NeutralVariant", m_configuredScheme->neutral_variant_palette);
            draw_palette("Error", m_configuredScheme->error_palette);
        }

        if (dialog.ActionButton(Translate("Settings.Apply"), ICON_CHECK))
        {
            const ImGuiEx::M3::SchemeConfig schemeConfig{
                .contrastLevel = m_configuredContrastLevel,
                .sourceColor   = Hct(m_configuredHct.hue, m_configuredHct.chroma, m_configuredHct.tone).ToInt(),
                .darkMode      = m_configuredDarkMode
            };
            m3Styles.RebuildColors(schemeConfig);
            ImGuiEx::M3::SetupDefaultImGuiStyles(ImGui::GetStyle());
            settings.appearance.schemeConfig = schemeConfig;
            m_configuredScheme.reset();
        }

        ImGui::SameLine();
        (void)dialog.ActionButton(Translate("Settings.Cancel"), ICON_X);
    }
}

void AppearancePanel::DrawLanguagesCombo(Settings::Appearance &appearance) const
{
    bool clicked = false;
    if (ImGuiEx::M3::BeginCombo(Translate("Settings.Appearance.Languages"), appearance.language))
    {
        int32_t idx = 0;
        for (const auto &language : m_translateLanguages)
        {
            ImGui::PushID(idx);
            const bool isSelected = appearance.language == language;
            if (ImGuiEx::M3::MenuItem(language, isSelected) && !isSelected)
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
        ImGuiEx::M3::EndCombo();
    }

    if (clicked)
    {
        i18n::UpdateTranslator(appearance.language, "english");
    }
}
} // namespace Ime
