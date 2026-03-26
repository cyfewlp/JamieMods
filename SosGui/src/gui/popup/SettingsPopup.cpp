//
// Created by jamie on 2025/5/15.
//

#include "SettingsPopup.h"

#include "../font/FontManager.h"
#include "Translation.h"
#include "data/SosUiData.h"
#include "gui/UiSettings.h"
#include "imgui/ImThemeLoader.h"
#include "imgui_internal.h"
#include "util/utils.h"

namespace SosGui
{
void Popup::SettingsPopup::DoDraw(SosUiData &, bool &)
{
    FontManager::GetInstance().DrawPanel();

    ThemeCombo();
}

void Popup::SettingsPopup::ThemeCombo()
{
    const auto   &loader     = ImTheme::Loader::GetInstance();
    auto         *settings   = Settings::UiSettings::GetInstance();
    const int32_t themeIndex = settings->selectedThemeIndex;
    std::string   preview    = "";
    const auto   &themes     = loader.GetAvailableThemes();
    if (themeIndex > Settings::UiSettings::DefaultThemeIndex_Invalid)
    {
        switch (themeIndex)
        {
            case Settings::UiSettings::DefaultThemeIndex_Classic:
                preview = "Default: Classic";
                break;
            case Settings::UiSettings::DefaultThemeIndex_Dark:
                preview = "Default: Dark";
                break;
            case Settings::UiSettings::DefaultThemeIndex_Light:
                preview = "Default: Light";
                break;
            default: {
                if (loader.IsIndexInRange(themeIndex))
                {
                    preview = themes[themeIndex].name;
                }
                break;
            }
        }
    }
    if (!ImGui::BeginCombo("Theme", preview.c_str()))
    {
        return;
    }

    if (ImGui::Selectable("Default: Classic", false))
    {
        ImGui::StyleColorsClassic();
        settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Classic;
    }

    if (ImGui::Selectable("Default: Dark", false))
    {
        ImGui::StyleColorsDark();
        settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Dark;
    }

    if (ImGui::Selectable("Default: Light", false))
    {
        ImGui::StyleColorsLight();
        settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Light;
    }

    for (size_t index = 0; index < themes.size(); ++index)
    {
        const auto &theme = themes[index];
        if (ImGui::Selectable(theme.name.c_str(), false))
        {
            try
            {
                loader.UseTheme(index, util::GetInterfaceFile(ImTheme::THEME_FILE_NAME));
                settings->selectedThemeIndex = index;
            }
            catch (ImTheme::Loader::Error &e)
            {
                ErrorNotifier::GetInstance().Error(std::format("Can't use theme {}: {}", theme.name, e.what()));
                settings->selectedThemeIndex = Settings::UiSettings::DefaultThemeIndex_Invalid;
            }
        }
    }
    ImGui::EndCombo();

    if (themeIndex != settings->selectedThemeIndex)
    {
        ImGui::MarkIniSettingsDirty();
    }
}
} // namespace SosGui
