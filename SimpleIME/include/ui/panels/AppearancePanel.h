//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "cpp/scheme/scheme_tonal_spot.h"
#include "ui/Settings.h"

namespace Ime
{
class AppearancePanel
{
    using Scheme                                   = material_color_utilities::SchemeTonalSpot;
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
    std::unique_ptr<Scheme>  m_configuredScheme{nullptr};
    HctCache                 m_configuredHct{};
    uint32_t                 m_currentZoomPercent      = ZOOM_DEFAULT_PERCENT;
    double                   m_configuredContrastLevel = 0.0;
    bool                     m_configuredDarkMode      = false;

public:
    explicit AppearancePanel();

    void Draw(Settings &settings);

private:
    void DrawZoomCombo(Settings &settings);
    void DrawThemeBuilder(Settings &settings);
    void DrawLanguagesCombo(Settings::Appearance &appearance) const;
};

} // namespace Ime
