#include "ImeApp.h"
#include "ImeWnd.hpp"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"
#include "ime/ImeManager.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        auto TemporaryFocusImeManager::EnableIme(bool enable) -> bool
        {
            if (!State::GetInstance().IsModEnabled())
            {
                return true;
            }
            log_debug("Try {} IME", enable ? "enable" : "disable");
            m_fIsInEnableIme = true;
            bool  success    = true;
            auto &state      = State::GetInstance();
            if (Context::GetInstance()->KeepImeOpen() || enable)
            {
                log_debug("Clear IME_DISABLED and set TSF focus");
                if (state.Has(State::IME_DISABLED))
                {
                    state.Clear(State::IME_DISABLED);
                    success = m_ImeWnd->Focus();
                    success = success && UnlockKeyboard();
                    success = success && m_ImeWnd->SetTsfFocus(true);
                }
            }
            else
            {
                log_debug("Set IME_DISABLED and clear TSF focus");
                if (state.NotHas(State::IME_DISABLED))
                {
                    state.Set(State::IME_DISABLED);
                    success = Focus(m_hwndGame);
                    success = success && RestoreKeyboard();
                    // success = success && m_ImeWnd->SetTsfFocus(false);
                }
            }

            if (!success)
            {
                state.Set(State::IME_DISABLED);
                log_error("Enable IME failed! last error {}", GetLastError());
            }

            m_fIsInEnableIme = false;
            return success;
        }

        auto TemporaryFocusImeManager::NotifyEnableIme(bool enable) const -> bool
        {
            return ::SendNotifyMessageA(m_hwndGame, CM_IME_ENABLE, enable ? TRUE : FALSE, 0) != FALSE;
        }

        auto TemporaryFocusImeManager::EnableMod(bool fEnableMod) -> bool
        {
            if (State::GetInstance().IsModEnabled() == fEnableMod)
            {
                return true;
            }
            log_debug("{} mod", fEnableMod ? "enable" : "disable");
            if (!fEnableMod)
            {
                Context::GetInstance()->SetKeepImeOpen(false);
            }
            State::GetInstance().SetEnableMod(fEnableMod);

            bool success = true;
            if (!fEnableMod)
            {
                success = EnableIme(false);
            }
            else if (Hooks::ScaleformAllowTextInput::HasTextEntry())
            {
                success = EnableIme(true);
            }

            if (!success)
            {
                State::GetInstance().SetEnableMod(false);
                log_error("Can't enable/disable mod. last error {}", GetLastError());
            }
            return success;
        }

        auto TemporaryFocusImeManager::NotifyEnableMod(bool enable) const -> bool
        {
            return ::SendNotifyMessageA(m_hwndGame, CM_MOD_ENABLE, enable ? TRUE : FALSE, 0) != FALSE;
        }

        auto TemporaryFocusImeManager::GiveUpFocus() const -> bool
        {
            return false;
        }

        auto TemporaryFocusImeManager::ForceFocusIme() -> bool
        {
            return m_ImeWnd->Focus();
        }

        auto TemporaryFocusImeManager::TryFocusIme() -> bool
        {
            if (State::GetInstance().IsModEnabled() && !m_fIsInEnableIme)
            {
                m_ImeWnd->Focus();
                EnableIme(Hooks::ScaleformAllowTextInput::HasTextEntry());
            }
            else
            {
                log_warn("Can't focus IME because mod is disabled");
            }
            return false;
        }

        // call on Render thread
        auto TemporaryFocusImeManager::SyncImeState() const -> bool
        {
            auto enableIme = Hooks::ScaleformAllowTextInput::HasTextEntry();
            if (enableIme)
            {
                State::GetInstance().Set(State::IME_DISABLED);
            }
            else
            {
                State::GetInstance().Clear(State::IME_DISABLED);
            }
            return NotifyEnableIme(enableIme);
        }
    }
}