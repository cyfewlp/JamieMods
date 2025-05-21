#pragma once

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
    explicit BaseImeManager(HWND gameHwnd) : ImeManager(gameHwnd) {}

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

protected:
    virtual auto DoEnableIme(bool enable) -> bool = 0;
    virtual auto DoEnableMod(bool enable) -> bool = 0;
    virtual auto DoForceFocusIme() -> bool        = 0;
    virtual auto DoSyncImeState() -> bool         = 0;
    virtual auto DoTryFocusIme() -> bool          = 0;
};
}
}