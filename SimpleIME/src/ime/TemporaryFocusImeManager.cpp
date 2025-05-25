#include "ime/TemporaryFocusImeManager.h"

#include "ImeWnd.hpp"
#include "common/log.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
auto TemporaryFocusImeManager::DoEnableIme(bool enable) -> bool
{
    bool  success;
    auto &state = State::GetInstance();
    if (enable)
    {
        log_debug("Clear IME_DISABLED and set TSF focus");
        state.Clear(State::IME_DISABLED);
        success = UnlockKeyboard();
        if (success)
        {
            m_ImeWnd->Focus();
            success = success && m_ImeWnd->SetTsfFocus(true);
            // we may unnecessary restore focus when call the `SetTsfFocus` fail;
        }
    }
    else
    {
        log_debug("Set IME_DISABLED and clear TSF focus");
        state.Set(State::IME_DISABLED);
        success = Focus(GetGameHwnd());
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
    else if (IsShouldEnableIme())
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
    m_ImeWnd->Focus();
    return true;
}

// call on render thread
auto TemporaryFocusImeManager::DoSyncImeState() -> bool
{
    return EnableIme(IsShouldEnableIme());
}

// On main thread
auto TemporaryFocusImeManager::DoTryFocusIme() -> bool
{
    bool success = true;
    if (!m_fIsInEnableIme)
    {
        m_fIsInEnableIme = true;
        success          = EnableIme(IsShouldEnableIme());
        m_fIsInEnableIme = false;
    }
    return success;
}
}
}