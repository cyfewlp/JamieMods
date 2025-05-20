#pragma once

#include "common/log.h"
#include "context.h"
#include "core/State.h"
#include "ime/ImeManager.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
class BaseImeManager : public ImeManager
{
    static inline std::atomic_bool g_fKeepImeOpen;
    using State                 = Core::State;
    mutable bool m_fForceUpdate = false;

public:
    auto EnableIme(bool enable) -> bool override;

    auto NotifyEnableIme(bool enable) const -> bool override;

    auto WaitEnableIme(bool enable) const -> bool override;

    auto EnableMod(bool fEnableMod) -> bool override;

    auto NotifyEnableMod(bool enable) const -> bool override
    {
        log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
        return DoNotifyEnableMod(enable);
    }

    auto WaitEnableMod(bool enable) const -> bool override
    {
        log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
        return DoWaitEnableMod(enable);
    }

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

protected:
    virtual auto DoEnableIme(bool enable) -> bool             = 0;
    virtual auto DoNotifyEnableIme(bool enable) const -> bool = 0;
    virtual auto DoWaitEnableIme(bool enable) const -> bool   = 0;
    virtual auto DoEnableMod(bool enable) -> bool             = 0;
    virtual auto DoNotifyEnableMod(bool enable) const -> bool = 0;
    virtual auto DoWaitEnableMod(bool enable) const -> bool   = 0;
    virtual auto DoForceFocusIme() -> bool                    = 0;
    virtual auto DoSyncImeState() -> bool                     = 0;
    virtual auto DoTryFocusIme() -> bool                      = 0;
};
}
}