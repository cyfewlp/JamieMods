#pragma once

#include "context.h"
#include "core/State.h"
#include "ime/ImeManager.h"
#include "ui/Settings.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class BaseImeManager : public ImeManager
{
    static inline std::atomic_bool g_fKeepImeOpen;
    using State                 = Core::State;
    mutable bool m_fForceUpdate = false;
    Settings    &m_settings;

public:
    explicit BaseImeManager(HWND gameHwnd, Settings &settings) : ImeManager(gameHwnd), m_settings(settings) {}

    auto EnableIme(bool enable) -> bool override;

    auto EnableMod(bool fEnableMod) -> bool override;

    // no effect.
    auto GiveUpFocus() const -> bool override
    {
        // log_debug("BaseImeManager::{}", __func__);
        // if (!IsModeEnabled())
        //{
        //     log_warn("Mod is disabled, operate failed");
        //     return false;
        // }
        return false;
    }

    auto ForceFocusIme() -> bool override;

    auto SyncImeState() -> bool override;

    auto TryFocusIme() -> bool override;

    // return true if checked "KeepImeOpen" or exists text entry, otherwise return false;
    auto IsShouldEnableIme() const -> bool;

protected:
    virtual auto DoEnableIme(bool enable) -> bool = 0;
    virtual auto DoEnableMod(bool enable) -> bool = 0;
    virtual auto DoForceFocusIme() -> bool        = 0;
    virtual auto DoSyncImeState() -> bool         = 0;
    virtual auto DoTryFocusIme() -> bool          = 0;
};
}
}