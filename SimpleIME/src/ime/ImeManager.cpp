#include "ime/ImeManager.h"

#include "FakeDirectInputDevice.h"
#include "ImeWnd.hpp"
#include "common/log.h"
#include "hooks/ScaleformHook.h"

#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{

ImeManager::~ImeManager() {}

auto ImeManager::Focus(HWND hwnd) -> bool
{
    auto  hwndThread      = GetWindowThreadProcessId(hwnd, nullptr);
    DWORD currentThreadId = GetCurrentThreadId();
    bool  success         = true;
    if (hwndThread != currentThreadId)
    {
        success = AttachThreadInput(hwndThread, currentThreadId, TRUE) != FALSE;
        if (success)
        {
            SetFocus(hwnd);
            AttachThreadInput(hwndThread, currentThreadId, FALSE);
        }
    }
    else
    {
        SetFocus(hwnd);
    }
    return success;
}

auto ImeManager::EnableIme(bool enable) -> Result
{
    log_debug("ImeManager::{} {}", __func__, enable ? "enable" : "disable");

    if ((State::GetInstance().Has(State::IME_DISABLED) && enable) ||
        (State::GetInstance().NotHas(State::IME_DISABLED) && !enable) || m_fForceUpdate)
    {
        m_fForceUpdate = false;

        bool  success;
        auto &state = State::GetInstance();
        if (enable)
        {
            log_debug("Clear IME_DISABLED and set TSF focus");
            state.Clear(State::IME_DISABLED);
            success = m_imeWnd->SetTsfFocus(true);
        }
        else
        {
            log_debug("Set IME_DISABLED and clear TSF focus");
            state.Set(State::IME_DISABLED);
            success = m_imeWnd->SetTsfFocus(false);
        }

        if (!success)
        {
            state.Set(State::IME_DISABLED);
            log_error("Enable IME failed! last error {}", GetLastError());
        }

        return success ? Result::SUCCESS : Result::FAILED;
    }
    return Result::SUCCESS;
}

auto ImeManager::ForceFocusIme() -> Result
{
    log_debug("ImeManager::{}", __func__);
    m_imeWnd->Focus();
    return Result::SUCCESS;
}

auto ImeManager::SyncImeState() -> Result
{
    log_debug("ImeManager::{}", __func__);
    m_fForceUpdate = true;

    const auto result = EnableIme(IsShouldEnableIme());
    if (IsSuccess(result))
    {
        m_imeWnd->Focus();
    }
    return result;
}

auto ImeManager::TryFocusIme() -> Result
{
    log_debug("ImeManager::{}", __func__);

    log_debug("focus to IME wnd");
    if (Focus(m_imeWnd->GetHWND()))
    {
        const auto result = EnableIme(IsShouldEnableIme());
        if (!IsSuccess(result))
        {
            log_error("Failed to focus IME: {}", GetLastError());
        }
        return result;
    }
    return Result::FAILED;
}

auto ImeManager::IsShouldEnableIme() const -> bool
{
    return m_settings.input.keepImeOpen || Hooks::ControlMap::GetSingleton()->HasTextEntry();
}

}
}