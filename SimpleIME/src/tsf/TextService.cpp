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
    }
}