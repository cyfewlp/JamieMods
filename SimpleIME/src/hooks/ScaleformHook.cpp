//
// Created by jamie on 2025/3/2.
//

#include "hooks/ScaleformHook.h"

#include "common/log.h"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "context.h"

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

            auto *context = Ime::Context::GetInstance();
            if (oldValue == 0 && newValue > 0)
            {
                if (context->HwndIme() != nullptr)
                {
                    if (::SendNotifyMessageW(context->HwndIme(), CM_IME_ENABLE, TRUE, 0) != TRUE)
                    {
                        log_error("Send notify message fail {}", GetLastError());
                    }
                }
                else
                {
                    log_warn("ImeWnd is not Start yet!");
                }
            }
            else if (oldValue > 0 && newValue == 0)
            {
                if (context->HwndIme() != nullptr)
                {
                    if (::SendNotifyMessageW(context->HwndIme(), CM_IME_ENABLE, FALSE, 0) != TRUE)
                    {
                        log_error("Send notify message fail {}", GetLastError());
                    }
                }
                else
                {
                    log_warn("ImeWnd is not Start yet!");
                }
            }
            g_textEntryCount = newValue;
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

            log_trace("GfxMovieInstallHook: {}",
                      pMovieView->GetMovieDef() ? pMovieView->GetMovieDef()->GetFileURL() : "");

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

        void ScaleformHooks::InstallHooks()
        {
            log_debug("Install GfxMovieInstallHook...");
            g_SetScaleModeTypeHook = std::make_unique<Scaleform_SetScaleModeTypeHookData>(SetScaleModeTypeHook);
        }

        void ScaleformHooks::UninstallHooks()
        {
            g_SetScaleModeTypeHook = nullptr;
        }

    }
}