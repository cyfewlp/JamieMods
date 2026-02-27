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

    static constexpr uint32_t ZOOM_STEP_PERCENT    = 25U;
    static constexpr uint32_t ZOOM_MIN_PERCENT     = 50U;
    static constexpr uint32_t ZOOM_MAX_PERCENT     = 200U;
    static constexpr uint32_t ZOOM_DEFAULT_PERCENT = 100U;

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
    double                   m_contrastLevelTemp  = 0.0;
    uint32_t                 m_currentZoomPercent = ZOOM_DEFAULT_PERCENT;
    bool                     m_darkModeTemp       = false;

public:
    static constexpr auto ZOOM_MAX = 2.0f;
    static constexpr auto ZOOM_MIN = 0.5f;

    explicit AppearancePanel() = default;

    void Draw(Settings &settings);

private:
    void DrawZoomCombo();
    void DrawThemeBuilder();
    void DrawLanguagesCombo(Settings::Appearance &appearance) const;

public:
    void ApplySettings(Settings::Appearance &appearance);

private:
    void LoadTranslation(std::string_view language) const;
};

} // namespace Ime
