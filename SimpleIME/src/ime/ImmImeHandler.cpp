#include "ime/ImmImeHandler.h"
#include "common/WCharUtils.h"
#include "ime/ime.h"

#include <imm.h>

namespace LIBC_NAMESPACE_DECL
{
    auto Ime::ImmImeHandler::ImeNotify(const HWND hWnd, WPARAM wParam, LPARAM lParam) const -> bool
    {
        log_trace("ImeNotify {:#x}, {:#x}", wParam, lParam);
        switch (wParam)
        {
            case IMN_SETCANDIDATEPOS:
            case IMN_OPENCANDIDATE: {
                m_pInputMethod->SetState(InputMethod::State::IN_CANDCHOOSEN);
                HIMC hImc = ImmGetContext(hWnd);
                if (hImc != nullptr)
                {
                    OpenCandidate(hImc, lParam);
                }
                return true;
            }
            case IMN_CLOSECANDIDATE: {
                m_pInputMethod->ClearState(InputMethod::State::IN_CANDCHOOSEN);
                CloseCandidate(lParam);
                return true;
            }
            case IMN_CHANGECANDIDATE: {
                HIMC hIMC = ImmGetContext(hWnd);
                if (hIMC != nullptr)
                {
                    ChangeCandidate(hIMC, lParam);
                    ImmReleaseContext(hWnd, hIMC);
                }
                return true;
            }
            case IMN_SETCONVERSIONMODE: {
                HIMC hIMC = ImmGetContext(hWnd);
                if (hIMC != nullptr)
                {
                    UpdateConversionMode(hIMC);
                    ImmReleaseContext(hWnd, hIMC);
                }
                return true;
            }
            case IMN_SETOPENSTATUS: {
                HIMC hIMC = ImmGetContext(hWnd);
                if (hIMC != nullptr)
                {
                    OnSetOpenStatus(hIMC);
                    ImmReleaseContext(hWnd, hIMC);
                }
                return true;
            }
            default:
                break;
        }
        return false;
    }

    void Ime::ImmImeHandler::OpenCandidate(HIMC hIMC, LPARAM candListFlag) const
    {
        ChangeCandidateAt(hIMC);
    }

    void Ime::ImmImeHandler::CloseCandidate(LPARAM candListFlag) const
    {
        m_pInputMethod->CloseCandidateUi();
    }

    void Ime::ImmImeHandler::ChangeCandidate(const HIMC hIMC, LPARAM candListFlag) const
    {
        ChangeCandidateAt(hIMC);
    }

    void Ime::ImmImeHandler::ChangeCandidateAt(const HIMC hIMC) const
    {
        DWORD bufLen = ImmGetCandidateListW(hIMC, 0, nullptr, 0);
        if (bufLen == 0)
        {
            return;
        }
        HGLOBAL hGlobal = GlobalAlloc(LPTR, bufLen);
        if (hGlobal == nullptr)
        {
            log_warn("Global alloc {} failed.", bufLen);
            return;
        }
        LPCANDIDATELIST lpCandList = static_cast<LPCANDIDATELIST>(GlobalLock(hGlobal));
        if (lpCandList == nullptr)
        {
            log_error("Candidate alloc memory failed.");
            GlobalFree(hGlobal);
            m_pInputMethod->CloseCandidateUi();
            return;
        }
        ImmGetCandidateListW(hIMC, 0, lpCandList, bufLen);
        DoUpdateCandidateList(lpCandList);
        GlobalUnlock(hGlobal);
        GlobalFree(hGlobal);
    }

    void Ime::ImmImeHandler::DoUpdateCandidateList(const LPCANDIDATELIST lpCandList) const
    {
        auto &candidateUi = m_pInputMethod->GetCandidateUi();
        candidateUi.SetPageSize(lpCandList->dwPageSize);

        DWORD dwStartIndex = lpCandList->dwPageStart;
        DWORD dwEndIndex   = dwStartIndex + candidateUi.PageSize();
        dwEndIndex         = std::min(dwEndIndex, lpCandList->dwCount);

        candidateUi.Close();

        auto *lpCandListByte = reinterpret_cast<LPCH>(lpCandList);
        for (DWORD index = 0; dwStartIndex < dwEndIndex; ++index, ++dwStartIndex)
        {
            auto        pcCandidate  = lpCandListByte + lpCandList->dwOffset[dwStartIndex];
            auto       *pwcCandidate = reinterpret_cast<LPWCH>(pcCandidate);
            auto        sizeInBytes  = WCharUtils::RequiredByteLength(pwcCandidate);
            std::string ansiStr(sizeInBytes, 0);
            WCharUtils::ToString(pwcCandidate, ansiStr.data(), sizeInBytes);
            ansiStr.insert_range(ansiStr.begin(), std::format("{}. ", index + 1));
            candidateUi.PushBack(ansiStr);
        }
        candidateUi.SetSelection(lpCandList->dwSelection);
    }

    void Ime::ImmImeHandler::OnSetOpenStatus(const HIMC hIMC) const
    {
        if (ImmGetOpenStatus(hIMC) != 0)
        {
            m_pInputMethod->SetState(InputMethod::State::OPEN);
            UpdateConversionMode(hIMC);
        }
        else
        {
            m_pInputMethod->ClearState(InputMethod::State::OPEN);
        }
    }

    void Ime::ImmImeHandler::UpdateConversionMode(const HIMC hIMC) const
    {
        DWORD conversion = 0;
        DWORD sentence   = 0;
        if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
        {
            switch (conversion & IME_CMODE_LANGUAGE)
            {
                case IME_CMODE_ALPHANUMERIC:
                    m_pInputMethod->SetState(InputMethod::State::IN_ALPHANUMERIC);
                    log_debug("CMODE:ALPHANUMERIC, Disable IME");
                    break;
                default:
                    m_pInputMethod->ClearState(InputMethod::State::IN_ALPHANUMERIC);
                    break;
            }
        }
    }
}