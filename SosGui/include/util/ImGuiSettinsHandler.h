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
            static constexpr std::string_view SETTING_NAME = "UiSetting";
            static constexpr float FONT_SCALE_MIN = 0.5F;
            static constexpr float FONT_SCALE_MAX = 5.0F;

            int32_t selectedThemeIndex = -1;   // setting key: selectedThemeIndex
            float   globalFontScale    = 1.0F; // setting key: globalFontScale

            void Reset()
            {
                globalFontScale    = 1.0F;
                selectedThemeIndex = -1;
            }

            static auto GetInstance() -> UiSetting *;
        };

        void *UiSettingReadOpenFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name);
        void  UiSettingReadLineFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line);
        void  UiSettingWriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf);
        void  UiSettingClearAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler);
        void  UiSettingApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler);
    }
}