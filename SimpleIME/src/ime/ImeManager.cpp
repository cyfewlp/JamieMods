#include "ime/ImeManager.h"
#include "FakeDirectInputDevice.h"
#include "ImeApp.h"
#include "ImeWnd.hpp"
#include "context.h"
#include "hooks/ScaleformHook.h"

#include "common/log.h"
#include <core/State.h>
#include <dinput.h>
#include <errhandlingapi.h>
#include <memory>
#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        std::unique_ptr<ImeManagerComposer> ImeManagerComposer::g_instance = nullptr;

        //////////////////////////////////////////////////////////////////////////
        // GrabFocusImeManager
        //////////////////////////////////////////////////////////////////////////

        // must call in Ime thread
        // 1. Clear/Set IME_DISABLED,
        // 2. Set TSF focus
        auto PermanentFocusImeManager::EnableIme(bool enable) -> bool
        {
            if (!State::GetInstance().IsModEnabled())
            {
                return true;
            }
            log_debug("Try {} IME", enable ? "enable" : "disable");
            bool  success = true;
            auto &state   = State::GetInstance();
            if (Context::GetInstance()->KeepImeOpen() || enable)
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

        auto PermanentFocusImeManager::NotifyEnableIme(bool enable) const -> bool
        {
            return m_ImeWnd->SendNotifyMessageToIme(CM_IME_ENABLE, enable ? TRUE : FALSE, 0) != FALSE;
        }

        auto PermanentFocusImeManager::WaitEnableIme(bool enable) const -> bool
        {
            return m_ImeWnd->SendMessageToIme(CM_IME_ENABLE, enable ? TRUE : FALSE, 0) != FALSE;
        }

        // must call on IME thread(by send message)
        // 0. Unset keepImeOpen if will disable mod
        // 1. Enable/Disable IME if any text entry changed
        // 2. Unlock/lock keyboard
        // 3. Focus to IME/Game
        auto PermanentFocusImeManager::EnableMod(bool fEnableMod) -> bool
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
            else
            {
                if (Context::GetInstance()->KeepImeOpen() || Hooks::ScaleformAllowTextInput::HasTextEntry())
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
                State::GetInstance().SetEnableMod(false);
                log_error("Can't enable/disable mod. last error {}", GetLastError());
            }
            return success;
        }

        auto PermanentFocusImeManager::NotifyEnableMod(bool enable) const -> bool
        {
            return m_ImeWnd->SendNotifyMessageToIme(CM_MOD_ENABLE, enable ? TRUE : FALSE, 0) != FALSE;
        }

        auto PermanentFocusImeManager::WaitEnableMod(bool enable) const -> bool
        {
            return m_ImeWnd->SendMessageToIme(CM_MOD_ENABLE, enable ? TRUE : FALSE, 0) != FALSE;
        }

        auto PermanentFocusImeManager::GiveUpFocus() const -> bool
        {
            return FocusImeOrGame(false);
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
                success = Focus(ImeApp::GetInstance().GetGameHWND());
            }

            if (!success)
            {
                log_error("Failed focus to {}", focusIme ? "IME" : "Game");
            }
            return success;
        }

        // call on Render thread
        auto PermanentFocusImeManager::ForceFocusIme() -> bool
        {
            return m_ImeWnd->Focus();
        }

        // call on main thread
        auto PermanentFocusImeManager::TryFocusIme() -> bool
        {
            bool success = false;
            if (State::GetInstance().IsModEnabled())
            {
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
                success = success && EnableIme(Hooks::ScaleformAllowTextInput::HasTextEntry());
                if (!success)
                {
                    log_error("Failed to focus IME: {}", ::GetLastError());
                }
            }
            else
            {
                log_warn("Can't focus IME because mod is disabled");
            }
            return success;
        }

        // call on Render thread
        auto PermanentFocusImeManager::SyncImeState() const -> bool
        {
            State::GetInstance().SetEnableMod(false);
            return NotifyEnableMod(true);
        }

        auto ImeManager::Focus(HWND hwnd) -> bool
        {
            auto  hwndThread      = ::GetWindowThreadProcessId(hwnd, 0);
            DWORD currentThreadId = ::GetCurrentThreadId();
            bool  success         = true;
            if (hwndThread != currentThreadId)
            {
                success = ::AttachThreadInput(hwndThread, currentThreadId, TRUE) != FALSE;
                if (success)
                {
                    ::SetFocus(hwnd);
                    ::AttachThreadInput(hwndThread, currentThreadId, FALSE);
                }
            }
            else
            {
                ::SetFocus(hwnd);
            }
            return success;
        }

        auto ImeManager::UnlockKeyboard() -> bool
        {
            log_debug("Unlock keyboard: NONEXCLUSIVE + BACKGROUND.");
            HRESULT hr = E_FAIL;
            if (auto *keyboard = Hooks::FakeDirectInputDevice::GetInstance(); keyboard != nullptr)
            {
                hr = keyboard->TryUnlockCooperativeLevel(ImeApp::GetInstance().GetGameHWND());
            }
            if (FAILED(hr))
            {
                log_error("Failed unlock keyboard.");
            }
            return SUCCEEDED(hr);
        }

        auto ImeManager::RestoreKeyboard() -> bool
        {
            log_debug("Restore keyboard: EXCLUSIVE + FOREGROUND + NOWINKEY.");
            HRESULT hr = E_FAIL;
            if (auto *keyboard = Hooks::FakeDirectInputDevice::GetInstance(); keyboard != nullptr)
            {
                hr = keyboard->TryRestoreCooperativeLevel(ImeApp::GetInstance().GetGameHWND());
            }
            if (FAILED(hr))
            {
                log_error("Failed lock keyboard.");
            }
            return SUCCEEDED(hr);
        }
    }
}