#pragma once

#include "common/config.h"

#include "core/State.h"
#include <memory>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImeWnd;

        class ImeManager
        {
            using State = Core::State;

        public:
            ImeManager()                                  = default;
            virtual ~ImeManager()                         = default;
            ImeManager(const ImeManager &other)           = delete;
            ImeManager(ImeManager &&other)                = delete;
            ImeManager operator=(const ImeManager &other) = delete;
            ImeManager operator=(ImeManager &&other)      = delete;

            virtual auto EnableIme(bool enable) -> bool             = 0;
            virtual auto NotifyEnableIme(bool enable) const -> bool = 0;
            virtual auto EnableMod(bool enable) -> bool             = 0;
            virtual auto NotifyEnableMod(bool enable) const -> bool = 0;
            virtual auto GiveUpFocus() const -> bool                = 0;
            virtual auto ForceFocusIme() -> bool                    = 0;
            virtual auto SyncImeState() const -> bool               = 0;
            /// <summary>
            /// Try focus IME when mod is enabled
            /// </summary>
            /// <returns>true if mod enabled and focus success, otherwise false</returns>
            virtual auto TryFocusIme() -> bool = 0;

            static auto Focus(HWND hwnd) -> bool;

            static auto UnlockKeyboard() -> bool;

            static auto RestoreKeyboard() -> bool;
        };

        class PermanentFocusImeManager : public ImeManager
        {
            using State = Core::State;

        public:
            PermanentFocusImeManager(ImeWnd *pImeWnd, HWND hwndGame) : m_ImeWnd(pImeWnd), m_hwndGame(hwndGame)
            {
            }

            ~PermanentFocusImeManager() override = default;

            auto EnableIme(bool enable) -> bool override;
            auto NotifyEnableIme(bool enable) const -> bool override;
            auto EnableMod(bool enable) -> bool override;
            auto NotifyEnableMod(bool enable) const -> bool override;
            auto GiveUpFocus() const -> bool override;
            auto ForceFocusIme() -> bool override;
            auto TryFocusIme() -> bool override;
            auto SyncImeState() const -> bool override;

        private:
            ImeWnd *m_ImeWnd;
            HWND    m_hwndGame;

            auto FocusImeOrGame(bool focusIme) const -> bool;
        };

        class TemporaryFocusImeManager : public ImeManager
        {
            using State = Core::State;

        public:
            TemporaryFocusImeManager(ImeWnd *pImeWnd, HWND hwndGame) : m_ImeWnd(pImeWnd), m_hwndGame(hwndGame)
            {
            }

            ~TemporaryFocusImeManager() override = default;

            auto EnableIme(bool enable) -> bool override;
            auto NotifyEnableIme(bool enable) const -> bool override;
            auto EnableMod(bool enable) -> bool override;
            auto NotifyEnableMod(bool enable) const -> bool override;
            auto GiveUpFocus() const -> bool override;
            auto ForceFocusIme() -> bool override;
            auto TryFocusIme() -> bool override;
            auto SyncImeState() const -> bool override;

        private:
            ImeWnd *m_ImeWnd;
            HWND    m_hwndGame;
            bool    m_fIsInEnableIme = false;
        };

        class ImeManagerComposer : public ImeManager
        {
        public:
            ImeManagerComposer(ImeWnd *pImeWnd, HWND hwndGame)
                : m_PermanentFocusImeManager(std::make_unique<PermanentFocusImeManager>(pImeWnd, hwndGame)),
                  m_temporaryFocusImeManager(std::make_unique<TemporaryFocusImeManager>(pImeWnd, hwndGame)),
                  m_delegate(m_PermanentFocusImeManager.get())
            {
            }

            ~ImeManagerComposer() override = default;

            auto UsePermanentFocusImeManager()
            {
                m_delegate = m_PermanentFocusImeManager.get();
            }

            auto UseTemporaryFocusImeManager()
            {
                m_delegate = m_temporaryFocusImeManager.get();
            }

            auto GetTemporaryFocusImeManager() -> TemporaryFocusImeManager*
            {
                return m_temporaryFocusImeManager.get();
            }

            auto EnableIme(bool enable) -> bool override
            {
                return m_delegate->EnableIme(enable);
            }

            auto EnableMod(bool enable) -> bool override
            {
                return m_delegate->EnableMod(enable);
            }

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

            auto SyncImeState() const -> bool override
            {
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

            static auto GetInstance() -> ImeManagerComposer *
            {
                return g_instance.get();
            }

        private:
            std::unique_ptr<PermanentFocusImeManager> m_PermanentFocusImeManager = nullptr;
            std::unique_ptr<TemporaryFocusImeManager>      m_temporaryFocusImeManager         = nullptr;
            ImeManager                          *m_delegate;

            friend class ImeWnd;

            static void Init(ImeWnd *imwWnd, HWND hwndGame)
            {
                g_instance = std::make_unique<ImeManagerComposer>(imwWnd, hwndGame);
            }

            static std::unique_ptr<ImeManagerComposer> g_instance;
        };
    }
}