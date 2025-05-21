#include "ime/TemporaryFocusImeManager.h"

#include "ImeWnd.hpp"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
auto TemporaryFocusImeManager::DoEnableIme(bool enable) -> bool
{
    bool  success = true;
    auto &state   = State::GetInstance();
    if (enable)
    {
        log_debug("Clear IME_DISABLED and set TSF focus");
        state.Clear(State::IME_DISABLED);
        success = m_ImeWnd->Focus();
        success = success && UnlockKeyboard();
        success = success && m_ImeWnd->SetTsfFocus(true);
    }
    else
    {
        log_debug("Set IME_DISABLED and clear TSF focus");
        state.Set(State::IME_DISABLED);
        success = Focus(m_hwndGame);
        success = success && RestoreKeyboard();
        // success = success && m_ImeWnd->SetTsfFocus(false);
    }

    if (!success)
    {
        state.Set(State::IME_DISABLED);
        log_error("Enable IME failed! last error {}", GetLastError());
    }

    return success;
}

// On main thread
auto TemporaryFocusImeManager::DoEnableMod(bool fEnableMod) -> bool
{
    bool success = true;
    if (!fEnableMod)
    {
        success = EnableIme(false);
    }
    else if (Hooks::SKSE_ScaleformAllowTextInput::HasTextEntry())
    {
        success = EnableIme(true);
    }

    if (!success)
    {
        log_error("Can't enable/disable mod. last error {}", GetLastError());
    }
    return success;
}

auto TemporaryFocusImeManager::DoForceFocusIme() -> bool
{
    return m_ImeWnd->Focus();
}

// call on render thread
auto TemporaryFocusImeManager::DoSyncImeState() -> bool
{
    const auto enableIme = Hooks::SKSE_ScaleformAllowTextInput::HasTextEntry();
    return EnableIme(enableIme);
}

// On main thread
auto TemporaryFocusImeManager::DoTryFocusIme() -> bool
{
    m_fIsInEnableIme = true;
    bool success     = true;
    if (!m_fIsInEnableIme) // FIXME
    {
        success          = EnableIme(Hooks::SKSE_ScaleformAllowTextInput::HasTextEntry());
        m_fIsInEnableIme = false;
    }
    return success;
}
}
}