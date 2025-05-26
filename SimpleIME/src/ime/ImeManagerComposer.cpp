//
// Created by jamie on 2025/5/6.
//
#include "ime/ImeManagerComposer.h"

#include "ImeWnd.hpp"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "ime/PermanentFocusImeManager.h"
#include "ime/TemporaryFocusImeManager.h"
#include "ui/Settings.h"
#include "ui/TaskQueue.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
void ImeManagerComposer::Use(const Settings::FocusType type)
{
    if (Settings::FocusType::Permanent == type)
    {
        m_delegate = std::make_unique<PermanentFocusImeManager>(m_pImeWnd, m_gameHwnd, *m_settings);
    }
    else
    {
        m_delegate = std::make_unique<TemporaryFocusImeManager>(m_pImeWnd, m_gameHwnd, *m_settings);
    }
}

void ImeManagerComposer::ApplyUiSettings(const Settings &settings)
{
    if (!IsInited())
    {
        ErrorNotifier::GetInstance().Error("Fatal error: IME manager is not initialized.");
        return;
    }
    PushType(settings.focusType);
    EnableMod(settings.enableMod);

    m_fDirty = true;
    if (settings.enableMod)
    {
        SyncImeStateIfDirty();
    }
}

void ImeManagerComposer::PushType(Settings::FocusType type, bool syncImeState)
{
    assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
    const bool diff = m_FocusTypeStack.empty() || type != m_FocusTypeStack.top();
    if (diff)
    {
        m_fDirty = true;
        if (!dynamic_cast<DummyFocusImeManager *>(m_delegate.get()))
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
        ErrorNotifier::GetInstance().Error("Invalid call! Focus type stack is empty.");
        return;
    }
    auto prev = m_FocusTypeStack.top();
    m_FocusTypeStack.pop();
    if (m_FocusTypeStack.empty())
    {
        m_delegate = std::make_unique<DummyFocusImeManager>(*m_settings);
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

void ImeManagerComposer::PopAndPushType(const Settings::FocusType type, const bool syncImeState)
{
    assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
    if (m_FocusTypeStack.empty())
    {
        ErrorNotifier::GetInstance().Error("Invalid call! Focus type stack is empty.");
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
    AddTask([this] {
        if (!m_delegate->EnableIme(false)) // force disable ime with last focus type;
        {
            ErrorNotifier::GetInstance().Warning(std::format("Unexpected error: disable ime failed."));
        }
    });
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
        if (!m_delegate->EnableIme(m_settings->keepImeOpen || enable))
        {
            ErrorNotifier::GetInstance().Warning(std::format("Unexpected error: EnableIme({}) failed.", enable));
        }
    });
}

auto ImeManagerComposer::EnableMod(bool enable) -> void
{
    if (m_settings->enableMod != enable)
    {
        AddTask([this, enable] {
            if (m_delegate->EnableMod(enable))
            {
                m_fDirty              = enable;
                m_settings->enableMod = enable;
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
            ErrorNotifier::GetInstance().Error("Unexpected error: SyncImeState failed.");
        }
    });
}

void ImeManagerComposer::AddTask(TaskQueue::Task &&task) const
{
    if (m_FocusTypeStack.empty())
    {
        return;
    }
    if (m_FocusTypeStack.top() == Settings::FocusType::Permanent)
    {
        TaskQueue::GetInstance().AddImeThreadTask(std::move(task));
        ::SendMessageA(m_pImeWnd->GetHWND(), CM_EXECUTE_TASK, 0, 0);
    }
    else
    {
        TaskQueue::GetInstance().AddMainThreadTask(std::move(task));
        ::SendMessageA(m_gameHwnd, CM_EXECUTE_TASK, 0, 0);
    }
}
}
}