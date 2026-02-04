//
// Created by jamie on 2025/3/2.
//

#include "hooks/ScaleformHook.h"

#include "common/log.h"
#include "ime/ImeController.h"
#include "utils/FocusGFxCharacterInfo.h"

#include <memory>

// #define HOOK_LOAD_MOVIE

namespace Hooks
{

namespace
{
constexpr const char *SKSE_ORIGINAL_FN_AllowTextInput = "AllowTextInput";
constexpr const char *SKSE_BACKUP_FN_AllowTextInput   = "_skse_AllowTextInput";

struct ScaleformHooksState
{
    static inline bool                            skseFnAllowTextInputBackuped = false;
    static inline std::unique_ptr<ScaleformHooks> g_ScaleformHooks             = nullptr;
};

} // namespace

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

auto ControlMap::GetSingleton() -> ControlMap *
{
    static REL::Relocation<ControlMap **> const singleton{RELOCATION_ID(514705, 400863)};
    return *singleton;
}

void ControlMap::DoAllowTextInput(bool allow, std::uint8_t &entryCount)
{
    if (allow)
    {
        if (entryCount == UINT8_MAX)
        {
            logger::warn("InputManager::AllowTextInput: counter overflow");
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
            logger::warn("InputManager::AllowTextInput: counter underflow");
        }
        else
        {
            entryCount--;
        }
    }

    logger::debug("{} text input, count = {}", allow ? "allowed" : "disallowed", entryCount);
}

auto SKSE_ScaleformAllowTextInput::AllowTextInput(bool allow) -> std::uint8_t
{
    std::uint8_t entryCount = ControlMap::GetSingleton()->SKSE_AllowTextInput(allow);
    OnTextEntryCountChanged(entryCount);
    logger::trace("Text entry count: {}", g_prevTextEntryCount);
    return g_prevTextEntryCount;
}

void SKSE_ScaleformAllowTextInput::OnTextEntryCountChanged(std::uint8_t entryCount)
{
    const uint8_t oldValue = g_prevTextEntryCount;
    logger::trace("OnTextEntryCountChanged: prev {}, curr {}", oldValue, entryCount);
    if (entryCount == oldValue)
    {
        return;
    }

    g_prevTextEntryCount = entryCount;
    auto *imeManager     = Ime::ImeController::GetInstance();
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
        logger::error("AllowInput called with illegal args");
        return;
    }
    auto *fxMovieView = reinterpret_cast<RE::GFxMovieView *>(params.movie);

    RE::GFxValue skse;
    bool         called = false;
    if (fxMovieView->GetVariable(&skse, "_global.skse") && skse.IsObject())
    {
        if (ScaleformHooksState::skseFnAllowTextInputBackuped)
        {
            RE::GFxValue result;
            called = skse.Invoke(SKSE_BACKUP_FN_AllowTextInput, &result, params.args, params.argCount);

            if (called && result.IsNumber())
            {
                const auto textEntryCount = static_cast<uint8_t>(result.GetUInt());
                OnTextEntryCountChanged(textEntryCount);
            }
            logger::trace("Called backup skse fn AllowTextInput.");
        }
    }
    else
    {
        if (!skse.IsObject())
        {
            logger::warn("Already installed SKSE extension function: AllowTextInput, but _global.skse missing!");
        }
    }
    if (!called)
    {
        const bool enable         = params.args[0].GetBool();
        const auto textEntryCount = AllowTextInput(enable);
        params.retVal->SetNumber(textEntryCount);
        UpdateFocusCharacterBound(fxMovieView, enable);
    }
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
    ScaleformHooksState::g_ScaleformHooks->m_SetScaleModeTypeHook->Original(pMovieView, scaleMode);

    if (pMovieView == nullptr)
    {
        return;
    }
    if (auto *movieView = pMovieView->GetMovieDef(); movieView != nullptr)
    {
        logger::trace("GfxMovieInstallHook: {}", movieView->GetFileURL());
    }

    RE::GFxValue skse;
    if (!pMovieView->GetVariable(&skse, "_global.skse") || !skse.IsObject())
    {
        logger::error("Can't get _global.skse");
        return;
    }
    static auto *AllowTextInputFnHandler = new SKSE_ScaleformAllowTextInput; // NOLINT(*-owning-memory)

    RE::GFxValue fn_AllowTextInput;
    RE::GFxValue skse_fn_AllowTextInput;
    pMovieView->CreateFunction(&fn_AllowTextInput, AllowTextInputFnHandler);
    if (skse.GetMember(SKSE_ORIGINAL_FN_AllowTextInput, &skse_fn_AllowTextInput))
    {
        if (skse.SetMember(SKSE_BACKUP_FN_AllowTextInput, skse_fn_AllowTextInput))
        {
            ScaleformHooksState::skseFnAllowTextInputBackuped = true;
        }
    }
    skse.SetMember(SKSE_ORIGINAL_FN_AllowTextInput, fn_AllowTextInput);
}

#ifdef HOOK_LOAD_MOVIE
RE::FxDelegateHandler::CallbackFn *SetAllowTextInput;

void MySetAllowTextInput(const RE::FxDelegateArgs &a_params)
{
    logger::debug("MySetAllowTextInput");
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
    logger::debug("Scaleform_AllowTextInputHook");
    auto result = ScaleformHooksState::g_ScaleformHooks->m_AllowTextInputHook->Original(self, allow);

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
    if (ScaleformHooksState::g_ScaleformHooks)
    {
        logger::warn("Already installed ScaleformHooks.");
        return;
    }
    logger::debug("Install GfxMovieInstallHook...");
    ScaleformHooksState::skseFnAllowTextInputBackuped = false;
    ScaleformHooksState::g_ScaleformHooks             = std::make_unique<ScaleformHooks>();
}

void ScaleformHooks::Uninstall()
{
    ScaleformHooksState::g_ScaleformHooks = nullptr;
    // uninstall SKSE FN hook may unnecessary.
    // ScaleformHooksState::skseFnAllowTextInputBackuped = false;
}

} // namespace Hooks
