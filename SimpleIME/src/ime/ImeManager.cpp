#include "ime/ImeManager.h"
#include "FakeDirectInputDevice.h"
#include "ImeApp.h"

#include "common/log.h"
#include <cstdint>
#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
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

        auto BaseImeManager::EnableIme(bool enable) -> bool
        {
            log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
            if (m_fForceUpdate)
            {
                auto result    = DoEnableIme(enable);
                m_fForceUpdate = false;
                return result;
            }
            if (!IsModEnabled())
            {
                log_warn("Mod is disabled, operate failed");
                return false;
            }
            if ((State::GetInstance().Has(State::IME_DISABLED) && enable) ||
                (State::GetInstance().NotHas(State::IME_DISABLED) && !enable))
            {
                return DoEnableIme(enable);
            }
            return true;
        }

        auto BaseImeManager::NotifyEnableIme(bool enable) const -> bool
        {
            log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
            if (!IsModEnabled())
            {
                log_warn("Mod is disabled, operate failed");
                return false;
            }
            return DoNotifyEnableIme(enable);
        }

        auto BaseImeManager::WaitEnableIme(bool enable) const -> bool
        {
            log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
            if (!IsModEnabled())
            {
                log_warn("Mod is disabled, operate failed");
                return false;
            }
            return DoWaitEnableIme(enable);
        }

        auto BaseImeManager::EnableMod(bool fEnableMod) -> bool
        {
            log_debug("BaseImeManager::{} {}", __func__, fEnableMod ? "enable" : "disable");
            m_fForceUpdate = true;
            return DoEnableMod(fEnableMod);
        }

        auto BaseImeManager::ForceFocusIme() -> bool
        {
            log_debug("BaseImeManager::{}", __func__);
            if (!IsModEnabled())
            {
                log_warn("Mod is disabled, operate failed");
                return false;
            }
            return DoForceFocusIme();
        }

        auto BaseImeManager::SyncImeState() -> bool
        {
            log_debug("BaseImeManager::{}", __func__);
            if (!IsModEnabled())
            {
                log_warn("Mod is disabled, operate failed");
                return false;
            }
            m_fForceUpdate = true;
            return DoSyncImeState();
        }

        auto BaseImeManager::TryFocusIme() -> bool
        {
            log_debug("BaseImeManager::{}", __func__);
            if (!IsModEnabled())
            {
                log_warn("Mod is disabled, operate failed");
                return false;
            }
            return DoTryFocusIme();
        }

    }
}