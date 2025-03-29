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
    }
}