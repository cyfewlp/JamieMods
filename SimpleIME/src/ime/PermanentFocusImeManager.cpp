#include "ime/PermanentFocusImeManager.h"
#include "ImeWnd.hpp"
#include "common/log.h"
#include "configs/CustomMessage.h"
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
            bool  success = true;
            auto &state   = State::GetInstance();
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

        auto PermanentFocusImeManager::DoNotifyEnableIme(bool enable) const -> bool
        {
            return m_ImeWnd->SendNotifyMessageToIme(CM_IME_ENABLE, enable ? TRUE : FALSE, 0);
        }

        auto PermanentFocusImeManager::DoWaitEnableIme(bool enable) const -> bool
        {
            return m_ImeWnd->SendMessageToIme(CM_IME_ENABLE, enable ? TRUE : FALSE, 0);
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
                if (Hooks::SKSE_ScaleformAllowTextInput::HasTextEntry())
                {
                    success = EnableIme(true);
                }
                else
                {
                    success = EnableIme(false);
                }
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

        auto PermanentFocusImeManager::DoNotifyEnableMod(bool enable) const -> bool
        {
            return m_ImeWnd->SendNotifyMessageToIme(CM_MOD_ENABLE, enable ? TRUE : FALSE, 0);
        }

        auto PermanentFocusImeManager::DoWaitEnableMod(bool enable) const -> bool
        {
            return m_ImeWnd->SendMessageToIme(CM_MOD_ENABLE, enable ? TRUE : FALSE, 0);
        }

        // Call on IME thread
        auto PermanentFocusImeManager::FocusImeOrGame(bool focusIme) const -> bool
        {
            auto success = false;
            if (focusIme)
            {
                success = m_ImeWnd->Focus();
            }
            else
            {
                success = Focus(m_hwndGame);
            }

            if (!success)
            {
                log_error("Failed focus to {}", focusIme ? "IME" : "Game");
            }
            return success;
        }

        // call on Render thread
        auto PermanentFocusImeManager::DoForceFocusIme() -> bool
        {
            return m_ImeWnd->Focus();
        }

        // call on Render thread
        auto PermanentFocusImeManager::DoSyncImeState() const -> bool
        {
            return NotifyEnableMod(true);
        }

        // call on main thread
        auto PermanentFocusImeManager::DoTryFocusIme() -> bool
        {
            bool success = false;
            log_debug("focus to IME wnd");
            DWORD imeThreadId     = m_ImeWnd->GetImeThreadId();
            DWORD currentThreadId = ::GetCurrentThreadId();
            success               = imeThreadId != 0;
            if (success)
            {
                if (imeThreadId != currentThreadId)
                {
                    success = ::AttachThreadInput(imeThreadId, currentThreadId, TRUE) != FALSE;
                    success = success && m_ImeWnd->Focus();
                    ::AttachThreadInput(imeThreadId, currentThreadId, FALSE);
                }
                else
                {
                    success = m_ImeWnd->Focus();
                }
            }
            success = success && EnableIme(Hooks::SKSE_ScaleformAllowTextInput::HasTextEntry());
            if (!success)
            {
                log_error("Failed to focus IME: {}", ::GetLastError());
            }
            return success;
        }
    }
}