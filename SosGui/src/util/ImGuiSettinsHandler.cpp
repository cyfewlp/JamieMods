#include "util/ImGuiSettinsHandler.h"

#include "common/config.h"
#include "common/imgui/ImThemeLoader.h"
#include "common/log.h"
#include "gui/UiSetting.h"
#include "imgui_internal.h"
#include "util/utils.h"

#include <cstdint>
#include <imgui.h>

namespace LIBC_NAMESPACE_DECL
{
void *Setting::UiSettingReadOpenFn(ImGuiContext * /*ctx*/, ImGuiSettingsHandler * /*handler*/, const char *name)
{
    if (UiSetting::SETTING_NAME != name)
    {
        return nullptr;
    }
    auto *settings = UiSetting::GetInstance();
    settings->Reset();
    return settings;
}

void Setting::UiSettingReadLineFn(
    ImGuiContext * /*ctx*/, ImGuiSettingsHandler * /*handler*/, void *entry, const char *line
)
{
    UiSetting *uiSetting = static_cast<UiSetting *>(entry);
    int32_t    index     = -1;
    float      scale     = 1.0F;
    if (sscanf_s(line, "selectedThemeIndex=%d", &index, sizeof(int32_t)) == 1)
    {
        uiSetting->selectedThemeIndex = index;
    }
    else if (sscanf_s(line, "globalFontScale=%f", &scale, sizeof(float)) == 1)
    {
        if (scale >= UiSetting::FONT_SCALE_MIN && scale <= UiSetting::FONT_SCALE_MAX)
        {
            uiSetting->globalFontScale = scale;
        }
    }
}

void Setting::UiSettingWriteAll([[maybe_unused]] ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf)
{
    buf->reserve(buf->size() + sizeof(UiSetting) * 2); // ballpark reserve

    auto *setting = UiSetting::GetInstance();
    buf->appendf("[%s][%s]\n", handler->TypeName, UiSetting::SETTING_NAME.data());
    buf->appendf("selectedThemeIndex=%d\n", setting->selectedThemeIndex);
    buf->appendf("globalFontScale=%f\n", setting->globalFontScale);
    buf->appendf("\n");
}

void Setting::UiSettingClearAll(ImGuiContext * /*ctx*/, ImGuiSettingsHandler * /*handler*/)
{
    UiSetting::GetInstance()->Reset();
}

void Setting::UiSettingApplyAll(ImGuiContext * /*ctx*/, ImGuiSettingsHandler * /*handler*/)
{
    auto *setting                  = UiSetting::GetInstance();
    ImGui::GetIO().FontGlobalScale = setting->globalFontScale;
    auto &loader                   = ImTheme::Loader::GetInstance();
    if (loader.IsIndexInRange(setting->selectedThemeIndex))
    {
        const auto &themeFilePath = util::GetInterfaceFile(ImTheme::THEME_FILE_NAME);
        try
        {
            loader.UseTheme(setting->selectedThemeIndex, themeFilePath);
        }
        catch (ImTheme::Loader::Error &e)
        {
            log_error("Failed to load theme file {}: {}", themeFilePath.c_str(), e.what());
            setting->selectedThemeIndex = UiSetting::DefaultThemeIndex_Invalid;
        }
    }
}

auto Setting::UiSetting::GetInstance() -> UiSetting *
{
    static UiSetting g_instance;
    return &g_instance;
}

}