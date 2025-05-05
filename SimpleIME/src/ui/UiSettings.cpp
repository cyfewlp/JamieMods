//
// Created by jamie on 2025/5/5.
//

#include "ui/UiSettings.h"

#include "ImeUI.h"
#include "imgui_internal.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        void UiSettings::RegisterImGuiIniHandler(ImeUI *imeUi)
        {
            // add our setting handler
            ImGuiSettingsHandler ini_handler;
            ini_handler.TypeName   = SKSE::PluginDeclaration::GetSingleton()->GetName().data();
            ini_handler.TypeHash   = ImHashStr(ini_handler.TypeName);
            ini_handler.ReadOpenFn = ReadOpenFn;
            ini_handler.ReadLineFn = ReadLineFn;
            ini_handler.WriteAllFn = WriteAll;
            ini_handler.ClearAllFn = ClearAll;
            ini_handler.ApplyAllFn = ApplyAll;
            ini_handler.UserData   = imeUi;
            ImGui::AddSettingsHandler(&ini_handler);
        }

        void *UiSettings::ReadOpenFn(ImGuiContext *, ImGuiSettingsHandler *, const char *name)
        {
            if (SETTING_NAME != name)
            {
                return nullptr;
            }
            auto *settings = GetInstance();
            settings->Reset();
            return settings;
        }

        void UiSettings::ReadLineFn(ImGuiContext *, ImGuiSettingsHandler *, void *entry, const char *line)
        {
            UiSettings *uiSetting  = static_cast<UiSettings *>(entry);
            int32_t     int32Value = -1;
            uint8_t     uint8Value = 0;
            char        str64[64];
            if (sscanf_s(line, "showSettings=%d", &int32Value) == 1)
            {
                uiSetting->showSettings = int32Value != 0;
            }
            else if (sscanf_s(line, "enableMod=%d", &int32Value) == 1)
            {
                uiSetting->enableMod = int32Value != 0;
            }
            else if (sscanf_s(line, "usedLanguage=%s", &str64, sizeof(str64)) == 1)
            {
                uiSetting->usedLanguage = std::string(str64);
            }
            else if (sscanf_s(line, "focusType=%d", &int32Value) == 1)
            {
                if (int32Value == 0)
                {
                    uiSetting->focusType = Permanent;
                }
                else if (int32Value == 1)
                {
                    uiSetting->focusType = Temporary;
                }
            }
            else if (sscanf_s(line, "windowPosUpdatePolicy=%u", &uint8Value) == 1)
            {
                if (uint8Value == 1)
                {
                    uiSetting->windowPosUpdatePolicy = WindowPosPolicy::BASED_ON_CURSOR;
                }
                else if (uint8Value == 2)
                {
                    uiSetting->windowPosUpdatePolicy = WindowPosPolicy::BASED_ON_CARET;
                }
            }
            else if (sscanf_s(line, "enableUnicodePaste=%d", &int32Value) == 1)
            {
                uiSetting->enableUnicodePaste = int32Value != 0;
            }
            else if (sscanf_s(line, "keepImeOpen=%d", &int32Value) == 1)
            {
                uiSetting->keepImeOpen = int32Value != 0;
            }
            else if (sscanf_s(line, "usedThemeIndex=%d", &int32Value) == 1)
            {
                uiSetting->usedThemeIndex = int32Value != 0;
            }
        }

        void UiSettings::WriteAll(ImGuiContext *, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf)
        {
            buf->reserve(buf->size() + sizeof(UiSettings) * 2); // ballpark reserve

            auto *setting = GetInstance();
            buf->appendf("[%s][%s]\n", handler->TypeName, SETTING_NAME.data());
            buf->appendf("showSettings=%d\n", setting->showSettings ? 1 : 0);
            buf->appendf("enableMod=%d\n", setting->enableMod ? 1 : 0);
            buf->appendf("enableMod=%s\n", setting->usedLanguage.c_str());
            buf->appendf("focusType=%d\n", static_cast<int32_t>(setting->focusType));
            buf->appendf("windowPosUpdatePolicy=%u\n", static_cast<uint8_t>(setting->windowPosUpdatePolicy));
            buf->appendf("enableUnicodePaste=%d\n", setting->enableUnicodePaste ? 1 : 0);
            buf->appendf("keepImeOpen=%d\n", setting->keepImeOpen ? 1 : 0);
            buf->appendf("usedThemeIndex=%d\n", setting->usedThemeIndex);
            buf->appendf("\n");
        }

        void UiSettings::ClearAll(ImGuiContext *, ImGuiSettingsHandler *)
        {
            GetInstance()->Reset();
        }

        void UiSettings::ApplyAll(ImGuiContext *, ImGuiSettingsHandler *handler)
        {
            auto *imeUi = static_cast<ImeUI*>(handler->UserData);
            imeUi->ApplyUiSettings(GetInstance());
        }
    }
}
