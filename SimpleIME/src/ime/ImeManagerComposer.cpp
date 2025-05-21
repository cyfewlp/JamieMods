//
// Created by jamie on 2025/5/6.
//
#include "ime/ImeManagerComposer.h"

#include "ImeWnd.hpp"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "ui/TaskQueue.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
auto ImeManagerComposer::Use(FocusType type)
{
    if (FocusType::Permanent == type)
    {
        m_delegate = m_PermanentFocusImeManager.get();
    }
    else
    {
        m_delegate = m_temporaryFocusImeManager.get();
    }
}

void ImeManagerComposer::ApplyUiSettings(const SettingsConfig &settingsConfig)
{
    if (!IsInited())
    {
        ErrorNotifier::GetInstance().addError("Fatal error: IME manager is not initialized.");
        return;
    }
    PushType(settingsConfig.focusType.Value());
    SetImeWindowPosUpdatePolicy(settingsConfig.windowPosUpdatePolicy.Value());
    SetEnableUnicodePaste(settingsConfig.enableUnicodePaste.Value());
    SetKeepImeOpen(settingsConfig.keepImeOpen.Value());
    EnableMod(settingsConfig.enableMod.Value());
    if (settingsConfig.enableMod.Value())
    {
        SyncImeStateIfDirty();
    }
}

void ImeManagerComposer::SyncUiSettings(SettingsConfig &settingsConfig) const
{
    if (!IsInited())
    {
        return;
    }
    settingsConfig.enableMod.SetValue(ImeManager::IsModEnabled());
    settingsConfig.focusType.SetValue(GetFocusManageType());
    settingsConfig.windowPosUpdatePolicy.SetValue(m_ImeWindowPosUpdatePolicy);
    settingsConfig.enableUnicodePaste.SetValue(m_fEnableUnicodePaste);
    settingsConfig.keepImeOpen.SetValue(m_fKeepImeOpen);
}

void ImeManagerComposer::PushType(FocusType type, bool syncImeState)
{
    assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
    const bool diff = m_FocusTypeStack.empty() || type != m_FocusTypeStack.top();
    if (diff)
    {
        m_fDirty = true;
        if (!dynamic_cast<DummyFocusImeManager *>(m_delegate))
        {
            EnableIme(false);
        }
    }
    m_FocusTypeStack.push(type);
    Use(type);
    if (diff && syncImeState)
    {
        SyncImeStateIfDirty();
    }
}

void ImeManagerComposer::PopType(bool syncImeState)
{
    assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
    if (m_FocusTypeStack.empty())
    {
        ErrorNotifier::GetInstance().addError("Invalid call! Focus type stack is empty.");
        return;
    }
    auto prev = m_FocusTypeStack.top();
    m_FocusTypeStack.pop();
    if (m_FocusTypeStack.empty())
    {
        /*log_warn("Current focus type stack is empty, set fall back type to Permanent");
        PushType(FocusType::Permanent);*/
        m_delegate = nullptr;
    }
    else if (prev != m_FocusTypeStack.top())
    {
        m_fDirty = true;
        EnableIme(false);
        Use(m_FocusTypeStack.top());
        if (syncImeState)
        {
            SyncImeStateIfDirty();
        }
    }
}

void ImeManagerComposer::PopAndPushType(const FocusType type, const bool syncImeState)
{
    assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
    if (m_FocusTypeStack.empty())
    {
        ErrorNotifier::GetInstance().addError("Invalid call! Focus type stack is empty.");
        return;
    }
    if (const auto prev = m_FocusTypeStack.top(); prev == type)
    {
        if (syncImeState)
        {
            SyncImeStateIfDirty();
        }
        return;
    }
    m_fDirty = true;
    EnableIme(false);
    m_FocusTypeStack.pop();
    m_FocusTypeStack.push(type);
    Use(type);

    if (syncImeState)
    {
        SyncImeStateIfDirty();
    }
}

auto ImeManagerComposer::EnableIme(bool enable) const -> void
{
    AddTask([this, enable] {
        if (!m_delegate->EnableIme(m_fKeepImeOpen || enable))
        {
            ErrorNotifier::GetInstance().addError(std::format("Unexpected error: EnableIme({}) failed.", enable));
        }
    });
}

auto ImeManagerComposer::EnableMod(bool enable) -> void
{
    if (ImeManager::IsModEnabled() != enable)
    {
        AddTask([this, enable] {
            if (m_delegate->EnableMod(enable))
            {
                m_fDirty = true;
                ImeManager::SetEnableMod(enable);
                return;
            }
            ErrorNotifier::GetInstance().Debug(std::format("Unexpected error: EnableMod({}) failed.", enable));
        });
    }
}

auto ImeManagerComposer::GiveUpFocus() const -> void
{
    AddTask([this] {
        if (!m_delegate->GiveUpFocus())
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: GiveUpFocus failed.");
        }
    });
}

auto ImeManagerComposer::ForceFocusIme() const -> void
{
    AddTask([this] {
        if (!m_delegate->ForceFocusIme())
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: ForceFocusIme failed");
        }
    });
}

auto ImeManagerComposer::TryFocusIme() const -> void
{
    AddTask([this] {
        if (!m_delegate->TryFocusIme())
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: TryFocusIme failed");
        }
    });
}

auto ImeManagerComposer::SyncImeState() -> void
{
    AddTask([this] {
        m_fDirty = false;
        if (!m_delegate->SyncImeState())
        {
            ErrorNotifier::GetInstance().Warning("Unexpected error: SyncImeState failed.");
        }
    });
}

void ImeManagerComposer::AddTask(TaskQueue::Task &&task) const
{
    if (m_FocusTypeStack.top() == FocusType::Permanent)
    {
        TaskQueue::GetInstance().AddImeThreadTask(std::move(task));
        ::SendNotifyMessageA(m_pImeWnd->GetHWND(), CM_EXECUTE_TASK, 0, 0);
    }
    else
    {
        TaskQueue::GetInstance().AddMainThreadTask(std::move(task));
        ::SendNotifyMessageA(reinterpret_cast<HWND>(RE::Main::GetSingleton()->wnd), CM_EXECUTE_TASK, 0, 0);
    }
}
}
}