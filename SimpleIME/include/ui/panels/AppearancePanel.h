//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "common/imgui/Material3.h"
#include "i18n/TranslatorHolder.h"
#include "imgui.h"
#include "ui/Settings.h"

namespace ImGuiEx::M3
{
class M3Styles;
}

namespace Ime
{

class AppearancePanel
{
    using i18nHandle = std::optional<TranslatorHolder::UpdateHandle>;

    ImColor                  m_colorInThemeBuilder;
    bool                     m_darkModeInThemeBuilder = false;
    std::vector<std::string> m_translateLanguages;
    i18nHandle               m_i18nHandle;

public:
    static constexpr auto ZOOM_MAX = 2.0f;
    static constexpr auto ZOOM_MIN = 0.5f;

    explicit AppearancePanel() = default;

    void Draw(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles);

private:
    static void DrawZoomCombo(ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawThemeBuilder(ImGuiEx::M3::M3Styles &m3Styles);
    void        DrawLanguagesCombo(Settings::Appearance &appearance) const;

public:
    void ApplySettings(Settings::Appearance &appearance, ImGuiEx::M3::M3Styles &m3Styles);

private:
    void LoadTranslation(std::string_view language) const;
};

}
