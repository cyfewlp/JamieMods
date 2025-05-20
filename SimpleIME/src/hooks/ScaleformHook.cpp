//
// Created by jamie on 2025/3/2.
//

#include "hooks/ScaleformHook.h"

#include "common/log.h"
#include "ime/ImeManagerComposer.h"
#include "utils/FocusGFxCharacterInfo.h"

#include <memory>

// #define HOOK_LOAD_MOVIE

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

    log_debug("{} text input, count = {}", allow ? "allowed" : "disallowed", allowTextInput);
    return allowTextInput;
}

ControlMap *ControlMap::GetSingleton()
{
    const REL::Relocation<ControlMap **> singleton{RE::Offset::ControlMap::Singleton};
    return *singleton;
}

auto SKSE_ScaleformAllowTextInput::AllowTextInput(bool allow) -> std::uint8_t
{
    ControlMap::GetSingleton()->SKSE_AllowTextInput(allow);
    OnTextEntryCountChanged();
    log_trace("Text entry count: {}", g_textEntryCount);
    return g_textEntryCount;
}

void SKSE_ScaleformAllowTextInput::OnTextEntryCountChanged()
{
    uint8_t newValue = 0;
    uint8_t oldValue = g_textEntryCount;
    if (REL::Module::IsAE())
    {
        newValue = ControlMap::GetSingleton()->allowTextInput;
    }
    else if (REL::Module::IsSE())
    {
        newValue = ControlMap::GetSingleton()->textEntryCount;
    }
    if (newValue == oldValue)
    {
        return;
    }

    g_textEntryCount = newValue;
    auto *imeManager = Ime::ImeManagerComposer::GetInstance();
    if (Ime::ImeManagerComposer::GetInstance()->IsSupportOtherMod())
    {
        return;
    }
    imeManager->SyncImeStateIfDirty();
    if (oldValue == 0 && newValue > 0)
    {
        if (!imeManager->NotifyEnableIme(true))
        {
            log_error("Send notify message fail {}", GetLastError());
        }
    }
    else if (oldValue > 0 && newValue == 0)
    {
        if (!imeManager->NotifyEnableIme(false))
        {
            log_error("Send notify message fail {}", GetLastError());
        }
    }
}

void SKSE_ScaleformAllowTextInput::Call(Params &params)
{
    if (params.argCount == 0)
    {
        log_error("AllowInput called with illegal args");
        return;
    }

    const bool enable = params.args[0].GetBool();
    AllowTextInput(enable);

    UpdateFocusCharacterBound(reinterpret_cast<RE::GFxMovieView *>(params.movie), enable);
}

void UpdateFocusCharacterBound(RE::GFxMovieView *movieView, bool allow)
{
    if (!allow || movieView == nullptr)
    {
        Ime::FocusGFxCharacterInfo::GetInstance().Update(nullptr);
    }
    else
    {
        Ime::FocusGFxCharacterInfo::GetInstance().Update(movieView);
    }
}

void ScaleformHooks::SetScaleModeTypeHook(RE::GFxMovieView *pMovieView, RE::GFxMovieView::ScaleModeType scaleMode)
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
    static auto *AllowTextInput = new SKSE_ScaleformAllowTextInput();
    pMovieView->CreateFunction(&fn_AllowTextInput, AllowTextInput);
    skse.SetMember("AllowTextInput", fn_AllowTextInput);
}

#ifdef HOOK_LOAD_MOVIE
RE::FxDelegateHandler::CallbackFn *SetAllowTextInput;

void MySetAllowTextInput(const RE::FxDelegateArgs &a_params)
{
    log_debug("MySetAllowTextInput");
    SetAllowTextInput(a_params); // call sub_140CD5910 id: 68552

    UpdateFocusCharacterBound(a_params.GetMovie(), true);
}

bool ScaleformHooks::LoadMovieHook(
    RE::BSScaleformManager *self, RE::IMenu *menu, RE::GPtr<RE::GFxMovieView> &viewOut, const char *fileName,
    RE::BSScaleformManager::ScaleModeType mode, float backgroundAlpha
)
{
    bool result = g_LoadMovieHook->Original(self, menu, viewOut, fileName, mode, backgroundAlpha);
    if (menu != nullptr)
    {
        auto &callbacksMap = menu->fxDelegate->callbacks;

        if (auto *registeredCallback = callbacksMap.Get("SetAllowTextInput");
            registeredCallback != nullptr && registeredCallback->callback != nullptr)
        {
            SetAllowTextInput            = registeredCallback->callback;
            registeredCallback->callback = MySetAllowTextInput;
        }
    }
    return result;
}
#endif // HOOK_LOAD_MOVIE

auto ScaleformHooks::Scaleform_AllowTextInputHook(ControlMap *self, bool allow) -> uint8_t
{
    log_debug("Scaleform_AllowTextInputHook");
    auto result = g_AllowTextInputHook->Original(self, allow);

    Ime::FocusGFxCharacterInfo::GetInstance().UpdateByTopMenu();
    SKSE_ScaleformAllowTextInput::OnTextEntryCountChanged();
    return result;
}

void ScaleformHooks::InstallHooks()
{
    log_debug("Install GfxMovieInstallHook...");
    if (g_SetScaleModeTypeHook == nullptr)
    {
        g_SetScaleModeTypeHook = std::make_unique<Scaleform_SetScaleModeTypeHookData>(SetScaleModeTypeHook);
    }
#ifdef HOOK_LOAD_MOVIE
    if (g_LoadMovieHook == nullptr)
    {
        g_LoadMovieHook = std::make_unique<Scaleform_LoadMovieHook>(LoadMovieHook);
    }
#endif
    if (g_AllowTextInputHook == nullptr)
    {
        g_AllowTextInputHook = std::make_unique<Scaleform_AllowTextInput>(Scaleform_AllowTextInputHook);
    }
}

void ScaleformHooks::UninstallHooks()
{
    g_SetScaleModeTypeHook = nullptr;
#ifdef HOOK_LOAD_MOVIE
    g_LoadMovieHook = nullptr;
#endif
    g_AllowTextInputHook = nullptr;
}

}
}