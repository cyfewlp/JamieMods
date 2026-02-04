//
// Created by jamie on 2025/5/6.
//
#include "ime/ImeController.h"

#include "FakeDirectInputDevice.h"
#include "ImeWnd.hpp"
#include "common/imgui/ErrorNotifier.h"
#include "configs/CustomMessage.h"
#include "hooks/Hooks.hpp"
#include "hooks/ScaleformHook.h"
#include "ui/Settings.h"
#include "ui/TaskQueue.h"

namespace Ime
{

void ImeController::ApplySettings()
{
    if (!IsInited())
    {
        ErrorNotifier::GetInstance().Error("Fatal error: IME manager is not initialized.");
        return;
    }
    EnableMod(m_settings->enableMod);

    m_fDirty = true;
    if (m_fEnabledMod)
    {
        SyncImeStateIfDirty();
    }
}

auto ImeController::EnableMod(bool enable) -> void
{
    if (m_fEnabledMod != enable)
    {
        AddTask([this, enable] {
            const bool prev = m_fEnabledMod;
            if (!prev)
            {
                m_fEnabledMod = true;
            }
            if (IImeModule::IsSuccess(DoEnableMod(enable)))
            {
                m_fEnabledMod = enable;
                m_fDirty      = enable;
                return;
            }
            m_fEnabledMod = prev;
            ErrorNotifier::GetInstance().Debug(std::format("Unexpected error: EnableMod({}) failed.", enable));
        });
    }
}

void ImeController::EnableIme(bool enable) const
{
    AddTask([this, enable] {
        if (IImeModule::IsFailed(DoEnableIme(enable)))
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: EnableIme failed.");
        }
    });
}

void ImeController::ForceFocusIme() const
{
    AddTask([this] {
        if (IImeModule::IsFailed(DoForceFocusIme()))
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: ForceFocusIme failed.");
        }
    });
}

void ImeController::SyncImeState()
{
    AddTask([this] {
        if (IImeModule::IsFailed(DoSyncImeState()))
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: SyncImeState failed");
        }
    });
}

void ImeController::TryFocusIme() const
{
    AddTask([this] {
        if (IImeModule::IsFailed(DoTryFocusIme()))
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: TryFocusIme failed");
        }
    });
}

auto ImeController::DoEnableMod(const bool enable) const -> IImeModule::Result
{
    const bool shouldEnableIme =
        enable && (m_settings->input.keepImeOpen || Hooks::ControlMap::GetSingleton()->HasTextEntry());

    auto result = DoEnableIme(shouldEnableIme);

    bool fResult = IImeModule::IsSuccess(result);
    if (fResult)
    {
        fResult = enable ? UnlockKeyboard() : RestoreKeyboard();
        fResult = fResult && FocusImeOrGame(enable);
    }

    if (!fResult)
    {
        logger::error("Can't enable/disable mod. last error {}", GetLastError());
    }
    return fResult ? IImeModule::Result::SUCCESS : IImeModule::Result::FAILED;
}

auto ImeController::DoEnableIme(bool enable) const -> IImeModule::Result
{
    if (!m_fEnabledMod)
    {
        return IImeModule::Result::DISABLED;
    }
    const auto result = m_delegate->EnableIme(m_settings->input.keepImeOpen || enable);
    if (!IImeModule::IsSuccess(result))
    {
        ErrorNotifier::GetInstance().Warning(std::format("Unexpected error: EnableIme({}) failed.", enable));
        return result;
    }
    return result;
}

auto ImeController::DoForceFocusIme() const -> IImeModule::Result
{
    if (!m_fEnabledMod)
    {
        return IImeModule::Result::DISABLED;
    }
    const auto result = m_delegate->ForceFocusIme();
    if (!IImeModule::IsSuccess(result))
    {
        ErrorNotifier::GetInstance().Warning("Unexpected error: ForceFocusIme failed");
        return result;
    }
    return result;
}

auto ImeController::DoTryFocusIme() const -> IImeModule::Result
{
    if (!m_fEnabledMod)
    {
        return IImeModule::Result::DISABLED;
    }
    const auto result = m_delegate->TryFocusIme();
    if (!IImeModule::IsSuccess(result))
    {
        ErrorNotifier::GetInstance().Warning("Unexpected error: TryFocusIme failed");
        return result;
    }
    return result;
}

auto ImeController::DoSyncImeState() -> IImeModule::Result
{
    if (!m_fEnabledMod)
    {
        return IImeModule::Result::DISABLED;
    }
    m_fDirty          = false;
    const auto result = m_delegate->SyncImeState();
    if (!IImeModule::IsSuccess(result))
    {
        ErrorNotifier::GetInstance().Error("Unexpected error: SyncImeState failed.");
        return result;
    }
    return result;
}

auto ImeController::RestoreKeyboard() const -> bool
{
    logger::debug("Restore keyboard: EXCLUSIVE + FOREGROUND + NOWINKEY.");
    HRESULT hr = E_FAIL;
    if (auto *keyboard = Hooks::FakeDirectInputDevice::GetInstance(); keyboard != nullptr)
    {
        hr = keyboard->TryRestoreCooperativeLevel(m_gameHwnd);
    }
    if (FAILED(hr))
    {
        logger::error("Failed lock keyboard.");
    }
    return SUCCEEDED(hr);
}

auto ImeController::UnlockKeyboard() const -> bool
{
    logger::debug("Unlock keyboard: NONEXCLUSIVE + BACKGROUND.");
    HRESULT hr = E_FAIL;
    if (auto *keyboard = Hooks::FakeDirectInputDevice::GetInstance(); keyboard != nullptr)
    {
        hr = keyboard->TryUnlockCooperativeLevel(m_gameHwnd);
    }
    if (FAILED(hr))
    {
        logger::error("Failed unlock keyboard.");
    }
    return SUCCEEDED(hr);
}

bool ImeController::FocusImeOrGame(const bool focusIme) const
{
    bool success = true;
    if (focusIme)
    {
        m_imeWnd->Focus();
    }
    else
    {
        success = ImeManager::Focus(m_gameHwnd);
    }

    if (!success)
    {
        logger::error("Failed focus to {}", focusIme ? "IME" : "Game");
    }
    return success;
}

void ImeController::AddTask(TaskQueue::Task &&task) const
{
    TaskQueue::GetInstance().AddImeThreadTask(std::move(task));
    SendMessageA(m_imeWnd->GetHWND(), CM_EXECUTE_TASK, 0, 0);
}

void ImeController::Init(ImeWnd *imeWnd, HWND gameHwnd, Settings &settings)
{
    auto *instance = GetInstance();
    if (instance->m_fInited)
    {
        return;
    }
    instance->m_settings = &settings;
    instance->m_delegate = std::make_unique<ImeManager>(gameHwnd, imeWnd, settings);
    instance->m_gameHwnd = gameHwnd;
    instance->m_imeWnd   = imeWnd;
    instance->m_fInited  = true;
}

} // namespace Ime
