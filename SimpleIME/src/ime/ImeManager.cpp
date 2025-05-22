#include "ime/ImeManager.h"

#include "FakeDirectInputDevice.h"
#include "common/log.h"
#include "hooks/ScaleformHook.h"
#include "ime/BaseImeManager.h"

#include <cstdint>
#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
auto ImeManager::Focus(HWND hwnd) -> bool
{
    auto  hwndThread      = ::GetWindowThreadProcessId(hwnd, nullptr);
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

auto ImeManager::UnlockKeyboard() const -> bool
{
    log_debug("Unlock keyboard: NONEXCLUSIVE + BACKGROUND.");
    HRESULT hr = E_FAIL;
    if (auto *keyboard = Hooks::FakeDirectInputDevice::GetInstance(); keyboard != nullptr)
    {
        hr = keyboard->TryUnlockCooperativeLevel(m_gameHwnd);
    }
    if (FAILED(hr))
    {
        log_error("Failed unlock keyboard.");
    }
    return SUCCEEDED(hr);
}

auto ImeManager::RestoreKeyboard() const -> bool
{
    log_debug("Restore keyboard: EXCLUSIVE + FOREGROUND + NOWINKEY.");
    HRESULT hr = E_FAIL;
    if (auto *keyboard = Hooks::FakeDirectInputDevice::GetInstance(); keyboard != nullptr)
    {
        hr = keyboard->TryRestoreCooperativeLevel(m_gameHwnd);
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
    if (!m_settings.enableMod)
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

auto BaseImeManager::EnableMod(bool fEnableMod) -> bool
{
    log_debug("BaseImeManager::{} {}", __func__, fEnableMod ? "enable" : "disable");
    m_fForceUpdate = true;
    return DoEnableMod(fEnableMod);
}

auto BaseImeManager::ForceFocusIme() -> bool
{
    log_debug("BaseImeManager::{}", __func__);
    if (!m_settings.enableMod)
    {
        log_warn("Mod is disabled, operate failed");
        return false;
    }
    return DoForceFocusIme();
}

auto BaseImeManager::SyncImeState() -> bool
{
    log_debug("BaseImeManager::{}", __func__);
    if (!m_settings.enableMod)
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
    if (!m_settings.enableMod)
    {
        log_warn("Mod is disabled, operate failed");
        return false;
    }
    return DoTryFocusIme();
}

auto BaseImeManager::IsShouldEnableIme() const -> bool
{
    return m_settings.keepImeOpen || Hooks::SKSE_ScaleformAllowTextInput::HasTextEntry();
}

}
}