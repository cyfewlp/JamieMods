//
// Created by jamie on 2026/1/27.
//

#pragma once

#include "common/config.h"
#include "i18n/TranslatorHolder.h"
#include "imgui.h"
#include "ui/Settings.h"

namespace LIBC_NAMESPACE_DECL
{
namespace ImGuiEx::M3
{
struct M3Styles;
}

namespace Ime
{

class AppearancePanel
{
    using i18nHandle = std::optional<TranslatorHolder::UpdateHandle>;

    // FIXME: Reference member violates C++ Core Guidelines (non-copyable/non-assignable).
    // Consider replacing with std::reference_wrapper or a pointer to allow proper
    // class movement and reassignment.
    ImGuiEx::M3::M3Styles   &m_styles;
    ImColor                  m_colorInThemeBuilder;
    bool                     m_darkModeInThemeBuilder = false;
    std::vector<std::string> m_translateLanguages;
    i18nHandle               m_i18nHandle;

public:
    static constexpr auto ZOOM_MAX = 2.0f;
    static constexpr auto ZOOM_MIN = 0.5f;

    explicit AppearancePanel(ImGuiEx::M3::M3Styles &styles) : m_styles(styles) {}

    void Draw(Settings &settings);

private:
    void DrawZoomCombo() const;
    void DrawThemeBuilder();
    void DrawLanguagesCombo(Settings::Appearance &appearance);

public:
    void ApplySettings(Settings::Appearance &appearance);

private:
    void ApplyM3Theme();
    void LoadTranslation(std::string_view language) const;
};

}
}
