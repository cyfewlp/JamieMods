//
// Created by jamie on 2025/2/22.
//
#include "tsf/TextStore.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        auto TextService::ProcessImeMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> bool
        {
            if (uMsg == WM_IME_NOTIFY && wParam == IMN_SETCONVERSIONMODE)
            {
                if (HIMC const hIMC = ImmGetContext(hWnd); hIMC != nullptr)
                {
                    DWORD conversion = 0;
                    DWORD sentence   = 0;
                    if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
                    {
                        if ((conversion & IME_CMODE_LANGUAGE) == IME_CMODE_ALPHANUMERIC)
                        {
                            SetState(Ime::ImeState::IN_ALPHANUMERIC);
                        }
                        else
                        {
                            ClearState(Ime::ImeState::IN_ALPHANUMERIC);
                        }
                    }
                    ImmReleaseContext(hWnd, hIMC);
                }
            }
            if (!m_pTextStore->IsSupportCandidateUi())
            {
                if (m_fallbackTextService.ProcessImeMessage(hWnd, uMsg, wParam, lParam))
                {
                    const auto &candidateUi = m_fallbackTextService.GetCandidateUi();
                    m_candidateUi.Close();
                    for (const auto &candidate : candidateUi.CandidateList())
                    {
                        m_candidateUi.PushBack(candidate);
                    }
                    m_candidateUi.SetSelection(candidateUi.Selection());
                    m_candidateUi.SetPageSize(candidateUi.PageSize());
                    return true;
                }
            }
            return false;
        }
    } // namespace Tsf
} // namespace LIBC_NAMESPACE_DECL