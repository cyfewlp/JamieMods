#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <cstdint>

namespace LIBC_NAMESPACE_DECL
{
namespace Setting
{
struct UiSetting
{
    static constexpr std::string_view SETTING_NAME   = "UiSetting";
    static constexpr float            FONT_SCALE_MIN = 0.5F;
    static constexpr float            FONT_SCALE_MAX = 5.0F;

    enum DefaultThemeIndex
    {
        DefaultThemeIndex_Classic = -1,
        DefaultThemeIndex_Dark    = -2,
        DefaultThemeIndex_Light   = -3,
        DefaultThemeIndex_Invalid = -4,
    };

    int32_t selectedThemeIndex = DefaultThemeIndex_Invalid; // setting key: selectedThemeIndex
    float   globalFontScale    = 1.0F;                      // setting key: globalFontScale

    void Reset()
    {
        globalFontScale    = 1.0F;
        selectedThemeIndex = DefaultThemeIndex_Invalid;
    }

    static auto GetInstance() -> UiSetting *;
};

void *UiSettingReadOpenFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name);
void  UiSettingReadLineFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line);
void  UiSettingWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf);
void  UiSettingClearAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler);

/**
 * called in ImGui::NewFrame
 */
void UiSettingApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler) noexcept(false);
}
}