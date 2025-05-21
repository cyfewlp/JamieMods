#pragma once

#include "common/imgui/ErrorNotifier.h"
#include "ime/ImeManager.h"
#include "ime/PermanentFocusImeManager.h"
#include "ime/TemporaryFocusImeManager.h"
#include "ui/TaskQueue.h"

#include <stack>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
struct SettingsConfig;

enum class FocusType : uint8_t
{
    Permanent = 0,
    Temporary
};

class DummyFocusImeManager final : public BaseImeManager
{
public:
    explicit DummyFocusImeManager() : BaseImeManager(nullptr) {}

    static auto GetInstance() -> DummyFocusImeManager *
    {
        static DummyFocusImeManager instance;
        return &instance;
    }

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
    void Use(FocusType type);

public:
    enum class ImeWindowPosUpdatePolicy : uint8_t
    {
        NONE = 0,
        BASED_ON_CURSOR,
        BASED_ON_CARET,
    };

    void ApplyUiSettings(const SettingsConfig &settingsConfig);
    void SyncUiSettings(SettingsConfig &settingsConfig) const;

    void PushType(FocusType type, bool syncImeState = false);

    void PopType(bool syncImeState = false);

    void PopAndPushType(FocusType type, bool syncImeState = false);

    constexpr auto GetFocusManageType() const -> const FocusType &
    {
        return m_FocusTypeStack.top();
    }

    auto GetTemporaryFocusImeManager() const -> TemporaryFocusImeManager *
    {
        return m_temporaryFocusImeManager.get();
    }

    [[nodiscard]] constexpr auto GetImeWindowPosUpdatePolicy() const -> ImeWindowPosUpdatePolicy
    {
        if (!ImeManager::IsModEnabled())
        {
            return ImeWindowPosUpdatePolicy::NONE;
        }
        return m_ImeWindowPosUpdatePolicy;
    }

    void SetImeWindowPosUpdatePolicy(const ImeWindowPosUpdatePolicy policy)
    {
        m_ImeWindowPosUpdatePolicy = policy;
    }

    [[nodiscard]] constexpr auto IsUnicodePasteEnabled() const -> bool
    {
        return ImeManager::IsModEnabled() && m_fEnableUnicodePaste;
    }

    void SetEnableUnicodePaste(const bool fEnableUnicodePaste)
    {
        m_fEnableUnicodePaste = fEnableUnicodePaste;
    }

    [[nodiscard]] constexpr auto IsKeepImeOpen() const -> bool
    {
        return m_fKeepImeOpen;
    }

    void SetKeepImeOpen(const bool fKeepImeOpen)
    {
        if (m_fKeepImeOpen != fKeepImeOpen)
        {
            m_fDirty = true;
        }
        m_fKeepImeOpen = fKeepImeOpen;
    }

    void SyncImeStateIfDirty()
    {
        if (m_fDirty)
        {
            SyncImeState();
        }
    }

    [[nodiscard]] constexpr auto IsDirty() const -> bool
    {
        return m_fDirty;
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
    std::unique_ptr<PermanentFocusImeManager> m_PermanentFocusImeManager = nullptr;
    std::unique_ptr<TemporaryFocusImeManager> m_temporaryFocusImeManager = nullptr;
    ImeManager                               *m_delegate                 = DummyFocusImeManager::GetInstance();
    std::stack<FocusType>                     m_FocusTypeStack;
    bool                                      m_fKeepImeOpen        = false;
    bool                                      m_fEnableUnicodePaste = true;
    std::atomic_bool                          m_fInited             = false;
    ImeWnd                                   *m_pImeWnd             = nullptr;
    HWND                                      m_gameHwnd            = nullptr;

    // m_fKeepImeOpen, focus type
    bool m_fDirty = false;

    ImeWindowPosUpdatePolicy m_ImeWindowPosUpdatePolicy = ImeWindowPosUpdatePolicy::BASED_ON_CARET;
    void                     AddTask(TaskQueue::Task &&task) const;

    friend class ImeWnd;

    static void Init(ImeWnd *imeWnd, HWND gameHwnd)
    {
        auto *instance = GetInstance();
        if (instance->m_fInited)
        {
            return;
        }
        instance->m_gameHwnd                 = gameHwnd;
        instance->m_pImeWnd                  = imeWnd;
        instance->m_PermanentFocusImeManager = std::make_unique<PermanentFocusImeManager>(imeWnd, gameHwnd);
        instance->m_temporaryFocusImeManager = std::make_unique<TemporaryFocusImeManager>(imeWnd, gameHwnd);
        instance->m_fInited                  = true;
    }
};
}
}