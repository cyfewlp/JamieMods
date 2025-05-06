//
// Created by jamie on 2025/5/6.
//
#include "ime/ImeManagerComposer.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        auto ImeManagerComposer::Use(FocusType type)
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

        void ImeManagerComposer::PushType(FocusType type, bool syncImeState)
        {
            assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
            const bool diff = m_FocusTypeStack.empty() || type != m_FocusTypeStack.top();
            if (diff)
            {
                m_fDirty = true;
                if (m_delegate != nullptr && !m_delegate->WaitEnableIme(false))
                {
                    ErrorNotifier::GetInstance().addError("Unexpected error: WaitEnableIme(false) failed.");
                }
            }
            m_FocusTypeStack.push(type);
            Use(type);
            if (diff && syncImeState)
            {
                SyncImeStateIfDirty();
            }
        }

        void ImeManagerComposer::PopType(bool syncImeState)
        {
            assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
            if (m_FocusTypeStack.empty())
            {
                ErrorNotifier::GetInstance().addError("Invalid call! Focus type stack is empty.");
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
                m_fDirty = true;
                if (!m_delegate->WaitEnableIme(false))
                {
                    ErrorNotifier::GetInstance().addError("Unexpected error: WaitEnableIme(false) failed.");
                }
                Use(m_FocusTypeStack.top());
                if (syncImeState)
                {
                    SyncImeStateIfDirty();
                }
            }
        }

        void ImeManagerComposer::PopAndPushType(const FocusType type, const bool syncImeState)
        {
            assert(!m_fDirty && "WARNING: Missing call SyncImeState?");
            if (m_FocusTypeStack.empty())
            {
                ErrorNotifier::GetInstance().addError("Invalid call! Focus type stack is empty.");
                return;
            }
            if (const auto prev = m_FocusTypeStack.top(); prev == type)
            {
                if (syncImeState)
                {
                    SyncImeStateIfDirty();
                }
                return;
            }
            m_fDirty = true;
            if (!m_delegate->WaitEnableIme(false))
            {
                ErrorNotifier::GetInstance().addError("Unexpected error: WaitEnableIme(false) failed.");
            }
            m_FocusTypeStack.pop();
            m_FocusTypeStack.push(type);
            Use(type);

            if (syncImeState)
            {
                SyncImeStateIfDirty();
            }
        }

        auto ImeManagerComposer::EnableIme(bool enable) -> bool
        {
            if (m_fSupportOtherMod)
            {
                return m_delegate->EnableIme(enable);
            }
            return m_delegate->EnableIme(m_fKeepImeOpen || enable);
        }

        auto ImeManagerComposer::EnableMod(bool enable) -> bool
        {
            const bool prev = IsModEnabled();
            SetEnableMod(enable);
            if (m_delegate->EnableMod(enable))
            {
                return true;
            }
            SetEnableMod(prev);
            return false;
        }
    }
}