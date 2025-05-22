#include "ime/PermanentFocusImeManager.h"

#include "ImeWnd.hpp"
#include "common/log.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"

#include <WinUser.h>
#include <processthreadsapi.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
// must call in Ime thread
// 1. Clear/Set IME_DISABLED,
// 2. Set TSF focus
auto PermanentFocusImeManager::DoEnableIme(bool enable) -> bool
{
    bool  success;
    auto &state = State::GetInstance();
    if (enable)
    {
        log_debug("Clear IME_DISABLED and set TSF focus");
        state.Clear(State::IME_DISABLED);
        success = m_ImeWnd->SetTsfFocus(true);
    }
    else
    {
        log_debug("Set IME_DISABLED and clear TSF focus");
        state.Set(State::IME_DISABLED);
        success = m_ImeWnd->SetTsfFocus(false);
    }

    if (!success)
    {
        state.Set(State::IME_DISABLED);
        log_error("Enable IME failed! last error {}", GetLastError());
    }

    return success;
}

// must call on IME thread(by send message)
// 0. Unset keepImeOpen if will disable mod
// 1. Enable/Disable IME if any text entry changed
// 2. Unlock/lock keyboard
// 3. Focus to IME/Game
auto PermanentFocusImeManager::DoEnableMod(bool fEnableMod) -> bool
{
    bool success;
    if (!fEnableMod)
    {
        success = EnableIme(false);
    }
    else
    {
        success = EnableIme(IsShouldEnableIme());
    }

    if (success)
    {
        success = fEnableMod ? UnlockKeyboard() : RestoreKeyboard();
        success = success && FocusImeOrGame(fEnableMod);
    }

    if (!success)
    {
        log_error("Can't enable/disable mod. last error {}", GetLastError());
    }
    return success;
}

// Call on IME thread
auto PermanentFocusImeManager::FocusImeOrGame(bool focusIme) const -> bool
{
    bool success = true;
    if (focusIme)
    {
        m_ImeWnd->Focus();
    }
    else
    {
        success = Focus(GetGameHwnd());
    }

    if (!success)
    {
        log_error("Failed focus to {}", focusIme ? "IME" : "Game");
    }
    return success;
}

auto PermanentFocusImeManager::DoForceFocusIme() -> bool
{
    m_ImeWnd->Focus();
    return true;
}

auto PermanentFocusImeManager::DoSyncImeState() -> bool
{
    bool success = EnableIme(IsShouldEnableIme());
    success      = success && UnlockKeyboard();
    if (success)
    {
        m_ImeWnd->Focus();
    }
    return success;
}

auto PermanentFocusImeManager::DoTryFocusIme() -> bool
{
    log_debug("focus to IME wnd");
    bool success = Focus(m_ImeWnd->GetHWND());
    success      = success && EnableIme(IsShouldEnableIme());
    if (!success)
    {
        log_error("Failed to focus IME: {}", ::GetLastError());
    }
    return success;
}
}
}