//
// Created by jamie on 2025/5/15.
//

#include "SettingsPopup.h"

#include "Translation.h"
#include "common/imgui/ImGuiScope.h"
#include "common/imgui/ImThemeLoader.h"
#include "data/SosUiData.h"
#include "gui/UiSetting.h"
#include "imgui_internal.h"
#include "util/utils.h"

namespace LIBC_NAMESPACE_DECL
{
void Popup::SettingsPopup::DoDraw(SosUiData &uiData, bool &)
{
    if (ImGui::DragFloat(
            "$SosGui_Global_FontSize_Scale"_T.c_str(),
            &ImGui::GetIO().FontGlobalScale,
            0.05F,
            Setting::UiSetting::FONT_SCALE_MIN,
            Setting::UiSetting::FONT_SCALE_MAX
        ))
    {
        Setting::UiSetting::GetInstance()->globalFontScale = ImGui::GetIO().FontGlobalScale;
    }
    ThemeCombo(uiData);
}

void Popup::SettingsPopup::ThemeCombo(SosUiData &uiData)
{
    auto         &loader     = ImTheme::Loader::GetInstance();
    auto         *settings   = Setting::UiSetting::GetInstance();
    const int32_t themeIndex = settings->selectedThemeIndex;
    std::string   preview    = "";
    const auto   &themes     = loader.GetAvailableThemes();
    if (themeIndex > Setting::UiSetting::DefaultThemeIndex_Invalid)
    {
        switch (themeIndex)
        {
            case Setting::UiSetting::DefaultThemeIndex_Classic:
                preview = "Default: Classic";
                break;
            case Setting::UiSetting::DefaultThemeIndex_Dark:
                preview = "Default: Dark";
                break;
            case Setting::UiSetting::DefaultThemeIndex_Light:
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
    ImGuiScope::Combo combo("Theme", preview.c_str());
    if (!combo)
    {
        return;
    }

    if (ImGui::Selectable("Default: Classic", false))
    {
        ImGui::StyleColorsClassic();
        settings->selectedThemeIndex = Setting::UiSetting::DefaultThemeIndex_Classic;
    }

    if (ImGui::Selectable("Default: Dark", false))
    {
        ImGui::StyleColorsDark();
        settings->selectedThemeIndex = Setting::UiSetting::DefaultThemeIndex_Dark;
    }

    if (ImGui::Selectable("Default: Light", false))
    {
        ImGui::StyleColorsLight();
        settings->selectedThemeIndex = Setting::UiSetting::DefaultThemeIndex_Light;
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
                uiData.PushErrorMessage(std::format("Can't use theme {}: {}", theme.name, e.what()));
                settings->selectedThemeIndex = Setting::UiSetting::DefaultThemeIndex_Invalid;
            }
        }
    }

    if (themeIndex != settings->selectedThemeIndex)
    {
        ImGui::MarkIniSettingsDirty();
    }
}
}
