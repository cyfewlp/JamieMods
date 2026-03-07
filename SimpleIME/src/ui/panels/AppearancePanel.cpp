//
// Created by jamie on 2026/1/27.
//

#include "ui/panels/AppearancePanel.h"

#include "cpp/scheme/scheme_tonal_spot.h"
#include "i18n/TranslationLoader.h"
#include "i18n/TranslatorHolder.h"
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

#include <imgui.h>

namespace Ime
{

using ColorRole = M3Spec::ColorRole;

namespace
{
using Scheme = material_color_utilities::SchemeTonalSpot;
using Hct    = material_color_utilities::Hct;

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
    constexpr int   kHueSegments = 6;
    constexpr float kHueMax      = 360.0F;

    const Hct col_hues[kHueSegments + 1] = {
        Hct(0xFFFF0000), Hct(0xFFFFFF00), Hct(0xFF00FF00), Hct(0xFF00FFFF), Hct(0xFF0000FF), Hct(0xFFFF00FF), Hct(0xFFFF0000)
    };

    auto *draw_list = ImGui::GetWindowDrawList();

    const ImVec2 picker_pos   = ImGui::GetCursorScreenPos();
    const float  segment_w    = pickerSize.x / static_cast<float>(kHueSegments);
    float        segment_minX = picker_pos.x;
    for (int i = 0; i < kHueSegments; ++i)
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

// --------------------
// |      picker      |
// --------------------
// |  apply  | cancel |
// --------------------
auto HctPickerPopup(const char *strId, AppearancePanel::HctCache &hctCache) -> bool
{
    bool applied = false;

    auto      &m3Styles        = ImGuiEx::M3::Context::GetM3Styles();
    const auto popupStyleGuard = ImGuiEx::StyleGuard()
                                     .Color<ImGuiCol_PopupBg>(m3Styles.Colors()[ColorRole::surfaceContainerHighest])
                                     .Style<ImGuiStyleVar_WindowPadding>(m3Styles.GetPadding<M3Spec::Dialogs>());
    if (!ImGui::BeginPopup(strId))
    {
        return applied;
    }

    HexRgbInputText(hctCache);

    const auto   fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
    const ImVec2 pickerSSize(m3Styles.GetPixels(M3Spec::Dialogs::minWidth), m3Styles.GetPixels(M3Spec::SmallSlider::frameHeight));

    ImGuiEx::M3::TextUnformatted(std::format("Hue: {:.3f}", hctCache.hue));
    DrawHuePicker(hctCache.hue, hctCache.chroma, pickerSSize);

    ImGuiEx::M3::TextUnformatted(std::format("Chroma: {:.3f}", hctCache.chroma));
    DrawChromaPicker(hctCache.hue, hctCache.chroma, pickerSSize);

    ImGuiEx::M3::TextUnformatted(std::format("Tone: {:.3f}", hctCache.tone));
    DrawTonePicker(hctCache.hue, hctCache.chroma, hctCache.tone, pickerSSize);

    {
        if (ImGuiEx::M3::Button(Translate("Settings.Appearance.Apply"), {ICON_CHECK}))
        {
            applied = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, m3Styles.GetPixels(M3Spec::StandardSmallButtonGroup::BetweenSpace));
        if (ImGuiEx::M3::SmallButton(Translate("Settings.Appearance.Cancel"), ICON_X))
        {
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::EndPopup();
    return applied;
}

} // namespace

// \fixme After resize window, center align will broken.
void AppearancePanel::Draw(Settings &settings)
{
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

void AppearancePanel::DrawZoomCombo()
{
    if (ImGuiEx::M3::BeginCombo(Translate("Settings.Appearance.Zoom"), std::format("{}%", m_currentZoomPercent)))
    {
        for (uint32_t zoom = ZOOM_MIN_PERCENT; zoom <= ZOOM_MAX_PERCENT; zoom += ZOOM_STEP_PERCENT)
        {
            if (const bool selected = (zoom == m_currentZoomPercent); //
                ImGuiEx::M3::MenuItem(std::format("{}%", zoom), selected) && !selected)
            {
                ImGuiEx::M3::Context::GetM3Styles().UpdateScaling(static_cast<float>(zoom) / 100.F);
                m_currentZoomPercent = zoom;
            }
        }
        ImGuiEx::M3::EndCombo();
    }
}

//! \todo need refactor ColorPicker style. maybe we can add a HUE wheel in the future.
void AppearancePanel::DrawThemeBuilder()
{
    auto       &m3Styles     = ImGuiEx::M3::Context::GetM3Styles();
    const auto &colors       = m3Styles.Colors();
    const auto &schemeConfig = m3Styles.Colors().GetSchemeConfig();
    bool        openPopup    = false;
    const auto  fontScope    = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();

    constexpr auto colorButtonFlags = ImGuiEx::ColorEditFlags().NoAlpha().NoPicker().NoTooltip();
    {
        const auto s = ImGuiEx::StyleGuard()
                           .Style<ImGuiStyleVar_FramePadding>({0.f, m3Styles[ImGuiEx::M3::Spacing::M]})
                           .Color<ImGuiCol_ChildBg>(colors[ColorRole::surface]);

        ImGuiEx::M3::ListItem([&] {
            const auto size = ImGuiEx::M3::ListLeadingImageSize();
            openPopup       = ImGui::ColorButton("##SourceColor", ImGuiEx::M3::ArgbToImVec4(schemeConfig.sourceColor), colorButtonFlags, size);
            ImGui::SameLine();
            ImGuiEx::M3::AlignedLabel(Translate("Settings.Appearance.ThemeColor"));
        });
    }
    bool edited = false;
    if (openPopup)
    {
        ImGui::OpenPopup("ThemeBuilder");
        ImGui::SetNextWindowSize({m3Styles.GetPixels(M3Spec::Layout::Medium::Breakpoint), 0.F});
        const Hct hct(schemeConfig.sourceColor);
        m_hctCache.hue    = static_cast<float>(hct.get_hue());
        m_hctCache.chroma = static_cast<float>(hct.get_chroma());
        m_hctCache.tone   = static_cast<float>(hct.get_tone());

        m_darkModeTemp = schemeConfig.darkMode;
        edited         = true;
    }
    constexpr auto HCT_PICKER_POPUP = "##HctPickerPopup";

    const auto styleGuard = ImGuiEx::StyleGuard()
                                .Style<ImGuiStyleVar_FramePadding>({0.f, m3Styles[ImGuiEx::M3::Spacing::S]})
                                .Style<ImGuiStyleVar_WindowPadding>({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::M]})
                                .Color<ImGuiCol_PopupBg>(colors[ColorRole::surfaceContainer]);
    bool open = true;
    if (ImGui::BeginPopupModal(Translate("Settings.Appearance.ThemeBuilder"), &open, ImGuiEx::WindowFlags()))
    {
        static std::unique_ptr<Scheme> scheme = nullptr;

        const auto hctPickerPopupId = ImGui::GetID(HCT_PICKER_POPUP);

        edited = HctPickerPopup(HCT_PICKER_POPUP, m_hctCache) || edited;
        edited = ImGui::Checkbox(Translate("Settings.Appearance.DarkMode"), &m_darkModeTemp) || edited;
        edited = ImGuiEx::M3::Slider::Draw(
                     Translate("Settings.Appearance.ContrastLevel"),
                     ImGuiEx::M3::Slider::Params{m_contrastLevelTemp, ImGuiEx::M3::CONTRAST_MIN, ImGuiEx::M3::CONTRAST_MAX}
                 ) ||
                 edited;
        if (edited)
        {
            scheme = std::make_unique<Scheme>(Hct(m_hctCache.hue, m_hctCache.chroma, m_hctCache.tone), m_darkModeTemp, m_contrastLevelTemp);
        }

        if (scheme)
        {
            const auto paletteSize = ImGuiEx::M3::ListLeadingImageSize();

            auto draw_palette = [&](std::string_view label, const auto &palette) {
                ImGuiEx::M3::ListItemPlain([&] {
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

            ImGuiEx::M3::ListItem([&] {
                if (ImGui::ColorButton(
                        "Primary", ImGuiEx::M3::ArgbToImVec4(scheme->primary_palette.get_key_color().ToInt()), colorButtonFlags, paletteSize
                    ))
                {
                    ImGui::OpenPopup(hctPickerPopupId);
                }
                ImGui::SameLine();
                ImGuiEx::M3::AlignedLabel("Primary");
            });
            draw_palette("Secondary", scheme->secondary_palette);
            draw_palette("Tertiary", scheme->tertiary_palette);
            draw_palette("Neutral", scheme->neutral_palette);
            draw_palette("NeutralVariant", scheme->neutral_variant_palette);
            draw_palette("Error", scheme->error_palette);
        }

        if (ImGuiEx::M3::SmallButton(Translate("Settings.Appearance.Apply"), ICON_CHECK))
        {
            m3Styles.RebuildColors({m_contrastLevelTemp, Hct(m_hctCache.hue, m_hctCache.chroma, m_hctCache.tone).ToInt(), m_darkModeTemp});
            ImGuiEx::M3::SetupDefaultImGuiStyles(ImGui::GetStyle());
            scheme.reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, m3Styles.GetPixels(M3Spec::StandardSmallButtonGroup::BetweenSpace));
        if (ImGuiEx::M3::SmallButton(Translate("Settings.Appearance.Cancel"), ICON_X))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void AppearancePanel::DrawLanguagesCombo(Settings::Appearance &appearance) const
{
    bool clicked = false;
    if (ImGuiEx::M3::BeginCombo(Translate("Settings.Appearance.Languages"), appearance.language.c_str()))
    {
        int32_t idx = 0;
        for (const auto &language : m_translateLanguages)
        {
            ImGui::PushID(idx);
            const bool isSelected = appearance.language == language;
            if (ImGuiEx::M3::MenuItem(language.c_str(), isSelected) && !isSelected)
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
        LoadTranslation(appearance.language);
    }
}

void AppearancePanel::ApplySettings(Settings::Appearance &appearance)
{
    appearance.zoom = std::min(ZOOM_MAX, appearance.zoom);
    appearance.zoom = std::max(ZOOM_MIN, appearance.zoom);

    auto &m3Styles = ImGuiEx::M3::Context::GetM3Styles();
    m3Styles.UpdateScaling(appearance.zoom);
    ImGuiEx::M3::SetupDefaultImGuiStyles(ImGui::GetStyle());

    i18n::ScanLanguages(utils::GetInterfacePath() / SIMPLE_IME, m_translateLanguages);

    if (const auto langIt = std::ranges::find(m_translateLanguages, appearance.language); langIt == m_translateLanguages.end())
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
