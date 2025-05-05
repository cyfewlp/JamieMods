//
// Created by jamie on 2025/5/5.
//

#ifndef UISETTINGS_H
#define UISETTINGS_H

#include "common/config.h"
#include "ime/ImeManagerComposer.h"

struct ImGuiTextBuffer;
struct ImGuiContext;
struct ImGuiSettingsHandler;

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImeUI;

        class UiSettings
        {
            static constexpr std::string_view SETTING_NAME = "UiSettings";
            using WindowPosPolicy                          = ImeManagerComposer::ImeWindowPosUpdatePolicy;

            bool            showSettings = true;
            bool            enableMod    = true;
            std::string     usedLanguage;
            FocusType       focusType             = Permanent;
            WindowPosPolicy windowPosUpdatePolicy = WindowPosPolicy::NONE;
            bool            enableUnicodePaste    = false;
            bool            keepImeOpen           = false;
            int             usedThemeIndex        = -1;

        public:
            static void RegisterImGuiIniHandler(ImeUI *imeUi);

            static auto GetInstance() -> UiSettings *
            {
                static UiSettings instance;
                return &instance;
            }

            [[nodiscard]] auto IsShowSettings() const -> bool
            {
                return showSettings;
            }

            [[nodiscard]] auto IsEnableMod() const -> bool
            {
                return enableMod;
            }

            [[nodiscard]] auto GetUsedLanguage() const -> const std::string &
            {
                return usedLanguage;
            }

            [[nodiscard]] auto GetFocusType() const -> FocusType
            {
                return focusType;
            }

            [[nodiscard]] auto GetWindowPosUpdatePolicy() const -> WindowPosPolicy
            {
                return windowPosUpdatePolicy;
            }

            [[nodiscard]] auto IsEnableUnicodePaste() const -> bool
            {
                return enableUnicodePaste;
            }

            [[nodiscard]] auto IsKeepImeOpen() const -> bool
            {
                return keepImeOpen;
            }

            [[nodiscard]] auto GetUsedThemeIndex() const -> int
            {
                return usedThemeIndex;
            }

            void Reset()
            {
                showSettings          = true;
                enableMod             = true;
                usedLanguage          = "";
                focusType             = Permanent;
                windowPosUpdatePolicy = WindowPosPolicy::NONE;
                enableUnicodePaste    = false;
                keepImeOpen           = false;
                usedThemeIndex        = -1;
            }

        private:
            static void *ReadOpenFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, const char *name);
            static void  ReadLineFn(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry, const char *line);
            static void  WriteAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf);
            static void  ClearAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler);
            static void  ApplyAll(ImGuiContext *ctx, ImGuiSettingsHandler *handler);
        };
    }
}

#endif // UISETTINGS_H
