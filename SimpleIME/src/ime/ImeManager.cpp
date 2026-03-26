#include "ime/ImeManager.h"

#include "FakeDirectInputDevice.h"
#include "ImeWnd.hpp"
#include "RE/ControlMap.h"
#include "imguiex/ErrorNotifier.h"
#include "log.h"

#include <processthreadsapi.h>
#include <windows.h>

namespace Ime
{

auto ImeManager::Focus(const HWND hwnd) -> bool
{
    auto        hwndThread      = GetWindowThreadProcessId(hwnd, nullptr);
    const DWORD currentThreadId = GetCurrentThreadId();
    bool        success         = true;
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
    logger::debug("ImeManager::{} {}", __func__, enable ? "enable" : "disable");

    // Do nothing if caller want to disable IME.
    if (enable)
    {
        if (const auto result = TryFocusIme(); IImeModule::IsFailed(result))
        {
            ErrorNotifier::GetInstance().Error("Can't focus to IME thread. IME can't work!");
            return result;
        }
    }

    auto &state = State::GetInstance();
    if (m_isForceUpdate || (state.Has(State::IME_DISABLED) && enable) || (state.NotHas(State::IME_DISABLED) && !enable))
    {
        m_isForceUpdate = false;

        bool success = false;
        if (enable)
        {
            logger::debug("Clear IME_DISABLED and set TSF focus");
            state.Clear(State::IME_DISABLED);
            success = m_imeWnd->FocusTextService(true);
        }
        else
        {
            logger::debug("Set IME_DISABLED and clear TSF focus");
            state.Set(State::IME_DISABLED);
            success = m_imeWnd->FocusTextService(false);
        }

        if (!success)
        {
            state.Set(State::IME_DISABLED);
            logger::error("Enable IME failed! last error {}", GetLastError());
        }

        return ToResult(success);
    }
    return Result::SUCCESS;
}

auto ImeManager::ForceFocusIme() -> Result
{
    logger::debug("ImeManager::{}", __func__);
    m_imeWnd->Focus();
    return Result::SUCCESS;
}

auto ImeManager::SyncImeState() -> Result
{
    logger::debug("ImeManager::{}", __func__);
    m_isForceUpdate = true;
    return EnableIme(IsShouldEnableIme());
}

auto ImeManager::TryFocusIme() -> Result
{
    logger::debug("ImeManager::{}", __func__);

    return ToResult(Focus(m_imeWnd->GetHWND()));
}

auto ImeManager::IsShouldEnableIme() const -> bool
{
    return m_settings.input.keepImeOpen || ControlMap::GetSingleton()->HasTextEntry();
}

} // namespace Ime
