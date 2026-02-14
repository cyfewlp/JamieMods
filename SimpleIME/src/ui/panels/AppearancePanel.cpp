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

constexpr auto ArgbToImU32(uint32_t argb) -> ImU32
{
    return (argb & 0xFF000000U) | (argb & 0x000000FFU) << 16 | (argb & 0x0000FF00U) | (argb & 0x00FF0000U) >> 16;
};

void HandlerPickerCursor(
    ImDrawList *drawList, const char *strId, float &value, float maxValue, const ImVec2 &pickerPos, const ImVec2 &size
)
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
        ArgbToImU32(col1.ToInt()),
        ArgbToImU32(col2.ToInt()),
        ArgbToImU32(col2.ToInt()),
        ArgbToImU32(col1.ToInt())
    );
}

void DrawHuePicker(float &hue, float chroma, const float pickerHeight)
{
    constexpr int   kHueSegments = 6;
    constexpr float kHueMax      = 360.0F;

    const Hct col_hues[kHueSegments + 1] = {
        Hct(0xFFFF0000),
        Hct(0xFFFFFF00),
        Hct(0xFF00FF00),
        Hct(0xFF00FFFF),
        Hct(0xFF0000FF),
        Hct(0xFFFF00FF),
        Hct(0xFFFF0000)
    };

    auto       *draw_list = ImGui::GetWindowDrawList();
    const float avail_x   = ImGui::GetContentRegionAvail().x;

    const ImVec2 picker_pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(avail_x, pickerHeight);
    const float  segment_w    = avail_x / static_cast<float>(kHueSegments);
    float        segment_minX = picker_pos.x;
    for (int i = 0; i < kHueSegments; ++i)
    {
        const auto col1 = Hct(col_hues[i].get_hue(), chroma, kToneDefault);
        const auto col2 = Hct(col_hues[i + 1].get_hue(), chroma, kToneDefault);

        DrawColorBar(draw_list, ImVec2(segment_minX, picker_pos.y), ImVec2(segment_w, size.y), col1, col2);

        segment_minX += segment_w;
    }

    HandlerPickerCursor(draw_list, "hue", hue, kHueMax, picker_pos, size);
}

void DrawChromaPicker(const float hue, float &chroma, const float pickerHeight)
{
    constexpr float kChromaMin = 0.0F;
    constexpr float kChromaMax = 150.0F;

    auto       *draw_list = ImGui::GetWindowDrawList();
    const float avail_x   = ImGui::GetContentRegionAvail().x;

    const ImVec2 picker_pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(avail_x, pickerHeight);

    const auto col1 = Hct(hue, kChromaMin, kToneDefault);
    const auto col2 = Hct(hue, kChromaMax, kToneDefault);
    DrawColorBar(draw_list, picker_pos, size, col1, col2);
    HandlerPickerCursor(draw_list, "chroma", chroma, kChromaMax, picker_pos, size);
}

void DrawTonePicker(const float hue, const float chroma, float &tone, const float pickerHeight)
{
    constexpr float kToneMin = 0.0F;
    constexpr float kToneMax = 100.0F;
    constexpr int   segments = 3;
    constexpr float toneStep = kToneMax / static_cast<float>(segments);

    auto       *draw_list = ImGui::GetWindowDrawList();
    const float avail_x   = ImGui::GetContentRegionAvail().x;

    const ImVec2 picker_pos = ImGui::GetCursorScreenPos();
    const ImVec2 size(avail_x, pickerHeight);
    const float  segment_w = avail_x / static_cast<float>(segments);

    float segment_minX = picker_pos.x;
    float tone0        = kToneMin;
    for (int i = 0; i < segments; ++i)
    {
        const auto col1 = Hct(hue, chroma, tone0);
        const auto col2 = Hct(hue, chroma, tone0 + toneStep);

        DrawColorBar(draw_list, {segment_minX, picker_pos.y}, {segment_w, size.y}, col1, col2);

        tone0 += toneStep;
        segment_minX += segment_w;
    }

    HandlerPickerCursor(draw_list, "tone", tone, kToneMax, picker_pos, size);
}

void HexRgbInputText(AppearancePanel::HctCache &hctCache, const ImGuiEx::M3::M3Styles &m3Styles)
{
    const Hct   hct(hctCache.hue, hctCache.chroma, hctCache.tone);
    std::string buffer = std::format("#{:06X}", hct.ToInt() & 0xFFFFFFU);

    const auto fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::BodyLarge>();
    const auto styleGuard =
        ImGuiEx::StyleGuard()
            .Style<ImGuiStyleVar_FramePadding>(
                {m3Styles.GetPixels(M3Spec::TextFieldBase::paddingX),
                 m3Styles.GetPixels(M3Spec::TextFieldBase::paddingY) + m3Styles.GetLastText().currHalfLineGap}
            )
            .Color<ImGuiCol_FrameBg>(m3Styles.Colors()[ColorRole::surfaceContainerHighest])
            .Color<ImGuiCol_FrameBgHovered>(
                m3Styles.Colors().Hovered(ColorRole::surfaceContainerHighest, ColorRole::onSurface)
            )
            .Color<ImGuiCol_FrameBgActive>(
                m3Styles.Colors().Pressed(ColorRole::surfaceContainerHighest, ColorRole::onSurface)
            );
    constexpr size_t BUFFER_SIZE = 64U;
    buffer.reserve(BUFFER_SIZE);
    if (ImGui::InputText("##Text", buffer.data(), buffer.capacity(), ImGuiInputTextFlags_CharsUppercase))
    {
        std::string_view view = buffer;
        for (auto &c : view)
        {
            if (c == '#' || std::isspace(c) != 0)
            {
                view.remove_prefix(1U);
                continue;
            }
            break;
        }

        std::array<int, 3U> color;

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
            auto new_Hct    = Hct(material_color_utilities::ArgbFromRgb(color[0U], color[1U], color[2U]));
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
auto HctPickerPopup(const char *strId, AppearancePanel::HctCache &hctCache, const ImGuiEx::M3::M3Styles &m3Styles)
    -> bool
{
    bool       applied = false;
    const auto popupStyleGuard =
        ImGuiEx::StyleGuard().Color<ImGuiCol_PopupBg>(m3Styles.Colors()[ColorRole::surfaceContainerHighest]);
    if (!ImGui::BeginPopup(strId))
    {
        return applied;
    }

    HexRgbInputText(hctCache, m3Styles);

    const auto pickerHeight = m3Styles.GetPixels(M3Spec::SmallSlider::frameHeight);

    ImGuiEx::M3::TextUnformatted(std::format("Hue: {:.3f}", hctCache.hue), m3Styles);
    DrawHuePicker(hctCache.hue, hctCache.chroma, pickerHeight);

    ImGuiEx::M3::TextUnformatted(std::format("Chroma: {:.3f}", hctCache.chroma), m3Styles);
    DrawChromaPicker(hctCache.hue, hctCache.chroma, pickerHeight);

    ImGuiEx::M3::TextUnformatted(std::format("Tone: {:.3f}", hctCache.tone), m3Styles);
    DrawTonePicker(hctCache.hue, hctCache.chroma, hctCache.tone, pickerHeight);

    {
        const auto buttonStyleGuard =
            ImGuiEx::StyleGuard()
                .Color<ImGuiCol_Text>(m3Styles.Colors()[ColorRole::onPrimary])
                .Style<ImGuiStyleVar_FramePadding>(m3Styles.GetPadding<M3Spec::SmallButton>())
                .Style<ImGuiStyleVar_FrameRounding>(m3Styles.GetRounding<M3Spec::SmallButton>());
        if (ImGui::Button(Translate("Settings.Appearance.Apply")))
        {
            applied = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, m3Styles.GetGap<M3Spec::SmallButtonGroup>());
        if (ImGui::Button(Translate("Settings.Appearance.Cancel")))
        {
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::EndPopup();
    return applied;
}

} // namespace

void AppearancePanel::Draw(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGuiEx::StyleGuard styleGuard;
    styleGuard
        .Style<ImGuiStyleVar_WindowPadding>({m3Styles[ImGuiEx::M3::Spacing::L], m3Styles[ImGuiEx::M3::Spacing::L]})
        .Style<ImGuiStyleVar_ItemSpacing>({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::Double_M]})
        .Color<ImGuiCol_Text>(m3Styles.Colors().at(M3Spec::ColorRole::onSurface))
        .Color<ImGuiCol_ChildBg>(m3Styles.Colors().at(M3Spec::ColorRole::surface));
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
    const auto _ = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();

    ImGuiEx::StyleGuard styleGuard;
    styleGuard.Color<ImGuiCol_Text>(m3Styles.Colors().at(ColorRole::onSurfaceVariant))
        .Style<ImGuiStyleVar_FrameBorderSize>(m3Styles[ImGuiEx::M3::Spacing::XS])
        .Style<ImGuiStyleVar_WindowPadding>(
            {m3Styles.GetPixels(M3Spec::Menu::paddingX), m3Styles.GetPixels(M3Spec::Menu::paddingY)}
        )
        .Color<ImGuiCol_Border>(m3Styles.Colors().at(ColorRole::primary))
        .Color<ImGuiCol_FrameBg>(m3Styles.Colors().at(ColorRole::surface))
        .Color<ImGuiCol_FrameBgHovered>(m3Styles.Colors().Hovered(ColorRole::surface, ColorRole::onSurfaceVariant))
        .Color<ImGuiCol_PopupBg>(m3Styles.Colors().at(ColorRole::surfaceContainerLow))
        .Color<ImGuiCol_HeaderActive>(m3Styles.Colors().at(ColorRole::tertiaryContainer))
        .Color<ImGuiCol_HeaderHovered>(m3Styles.Colors().Hovered(ColorRole::surfaceContainerLow, ColorRole::onSurface));
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

//! \todo need refactor ColorPicker style. maybe we can add a HUE wheel in the future.
void AppearancePanel::DrawThemeBuilder(ImGuiEx::M3::M3Styles &m3Styles)
{
    const auto &colors       = m3Styles.Colors();
    const auto &schemeConfig = m3Styles.Colors().GetSchemeConfig();
    bool        openPopup    = false;
    const auto  _            = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();

    constexpr auto colorButtonFlags = ImGuiEx::ColorEditFlags().NoAlpha().NoPicker().NoTooltip();
    {
        const auto s = ImGuiEx::StyleGuard()
                           .Style<ImGuiStyleVar_FramePadding>({0.f, m3Styles[ImGuiEx::M3::Spacing::M]})
                           .Color<ImGuiCol_ChildBg>(colors[ColorRole::surface]);

        ImGuiEx::M3::ListItem("ThemeBuilderList", m3Styles, [&] {
            const auto size = ImGuiEx::M3::ListLeadingImageSize(m3Styles);
            openPopup       = ImGui::ColorButton(
                "##SourceColor", ImGuiEx::M3::ArgbToImVec4(schemeConfig.sourceColor), colorButtonFlags, size
            );
            ImGui::SameLine();
            ImGuiEx::M3::AlignedLabel(Translate("Settings.Appearance.ThemeColor"), m3Styles);
        });
    }
    bool edited = false;
    if (openPopup)
    {
        ImGui::OpenPopup("ThemeBuilder");
        ImGui::SetNextWindowSize({m3Styles.GetPixels(M3Spec::Layout::breakpointMedium), 0.F});
        const Hct hct(schemeConfig.sourceColor);
        m_hctCache.hue    = static_cast<float>(hct.get_hue());
        m_hctCache.chroma = static_cast<float>(hct.get_chroma());
        m_hctCache.tone   = static_cast<float>(hct.get_tone());

        m_darkModeTemp = schemeConfig.darkMode;
        edited         = true;
    }
    constexpr auto HCT_PICKER_POPUP = "##HctPickerPopup";

    const auto styleGuard =
        ImGuiEx::StyleGuard()
            .Style<ImGuiStyleVar_FramePadding>({0.f, m3Styles[ImGuiEx::M3::Spacing::S]})
            .Style<ImGuiStyleVar_WindowPadding>({m3Styles[ImGuiEx::M3::Spacing::M], m3Styles[ImGuiEx::M3::Spacing::M]})
            .Color<ImGuiCol_PopupBg>(colors[ColorRole::surfaceContainerHigh]);
    bool open = true;
    if (ImGui::BeginPopupModal(Translate("Settings.Appearance.ThemeBuilder"), &open, ImGuiEx::WindowFlags()))
    {
        static std::unique_ptr<Scheme> scheme = nullptr;

        const auto hctPickerPopupId = ImGui::GetID(HCT_PICKER_POPUP);

        edited = HctPickerPopup(HCT_PICKER_POPUP, m_hctCache, m3Styles) || edited;
        edited = ImGui::Checkbox(Translate("Settings.Appearance.DarkMode"), &m_darkModeTemp) || edited;
        edited = ImGuiEx::M3::Slider::Draw(
                     Translate("Settings.Appearance.ContrastLevel"),
                     ImGuiEx::M3::Slider::Params{
                         m_contrastLevelTemp, ImGuiEx::M3::CONTRAST_MIN, ImGuiEx::M3::CONTRAST_MAX, m3Styles
                     }
                 ) ||
                 edited;
        if (edited)
        {
            scheme = std::make_unique<Scheme>(
                Hct(m_hctCache.hue, m_hctCache.chroma, m_hctCache.tone), m_darkModeTemp, m_contrastLevelTemp
            );
        }

        if (scheme)
        {
            const auto paletteSize = ImGuiEx::M3::ListLeadingImageSize(m3Styles);

            auto draw_palette = [&](std::string_view label, const auto &palette) {
                ImGuiEx::M3::ListItemPlain(label, m3Styles, [&] {
                    const auto cursorPos = ImGui::GetCursorScreenPos();
                    ImGui::GetWindowDrawList()->AddRectFilled(
                        cursorPos,
                        {cursorPos.x + paletteSize.x, cursorPos.y + paletteSize.y},
                        ArgbToImU32(palette.get_key_color().ToInt())
                    );
                    ImGui::Dummy(paletteSize);
                    ImGui::SameLine();
                    ImGuiEx::M3::AlignedLabel(label, m3Styles);
                });
            };

            ImGuiEx::M3::ListItem("Primary", m3Styles, [&] {
                if (ImGui::ColorButton(
                        "Primary",
                        ImGuiEx::M3::ArgbToImVec4(scheme->primary_palette.get_key_color().ToInt()),
                        colorButtonFlags,
                        paletteSize
                    ))
                {
                    ImGui::OpenPopup(hctPickerPopupId);
                }
                ImGui::SameLine();
                ImGuiEx::M3::AlignedLabel("Primary", m3Styles);
            });
            draw_palette("Secondary", scheme->secondary_palette);
            draw_palette("Tertiary", scheme->tertiary_palette);
            draw_palette("Neutral", scheme->neutral_palette);
            draw_palette("NeutralVariant", scheme->neutral_variant_palette);
            draw_palette("Error", scheme->error_palette);
        }

        if (ImGui::Button(Translate("Settings.Appearance.Apply")))
        {
            m3Styles.RebuildColors(
                {m_contrastLevelTemp, Hct(m_hctCache.hue, m_hctCache.chroma, m_hctCache.tone).ToInt(), m_darkModeTemp}
            );
            UI::ApplyM3Theme(m3Styles);
            scheme.reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, m3Styles.GetGap<M3Spec::SmallButtonGroup>());
        if (ImGui::Button(Translate("Settings.Appearance.Cancel")))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
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
