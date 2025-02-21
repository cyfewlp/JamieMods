#pragma once

#include "InputMethod.h"
#include "TextEditor.h"
#include "configs/Configs.h"

#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImmImeHandler
        {
        public:
            explicit ImmImeHandler(InputMethod *pInputMethod)
                : m_pInputMethod(pInputMethod), m_pTsfEditor(pInputMethod->GetTextEditor())
            {
            }

            auto ImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) const -> bool;

        private:
            void         OpenCandidate(HIMC hIMC, LPARAM candListFlag) const;
            void         CloseCandidate(LPARAM candListFlag) const;
            void         ChangeCandidate(HIMC hIMC, LPARAM candListFlag) const;
            void         ChangeCandidateAt(HIMC hIMC) const;
            void         DoUpdateCandidateList(LPCANDIDATELIST lpCandList) const;
            void         OnSetOpenStatus(HIMC hIMC) const;
            void         UpdateConversionMode(HIMC hIMC) const;

            InputMethod *m_pInputMethod = nullptr;
            TextEditor  *m_pTsfEditor   = nullptr;
        };
    }
}