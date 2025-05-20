#pragma once

#include "common/imgui/ErrorNotifier.h"
#include "ime/ImeManager.h"
#include "ime/PermanentFocusImeManager.h"
#include "ime/TemporaryFocusImeManager.h"

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

    auto DoNotifyEnableIme(bool) const -> bool override
    {
        return false;
    }

    auto DoWaitEnableIme(bool) const -> bool override
    {
        return false;
    }

    auto DoEnableMod(bool) -> bool override
    {
        return false;
    }

    auto DoNotifyEnableMod(bool) const -> bool override
    {
        return false;
    }

    auto DoWaitEnableMod(bool) const -> bool override
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

class ImeManagerComposer final : public ImeManager
{
public:
    ImeManagerComposer()           = default;
    ~ImeManagerComposer() override = default;

private:
    auto Use(FocusType type);

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
        if (!IsModEnabled())
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
        return IsModEnabled() && !m_fSupportOtherMod && m_fEnableUnicodePaste;
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

    [[nodiscard]] constexpr auto IsSupportOtherMod() const -> bool
    {
        return m_fSupportOtherMod;
    }

    void SetSupportOtherMod(const bool fSupportOtherMod)
    {
        m_fSupportOtherMod = fSupportOtherMod;
    }

    void SyncImeStateIfDirty()
    {
        if (m_fDirty && !SyncImeState())
        {
            ErrorNotifier::GetInstance().addError("Unexpected error: SyncImeState failed.");
        }
    }

    [[nodiscard]] constexpr auto IsDirty() const -> bool
    {
        return m_fDirty;
    }

    //////////////////////////////////////////////////

    auto EnableIme(bool enable) -> bool override;

    auto EnableMod(bool enable) -> bool override;

    auto GiveUpFocus() const -> bool override
    {
        return m_delegate->GiveUpFocus();
    }

    auto ForceFocusIme() -> bool override
    {
        return m_delegate->ForceFocusIme();
    }

    auto TryFocusIme() -> bool override
    {
        return m_delegate->TryFocusIme();
    }

    auto SyncImeState() -> bool override
    {
        m_fDirty = false;
        return m_delegate->SyncImeState();
    }

    auto NotifyEnableIme(bool enable) const -> bool override
    {
        return m_delegate->NotifyEnableIme(enable);
    }

    auto NotifyEnableMod(bool enable) const -> bool override
    {
        return m_delegate->NotifyEnableMod(enable);
    }

    auto WaitEnableIme(bool enable) const -> bool override
    {
        return m_delegate->WaitEnableIme(enable);
    }

    auto WaitEnableMod(bool enable) const -> bool override
    {
        return m_delegate->WaitEnableMod(enable);
    }

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
    std::stack<FocusType>                     m_FocusTypeStack{};
    bool                                      m_fKeepImeOpen        = false;
    bool                                      m_fEnableUnicodePaste = true;
    std::atomic_bool                          m_fSupportOtherMod    = false;
    std::atomic_bool                          m_fInited             = false;

    // m_fKeepImeOpen, focus type
    bool m_fDirty = false;

    ImeWindowPosUpdatePolicy m_ImeWindowPosUpdatePolicy = ImeWindowPosUpdatePolicy::BASED_ON_CARET;

    friend class ImeWnd;

    static void Init(ImeWnd *imwWnd, HWND hwndGame)
    {
        auto *instance = GetInstance();
        if (instance->m_fInited) return;

        instance->m_PermanentFocusImeManager = std::make_unique<PermanentFocusImeManager>(imwWnd, hwndGame);
        instance->m_temporaryFocusImeManager = std::make_unique<TemporaryFocusImeManager>(imwWnd, hwndGame);
        instance->m_fInited                  = true;
    }
};
}
}