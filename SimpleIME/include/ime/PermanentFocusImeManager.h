#pragma once

#include "ime/BaseImeManager.h"

namespace LIBC_NAMESPACE_DECL
{
	namespace Ime
	{

        class PermanentFocusImeManager final : public BaseImeManager
        {
            using State = Core::State;

        public:
            PermanentFocusImeManager(ImeWnd *pImeWnd, HWND hwndGame) : m_ImeWnd(pImeWnd), m_hwndGame(hwndGame)
            {
            }

            ~PermanentFocusImeManager() override = default;

        protected:
            auto DoEnableIme(bool enable) -> bool override;
            auto DoNotifyEnableIme(bool enable) const -> bool override;
            auto DoWaitEnableIme(bool enable) const -> bool override;
            auto DoEnableMod(bool enable) -> bool override;
            auto DoNotifyEnableMod(bool enable) const -> bool override;
            auto DoWaitEnableMod(bool enable) const -> bool override;
            auto DoForceFocusIme() -> bool override;
            auto DoSyncImeState() -> bool override;
            auto DoTryFocusIme() -> bool override;

        private:
            ImeWnd *m_ImeWnd;
            HWND    m_hwndGame;

            auto FocusImeOrGame(bool focusIme) const -> bool;
        };
	}
}