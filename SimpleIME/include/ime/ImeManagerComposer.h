#pragma once

#include "common/imgui/ErrorNotifier.h"
#include "ime/BaseImeManager.h"
#include "ime/ImeManager.h"
#include "ui/Settings.h"
#include "ui/TaskQueue.h"

#include <stack>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
struct SettingsConfig;

class DummyFocusImeManager final : public BaseImeManager
{
public:
    explicit DummyFocusImeManager(Settings &settings) : BaseImeManager(nullptr, settings) {}

protected:
    auto DoEnableIme(bool) -> bool override
    {
        return false;
    }

    auto DoEnableMod(bool) -> bool override
    {
        return false;
    }

    auto DoForceFocusIme() -> bool override
    {
        return false;
    }

    auto DoSyncImeState() -> bool override
    {
        return false;
    }

    auto DoTryFocusIme() -> bool override
    {
        return false;
    }
};

class ImeManagerComposer
{
    void Use(Settings::FocusType type);

public:
    void ApplyUiSettings(const Settings &settings);

    void PushType(Settings::FocusType type, bool syncImeState = false);

    void PopType(bool syncImeState = false);

    void PopAndPushType(Settings::FocusType type, bool syncImeState = false);

    constexpr auto GetFocusManageType() const -> const Settings::FocusType &
    {
        return m_FocusTypeStack.top();
    }

    void SyncImeStateIfDirty()
    {
        if (m_fDirty)
        {
            SyncImeState();
        }
    }

    void MarkDirty()
    {
        m_fDirty = true;
    }

    [[nodiscard]] constexpr auto IsDirty() const -> bool
    {
        return m_fDirty;
    }

    auto IsModEnabled() const -> bool
    {
        return m_settings->enableMod;
    }

    //////////////////////////////////////////////////

    auto EnableIme(bool enable) const -> void;

    auto EnableMod(bool enable) -> void;

    auto GiveUpFocus() const -> void;

    auto ForceFocusIme() const -> void;

    auto TryFocusIme() const -> void;

    auto SyncImeState() -> void;

    [[nodiscard]] auto IsInited() const -> bool
    {
        return m_fInited;
    }

    static auto GetInstance() -> ImeManagerComposer *
    {
        static ImeManagerComposer g_instance;
        return &g_instance;
    }

private:
    Settings                       *m_settings = nullptr;
    std::unique_ptr<ImeManager>     m_delegate = nullptr;
    std::stack<Settings::FocusType> m_FocusTypeStack;
    ImeWnd                         *m_pImeWnd  = nullptr;
    HWND                            m_gameHwnd = nullptr;
    bool                            m_fDirty   = false;
    std::atomic_bool                m_fInited  = false;

    void AddTask(TaskQueue::Task &&task) const;

    friend class ImeWnd;

    static void Init(ImeWnd *imeWnd, HWND gameHwnd, Settings *settings)
    {
        auto *instance = GetInstance();
        if (instance->m_fInited)
        {
            return;
        }
        instance->m_settings = settings;
        instance->m_delegate = std::make_unique<DummyFocusImeManager>(*settings);
        instance->m_gameHwnd = gameHwnd;
        instance->m_pImeWnd  = imeWnd;
        instance->m_fInited  = true;
    }
};
}
}