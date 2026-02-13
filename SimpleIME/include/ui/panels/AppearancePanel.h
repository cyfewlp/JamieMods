//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "i18n/TranslatorHolder.h"
#include "imgui.h"
#include "imguiex/Material3.h"
#include "ui/Settings.h"

namespace Ime
{

class AppearancePanel
{
    using i18nHandle = std::optional<TranslatorHolder::UpdateHandle>;

public:
    //! Cache each component of HCT separately, because each component may abruptly
    //! change at the boundary, such as: HUE 360 -> 0.
    struct HctCache
    {
        float hue;
        float chroma;
        float tone;
    };

private:
    std::vector<std::string> m_translateLanguages;
    i18nHandle               m_i18nHandle;
    HctCache                 m_hctCache;
    double                   m_contrastLevelTemp = 0.0;
    bool                     m_darkModeTemp      = false;

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

} // namespace Ime
