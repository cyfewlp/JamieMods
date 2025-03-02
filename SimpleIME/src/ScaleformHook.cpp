//
// Created by jamie on 2025/3/2.
//

#include "ScaleformHook.h"

#include "common/log.h"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "context.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {

        void ScaleformAllowTextInput::AllowTextInput(bool allow)
        {
            auto context = Ime::Context::GetInstance();
            if (allow)
            {
                if (g_textEntryCount != 0xFF && g_textEntryCount++ == 0)
                {
                    if (context->HwndIme() != nullptr)
                    {
                        ::SendMessageW(context->HwndIme(), CM_IME_ENABLE, TRUE, 0);
                    }
                    else
                    {
                        log_warn("ImeWnd is not Start yet!");
                    }
                }
            }
            else
            {
                if (g_textEntryCount != 0 && --g_textEntryCount == 0)
                {
                    if (context->HwndIme() != nullptr)
                    {
                        ::SendMessageW(context->HwndIme(), CM_IME_ENABLE, FALSE, 0);
                    }
                    else
                    {
                        log_warn("ImeWnd is not Start yet!");
                    }
                }
            }

            log_trace("Text entry count: {}", g_textEntryCount);
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

        void ScaleformHooks::GfxMovieInstallHook(RE::GFxMovieView               *pMovieView,
                                                 RE::GFxMovieView::ScaleModeType scaleMode)
        {
            if (pMovieView == nullptr)
            {
                return;
            }

            log_trace("GfxMovieInstallHook: {}",
                      pMovieView->GetMovieDef() ? pMovieView->GetMovieDef()->GetFileURL() : "");

            LoadMovieHook->Original(pMovieView, scaleMode);

            RE::GFxValue skse;
            if (!pMovieView->GetVariable(&skse, "_global.skse") || !skse.IsObject())
            {
                log_error("Can't get _global.skse");
                return;
            }
            RE::GFxValue fn_AllowTextInput;
            static auto  AllowTextInput = new ScaleformAllowTextInput();
            pMovieView->CreateFunction(&fn_AllowTextInput, AllowTextInput);
            skse.SetMember("AllowTextInput", fn_AllowTextInput);
        }

        void ScaleformHooks::InstallHooks()
        {
            log_debug("Install GfxMovieInstallHook...");
            LoadMovieHook.reset(new GfxMovieInstallHookData(GfxMovieInstallHook));
        }

    }
}