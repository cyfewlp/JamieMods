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
            using State = Core::State;

        public:
            auto EnableIme(bool enable) -> bool override
            {
                log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
                if (!IsModeEnabled())
                {
                    log_warn("Mod is disabled, operate failed");
                    return false;
                }
                auto isEnable = Context::GetInstance()->KeepImeOpen() || enable;
                return DoEnableIme(isEnable);
            }

            auto NotifyEnableIme(bool enable) const -> bool override
            {
                log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
                if (!IsModeEnabled())
                {
                    log_warn("Mod is disabled, operate failed");
                    return false;
                }
                return DoNotifyEnableIme(enable);
            }

            auto WaitEnableIme(bool enable) const -> bool override
            {
                log_debug("BaseImeManager::{} {}", __func__, enable ? "enable" : "disable");
                if (!IsModeEnabled())
                {
                    log_warn("Mod is disabled, operate failed");
                    return false;
                }
                return DoWaitEnableIme(enable);
            }

            auto EnableMod(bool fEnableMod) -> bool override
            {
                log_debug("BaseImeManager::{} {}", __func__, fEnableMod ? "enable" : "disable");
                if (State::GetInstance().IsModEnabled() == fEnableMod)
                {
                    return true;
                }
                log_debug("{} mod", fEnableMod ? "enable" : "disable");
                if (!fEnableMod)
                {
                    Context::GetInstance()->SetKeepImeOpen(false);
                }
                State::GetInstance().SetEnableMod(fEnableMod);
                return DoEnableMod(fEnableMod);
            }

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

            auto ForceFocusIme() -> bool override
            {
                log_debug("BaseImeManager::{}", __func__);
                if (!IsModeEnabled())
                {
                    log_warn("Mod is disabled, operate failed");
                    return false;
                }
                return DoForceFocusIme();
            }

            auto SyncImeState() const -> bool override
            {
                log_debug("BaseImeManager::{}", __func__);
                if (!IsModeEnabled())
                {
                    log_warn("Mod is disabled, operate failed");
                    return false;
                }
                return DoSyncImeState();
            }

            auto TryFocusIme() -> bool override
            {
                log_debug("BaseImeManager::{}", __func__);
                if (!IsModeEnabled())
                {
                    log_warn("Mod is disabled, operate failed");
                    return false;
                }
                return DoTryFocusIme();
            }

        protected:
            static constexpr auto IsModeEnabled() -> bool
            {
                return Core::State::GetInstance().IsModEnabled();
            }

            virtual auto DoEnableIme(bool enable) -> bool             = 0;
            virtual auto DoNotifyEnableIme(bool enable) const -> bool = 0;
            virtual auto DoWaitEnableIme(bool enable) const -> bool   = 0;
            virtual auto DoEnableMod(bool enable) -> bool             = 0;
            virtual auto DoNotifyEnableMod(bool enable) const -> bool = 0;
            virtual auto DoWaitEnableMod(bool enable) const -> bool   = 0;
            virtual auto DoForceFocusIme() -> bool                    = 0;
            virtual auto DoSyncImeState() const -> bool               = 0;
            virtual auto DoTryFocusIme() -> bool                      = 0;
        };
    }
}