#pragma once

#include "ime/ImeManager.h"
#include "ime/PermanentFocusImeManager.h"
#include "ime/TemporaryFocusImeManager.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        enum FocusType
        {
            Permanent = 0,
            Temporary
        };

        class ImeManagerComposer : public ImeManager
        {
        public:
            ImeManagerComposer(ImeWnd *pImeWnd, HWND hwndGame)
                : m_PermanentFocusImeManager(std::make_unique<PermanentFocusImeManager>(pImeWnd, hwndGame)),
                  m_temporaryFocusImeManager(std::make_unique<TemporaryFocusImeManager>(pImeWnd, hwndGame))
            {
            }

            ~ImeManagerComposer() override = default;

        private:
            auto Use(FocusType type)
            {
                if (FocusType::Permanent == type)
                {
                    m_delegate = m_PermanentFocusImeManager.get();
                }
                else
                {
                    m_delegate = m_temporaryFocusImeManager.get();
                }
            }

        public:
            auto PushType(FocusType type)
            {
                bool diff = !m_FocusTypeStack.empty() && type != m_FocusTypeStack.top();
                if (diff)
                {
                    m_delegate->WaitEnableIme(false);
                }
                m_FocusTypeStack.push(type);
                Use(type);
                if (diff)
                {
                    m_delegate->SyncImeState();
                }
            }

            auto PopType()
            {
                if (m_FocusTypeStack.empty())
                {
                    log_warn("Invalid call! Focus type stack is empty.");
                    return;
                }
                auto prev = m_FocusTypeStack.top();
                m_FocusTypeStack.pop();
                if (m_FocusTypeStack.empty())
                {
                    /*log_warn("Current focus type stack is empty, set fall back type to Permanent");
                    PushType(FocusType::Permanent);*/
                    m_delegate = nullptr;
                }
                else if (prev != m_FocusTypeStack.top())
                {
                    m_delegate->WaitEnableIme(false);
                    Use(m_FocusTypeStack.top());
                    m_delegate->SyncImeState();
                }
            }

            constexpr auto GetFocusManageType() const -> const FocusType &
            {
                return m_FocusTypeStack.top();
            }

            auto GetTemporaryFocusImeManager() -> TemporaryFocusImeManager *
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

            auto WaitEnableIme(bool enable) const -> bool override
            {
                return m_delegate->WaitEnableIme(enable);
            }

            auto WaitEnableMod(bool enable) const -> bool override
            {
                return m_delegate->WaitEnableMod(enable);
            }

            static auto GetInstance() -> ImeManagerComposer *
            {
                return g_instance.get();
            }

        private:
            std::unique_ptr<PermanentFocusImeManager> m_PermanentFocusImeManager = nullptr;
            std::unique_ptr<TemporaryFocusImeManager> m_temporaryFocusImeManager = nullptr;
            ImeManager                               *m_delegate                 = nullptr;
            std::stack<FocusType>                     m_FocusTypeStack{};

            friend class ImeWnd;

            static void Init(ImeWnd *imwWnd, HWND hwndGame)
            {
                g_instance = std::make_unique<ImeManagerComposer>(imwWnd, hwndGame);
            }

            static std::unique_ptr<ImeManagerComposer> g_instance;
        };
    }
}