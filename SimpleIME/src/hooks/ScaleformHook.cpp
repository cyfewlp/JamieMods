//
// Created by jamie on 2025/3/2.
//

#include "hooks/ScaleformHook.h"
#include "ime/ImeManagerComposer.h"
#include "common/log.h"

#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        auto ControlMap::SKSE_AllowTextInput(bool allow) -> uint8_t
        {
            if (allow)
            {
                if (allowTextInput == MAX_TEXT_ENTRY_COUNT)
                {
                    log_warn("InputManager::AllowTextInput: counter overflow");
                }
                else
                {
                    allowTextInput++;
                }
            }
            else
            {
                if (allowTextInput == 0)
                {
                    log_warn("InputManager::AllowTextInput: counter underflow");
                }
                else
                {
                    allowTextInput--;
                }
            }

            if (auto *consoleLog = RE::ConsoleLog::GetSingleton(); //
                consoleLog != nullptr && RE::ConsoleLog::IsConsoleMode())
            {
                consoleLog->Print("%s text input, count = %d", allow ? "allowed" : "disallowed", allowTextInput);
            }

            return allowTextInput;
        }

        ControlMap *ControlMap::GetSingleton()
        {
            const REL::Relocation<ControlMap **> singleton{RE::Offset::ControlMap::Singleton};
            return *singleton;
        }

        auto ScaleformAllowTextInput::AllowTextInput(bool allow) -> std::uint8_t
        {
            ControlMap::GetSingleton()->SKSE_AllowTextInput(allow);
            OnTextEntryCountChanged();
            log_trace("Text entry count: {}", g_textEntryCount);
            return g_textEntryCount;
        }

        void ScaleformAllowTextInput::OnTextEntryCountChanged()
        {
            auto newValue = ControlMap::GetSingleton()->allowTextInput;
            auto oldValue = g_textEntryCount;
            if (newValue == oldValue)
            {
                return;
            }

            g_textEntryCount = newValue;
            if (oldValue == 0 && newValue > 0)
            {
                if (!Ime::ImeManagerComposer::GetInstance()->NotifyEnableIme(true))
                {
                    log_error("Send notify message fail {}", GetLastError());
                }
            }
            else if (oldValue > 0 && newValue == 0)
            {
                if (!Ime::ImeManagerComposer::GetInstance()->NotifyEnableIme(false))
                {
                    log_error("Send notify message fail {}", GetLastError());
                }
            }
        }

        void ScaleformAllowTextInput::Call(Params &params)
        {
            if (params.argCount == 0)
            {
                log_error("AllowInput called with illegal args");
                return;
            }

            bool enable = params.args[0].GetBool();
            AllowTextInput(enable);
        }

        void ScaleformHooks::SetScaleModeTypeHook(RE::GFxMovieView               *pMovieView,
                                                  RE::GFxMovieView::ScaleModeType scaleMode)
        {
            g_SetScaleModeTypeHook->Original(pMovieView, scaleMode);

            if (pMovieView == nullptr)
            {
                return;
            }
            if (auto *movieView = pMovieView->GetMovieDef(); movieView != nullptr)
            {
                log_trace("GfxMovieInstallHook: {}", movieView->GetFileURL());
            }

            RE::GFxValue skse;
            if (!pMovieView->GetVariable(&skse, "_global.skse") || !skse.IsObject())
            {
                log_error("Can't get _global.skse");
                return;
            }
            RE::GFxValue fn_AllowTextInput;
            static auto *AllowTextInput = new ScaleformAllowTextInput();
            pMovieView->CreateFunction(&fn_AllowTextInput, AllowTextInput);
            skse.SetMember("AllowTextInput", fn_AllowTextInput);
        }

        bool ScaleformHooks::LoadMovieHook(RE::BSScaleformManager *self, RE::IMenu *menu,
                                           RE::GPtr<RE::GFxMovieView> &viewOut, const char *fileName,
                                           RE::BSScaleformManager::ScaleModeType mode, float backgroundAlpha)
        {
            bool result = g_LoadMovieHook->Original(self, menu, viewOut, fileName, mode, backgroundAlpha);
            /*if (menu != nullptr)
            {
                auto &callbacksMap = menu->fxDelegate->callbacks;

                if (auto *registeredCallback = callbacksMap.Get("addHealth");
                    registeredCallback != nullptr && registeredCallback->callback != nullptr)
                {
                    AddHealth                    = registeredCallback->callback;
                    registeredCallback->callback = MyAddHealth;
                }
            }*/
            return result;
        }

        void ScaleformHooks::InstallHooks()
        {
            log_debug("Install GfxMovieInstallHook...");
            if (g_SetScaleModeTypeHook == nullptr)
            {
                g_SetScaleModeTypeHook = std::make_unique<Scaleform_SetScaleModeTypeHookData>(SetScaleModeTypeHook);
            }
            /*if (g_LoadMovieHook == nullptr)
            {
                g_LoadMovieHook = std::make_unique<Scaleform_LoadMovieHook>(LoadMovieHook);
            }*/
        }

        void ScaleformHooks::UninstallHooks()
        {
            g_SetScaleModeTypeHook = nullptr;
            g_LoadMovieHook        = nullptr;
        }

    }
}