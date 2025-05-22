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
    if (REL::Module::IsSE())
    {
        DoAllowTextInput(allow, textEntryCount);
        return textEntryCount;
    }
    DoAllowTextInput(allow, allowTextInput);
    return allowTextInput;
}

auto ControlMap::GetTextEntryCount() const -> uint8_t
{
    if (REL::Module::IsSE())
    {
        return textEntryCount;
    }
    return allowTextInput;
}

ControlMap *ControlMap::GetSingleton()
{
    const REL::Relocation<ControlMap **> singleton{RE::Offset::ControlMap::Singleton};
    return *singleton;
}

void ControlMap::DoAllowTextInput(bool allow, std::uint8_t &entryCount)
{
    if (allow)
    {
        if (entryCount == UINT8_MAX)
        {
            log_warn("InputManager::AllowTextInput: counter overflow");
        }
        else
        {
            entryCount++;
        }
    }
    else
    {
        if (entryCount == 0)
        {
            log_warn("InputManager::AllowTextInput: counter underflow");
        }
        else
        {
            entryCount--;
        }
    }

    log_debug("{} text input, count = {}", allow ? "allowed" : "disallowed", entryCount);
}

auto SKSE_ScaleformAllowTextInput::AllowTextInput(bool allow) -> std::uint8_t
{
    std::uint8_t entryCount = ControlMap::GetSingleton()->SKSE_AllowTextInput(allow);
    OnTextEntryCountChanged(entryCount);
    log_trace("Text entry count: {}", g_prevTextEntryCount);
    return g_prevTextEntryCount;
}

void SKSE_ScaleformAllowTextInput::OnTextEntryCountChanged(std::uint8_t entryCount)
{
    const uint8_t oldValue = g_prevTextEntryCount;
    if (entryCount == oldValue)
    {
        return;
    }

    g_prevTextEntryCount = entryCount;
    auto *imeManager     = Ime::ImeManagerComposer::GetInstance();
    imeManager->SyncImeStateIfDirty();
    if (oldValue == 0 && entryCount > 0)
    {
        imeManager->EnableIme(true);
    }
    else if (oldValue > 0 && entryCount == 0)
    {
        imeManager->EnableIme(false);
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

static std::unique_ptr<ScaleformHooks> g_ScaleformHooks;

void ScaleformHooks::SetScaleModeTypeHook(RE::GFxMovieView *pMovieView, RE::GFxMovieView::ScaleModeType scaleMode)
{
    g_ScaleformHooks->m_SetScaleModeTypeHook->Original(pMovieView, scaleMode);

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
    auto result = g_ScaleformHooks->m_AllowTextInputHook->Original(self, allow);

    Ime::FocusGFxCharacterInfo::GetInstance().UpdateByTopMenu();
    SKSE_ScaleformAllowTextInput::OnTextEntryCountChanged(result);
    return result;
}

ScaleformHooks::ScaleformHooks()
{
    m_SetScaleModeTypeHook = std::make_unique<Scaleform_SetScaleModeTypeHookData>(SetScaleModeTypeHook);
    m_AllowTextInputHook   = std::make_unique<Scaleform_AllowTextInput>(Scaleform_AllowTextInputHook);
#ifdef HOOK_LOAD_MOVIE
    m_LoadMovieHook = std::make_unique<Scaleform_LoadMovieHook>(LoadMovieHook);
#endif
}

void ScaleformHooks::Install()
{
    if (!g_ScaleformHooks)
    {
        log_warn("Already installed ScaleformHooks.");
    }
    log_debug("Install GfxMovieInstallHook...");
    g_ScaleformHooks.reset(new ScaleformHooks);
}

void ScaleformHooks::Uninstall()
{
    g_ScaleformHooks = nullptr;
}

}
}