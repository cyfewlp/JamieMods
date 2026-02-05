//
// Created by jamie on 2025/3/2.
//

#include "hooks/ScaleformHook.h"

#include "RE/ControlMap.h"
#include "common/hook.h"
#include "common/log.h"
#include "hooks/Hooks.hpp"
#include "ime/ImeController.h"
#include "utils/FocusGFxCharacterInfo.h"

#include <memory>

namespace Hooks::Scaleform
{
namespace
{
constexpr const char *SKSE_ORIGINAL_FN_AllowTextInput = "AllowTextInput";
constexpr const char *SKSE_BACKUP_FN_AllowTextInput   = "_skse_simple_bk_AllowTextInput";

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

class Scaleform_SetScaleModeTypeHookData : public HookData<void(RE::GFxMovieView *, RE::GFxMovieView::ScaleModeType)>
{
public:
    // NOLINTBEGIN(*-magic-numbers)
    explicit Scaleform_SetScaleModeTypeHookData(func_type *ptr)
        : HookData(
              REL::RelocationID(80302, 82325),        //
              REL::VariantOffset(0x1D9, 0x1DD, 0x00), //
              ptr, true
          )
    {
        logger::debug("Installed {}: {}", __func__, ToString());
    }

    // NOLINTEND(*-magic-numbers)
};

class Scaleform_AllowTextInput : public FunctionHook<uint8_t(Ime::ControlMap *, bool)>
{
public:
    // NOLINTBEGIN(*-magic-numbers)
    explicit Scaleform_AllowTextInput(func_type *ptr) : FunctionHook(REL::RelocationID(67252, 68552), ptr)
    {
        logger::debug("Installed {}: {}", __func__, ToString());
    }

    // NOLINTEND(*-magic-numbers)
};

struct Scaleform_SetScaleModeTypeHook
{
    static inline std::unique_ptr<Scaleform_SetScaleModeTypeHookData> hookData = nullptr;

    static auto SetScaleModeType(RE::GFxMovieView *pMovieView, RE::GFxMovieView::ScaleModeType scaleMode)
    {
        hookData->Original(pMovieView, scaleMode);

        if (pMovieView == nullptr)
        {
            return;
        }

        RE::GFxValue skse;
        if (!pMovieView->GetVariable(&skse, "_global.skse") || !skse.IsObject())
        {
            logger::error("Can't get _global.skse");
            return;
        }
        if (skse.HasMember(SKSE_BACKUP_FN_AllowTextInput))
        {
            return;
        }

        static auto *handler = new SKSE_AllowTextInputFnHandler;

        RE::GFxValue skse_fn_AllowTextInput;
        if (skse.GetMember(SKSE_ORIGINAL_FN_AllowTextInput, &skse_fn_AllowTextInput))
        {
            skse.SetMember(SKSE_BACKUP_FN_AllowTextInput, skse_fn_AllowTextInput);

            RE::GFxValue fn_AllowTextInput;
            pMovieView->CreateFunction(&fn_AllowTextInput, handler);
            skse.SetMember(SKSE_ORIGINAL_FN_AllowTextInput, fn_AllowTextInput);

            logger::debug(
                "Successfully hooked skse.AllowTextInput for movie: {}",
                pMovieView->GetMovieDef() != nullptr ? pMovieView->GetMovieDef()->GetFileURL() : "Unknown"
            );
        }
    }

    static void Install()
    {
        hookData = std::make_unique<Scaleform_SetScaleModeTypeHookData>(SetScaleModeType);
    }
};

struct Scaleform_AllowTextInputHook
{
    static inline std::unique_ptr<Scaleform_AllowTextInput> hookData = nullptr;

    static auto AllowTextInput(Ime::ControlMap *self, bool allow)
    {
        logger::debug("Scaleform_AllowTextInputHook");
        auto result = hookData->Original(self, allow);

        Ime::FocusGFxCharacterInfo::GetInstance().UpdateByTopMenu();
        SKSE_AllowTextInputFnHandler::OnTextEntryCountChanged(result);
        return result;
    }

    static void Install()
    {
        hookData = std::make_unique<Scaleform_AllowTextInput>(AllowTextInput);
    }
};

} // namespace

auto SKSE_AllowTextInputFnHandler::AllowTextInput(bool allow) -> std::uint8_t
{
    const std::uint8_t entryCount = Ime::ControlMap::GetSingleton()->SKSE_AllowTextInput(allow);
    OnTextEntryCountChanged(entryCount);
    logger::trace("Text entry count: {}", g_prevTextEntryCount);
    return g_prevTextEntryCount;
}

void SKSE_AllowTextInputFnHandler::OnTextEntryCountChanged(std::uint8_t entryCount)
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

void SKSE_AllowTextInputFnHandler::Call(Params &params)
{
    if (params.argCount < 1)
    {
        logger::error("AllowInput called with insufficient args");
        return;
    }
    auto      *fxMovieView = reinterpret_cast<RE::GFxMovieView *>(params.movie);
    const bool enable      = params.args[0].GetBool(); // NOLINT(*-pro-bounds-pointer-arithmetic)

    RE::GFxValue skse;
    bool         calledOriginal = false;
    if (fxMovieView->GetVariable(&skse, "_global.skse") && skse.IsObject())
    {
        RE::GFxValue backupFn;
        if (skse.GetMember(SKSE_BACKUP_FN_AllowTextInput, &backupFn))
        {
            RE::GFxValue result; // this is AS return value, meaningless.
            calledOriginal = skse.Invoke(SKSE_BACKUP_FN_AllowTextInput, &result, params.args, params.argCount);

            if (calledOriginal)
            {
                const auto entryCount = Ime::ControlMap::GetSingleton()->GetTextEntryCount();
                OnTextEntryCountChanged(entryCount);
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
    if (!calledOriginal)
    {
        AllowTextInput(enable);
    }
    UpdateFocusCharacterBound(fxMovieView, enable);
}

void Install()
{
    static bool installed = false;
    if (installed)
    {
        return;
    }
    installed = true;
    Scaleform_SetScaleModeTypeHook::Install();
    Scaleform_AllowTextInputHook::Install();
}

void Uninstall()
{
    Scaleform_AllowTextInputHook::hookData = nullptr;
}
} // namespace Hooks::Scaleform
