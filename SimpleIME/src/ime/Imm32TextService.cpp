//
// Created by jamie on 2025/2/21.
//

#include "common/WCharUtils.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "ime/ITextService.h"

#include <imm.h>

#pragma comment(lib, "imm32.lib")

namespace LIBC_NAMESPACE_DECL
{
namespace Ime::Imm32
{
void Imm32TextService::OnStartComposition()
{
    State::GetInstance().Set(State::IN_COMPOSING);
}

void Imm32TextService::OnEndComposition()
{
    State::GetInstance().Clear(State::IN_COMPOSING);
    if (m_OnEndCompositionCallback != nullptr)
    {
        m_OnEndCompositionCallback(m_textEditor.GetText());
    }
    m_textEditor.Select(0, 0);
    m_textEditor.ClearText();

    if (State::GetInstance().Has(State::IN_CAND_CHOOSING))
    {
        CloseCandidate();
    }
}

auto Imm32TextService::ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool
{
    switch (message)
    {
        case WM_CREATE:
            m_hIMC = ImmCreateContext();
            break;
        case WM_DESTROY: {
            ImmAssociateContext(hWnd, nullptr);
            ImmDestroyContext(m_hIMC);
            break;
        }
        case WM_IME_STARTCOMPOSITION:
            OnStartComposition();
            return true;
        case WM_IME_ENDCOMPOSITION:
            OnEndComposition();
            return true;
        case CM_IME_COMPOSITION:
        case WM_IME_COMPOSITION:
            OnComposition(hWnd, lParam);
            return true;
        case WM_IME_NOTIFY: {
            if (ImeNotify(hWnd, wParam, lParam))
            {
                return true;
            }
            break;
        }
        default:
            break;
    }

    return false;
}

bool Imm32TextService::OnFocus(bool focus)
{
    if (focus)
    {
        if (!m_hIMC)
        {
            m_hIMC = ImmCreateContext();
        }
        ImmAssociateContext(m_imeHwnd, m_hIMC);
    }
    else
    {
        m_hIMC = ImmAssociateContext(m_imeHwnd, nullptr);
    }
    return ITextService::OnFocus(focus);
}

auto Imm32TextService::CommitCandidate(HWND hwnd, UINT index) -> bool
{
    HIMC hImc   = ImmGetContext(hwnd);
    bool result = ImmNotifyIME(hImc, NI_SELECTCANDIDATESTR, 0, index) != FALSE;
    ImmReleaseContext(hwnd, hImc);
    return result;
}

void Imm32TextService::OnComposition(HWND hWnd, LPARAM compFlag)
{
    HIMC hIMC = ImmGetContext(hWnd);
    if (hIMC == nullptr)
    {
        return;
    }

    std::wstring compositionSting;
    if (GetCompStr(hIMC, compFlag, GCS_RESULTSTR, compositionSting))
    {
        m_textEditor.SelectAll();
        m_textEditor.InsertText(compositionSting.c_str(), compositionSting.length());
        if (spdlog::should_log(spdlog::level::trace))
        {
            const auto str = WCharUtils::ToString(compositionSting);
            log_trace("IME Composition Result String: {}", str.c_str());
        }
    }
    else if (GetCompStr(hIMC, compFlag, GCS_COMPSTR, compositionSting))
    {
        const long cursorPos  = ImmGetCompositionStringW(hIMC, GCS_CURSORPOS, nullptr, 0);
        const long deltaStart = ImmGetCompositionStringW(hIMC, GCS_DELTASTART, nullptr, 0);
        UpdateComposition(compositionSting, cursorPos, deltaStart);
    }
    ImmReleaseContext(hWnd, hIMC);
}

void Imm32TextService::UpdateComposition(const std::wstring &compStr, long cursorPos, long deltaStart)
{
    if (cursorPos < 0) cursorPos = 0;
    if (deltaStart < 0) deltaStart = 0;

    const long prevSize = static_cast<long>(m_textEditor.GetTextSize());
    if (deltaStart > prevSize) deltaStart = prevSize;

    m_textEditor.Select(deltaStart, prevSize);

    const auto length = static_cast<long>(compStr.length());
    if (deltaStart > length) deltaStart = length;
    m_textEditor.InsertText(compStr.c_str() + deltaStart, length - deltaStart);

    cursorPos = std::min(cursorPos, static_cast<long>(m_textEditor.GetTextSize()));
    m_textEditor.Select(cursorPos, cursorPos);

    if (spdlog::should_log(spdlog::level::trace))
    {
        const auto str = WCharUtils::ToString(compStr);
        log_trace("IME Composition String: {}", str.c_str());
    }
}

auto Imm32TextService::GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, std::wstring &pWcharBuf) -> bool
{
    if ((compFlag & flagToCheck) != 0)
    {
        LONG bufLenInBytes = ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), nullptr, 0);
        if (bufLenInBytes > 0)
        {
            pWcharBuf.resize(bufLenInBytes / sizeof(WCHAR) + 1);
            ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), pWcharBuf.data(), bufLenInBytes);
            return true;
        }
    }
    return false;
}

auto Imm32TextService::ImeNotify(const HWND hWnd, WPARAM wParam, LPARAM lParam) -> bool
{
    // log_debug("ImeNotify {:#x}, {:#x}", wParam, lParam);
    switch (wParam)
    {
        case IMN_SETCANDIDATEPOS:
        case IMN_OPENCANDIDATE: {
            State::GetInstance().Set(State::IN_CAND_CHOOSING);
            HIMC hImc = ImmGetContext(hWnd);
            if (hImc != nullptr)
            {
                OpenCandidate(hImc);
            }
            break;
        }
        case IMN_CLOSECANDIDATE:
            CloseCandidate();
            break;
        case IMN_CHANGECANDIDATE: {
            HIMC hIMC = ImmGetContext(hWnd);
            if (hIMC != nullptr)
            {
                ChangeCandidate(hIMC);
                ImmReleaseContext(hWnd, hIMC);
            }
            break;
        }
        case IMN_SETCONVERSIONMODE: {
            HIMC hIMC = ImmGetContext(hWnd);
            if (hIMC != nullptr)
            {
                UpdateConversionMode(hIMC);
                ImmReleaseContext(hWnd, hIMC);
            }
            break;
        }
        case IMN_SETOPENSTATUS: {
            HIMC hIMC = ImmGetContext(hWnd);
            if (hIMC != nullptr)
            {
                OnSetOpenStatus(hIMC);
                ImmReleaseContext(hWnd, hIMC);
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void Imm32TextService::OpenCandidate(HIMC hIMC)
{
    ChangeCandidateAt(hIMC);
}

void Imm32TextService::CloseCandidate()
{
    State::GetInstance().Clear(State::IN_CAND_CHOOSING);
    m_candidateUi.Close();
}

void Imm32TextService::ChangeCandidate(const HIMC hIMC)
{
    ChangeCandidateAt(hIMC);
}

void Imm32TextService::ChangeCandidateAt(const HIMC hIMC)
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
        m_candidateUi.Close();
        return;
    }
    ImmGetCandidateListW(hIMC, 0, lpCandList, bufLen);
    DoUpdateCandidateList(lpCandList);
    GlobalUnlock(hGlobal);
    GlobalFree(hGlobal);
}

void Imm32TextService::DoUpdateCandidateList(const LPCANDIDATELIST lpCandList)
{
    DWORD dwStartIndex = lpCandList->dwPageStart;
    DWORD dwEndIndex   = dwStartIndex + lpCandList->dwPageSize;
    dwEndIndex         = std::min(dwEndIndex, lpCandList->dwCount);

    m_candidateUi.Close();
    m_candidateUi.SetPageSize(lpCandList->dwPageSize);
    auto *lpCandListByte = reinterpret_cast<LPCH>(lpCandList);
    for (DWORD index = 0; dwStartIndex < dwEndIndex; ++index, ++dwStartIndex)
    {
        auto        pcCandidate  = lpCandListByte + lpCandList->dwOffset[dwStartIndex];
        auto       *pwcCandidate = reinterpret_cast<LPWCH>(pcCandidate);
        auto        sizeInBytes  = WCharUtils::RequiredByteLength(pwcCandidate);
        std::string ansiStr(sizeInBytes, 0);
        WCharUtils::ToString(pwcCandidate, ansiStr.data(), sizeInBytes);
        ansiStr.insert_range(ansiStr.begin(), std::format("{}. ", index + 1));
        m_candidateUi.PushBack(ansiStr);
    }
    m_candidateUi.SetSelection(lpCandList->dwSelection);
}

void Imm32TextService::OnSetOpenStatus(const HIMC hIMC)
{
    if (ImmGetOpenStatus(hIMC) != 0)
    {
        State::GetInstance().Set(State::IME_OPEN);
        UpdateConversionMode(hIMC);
    }
    else
    {
        State::GetInstance().Clear(State::IME_OPEN);
    }
}

void Imm32TextService::UpdateConversionMode(HIMC hIMC)
{
    DWORD conversion = 0;
    DWORD sentence   = 0;
    if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
    {
        switch (conversion & IME_CMODE_LANGUAGE)
        {
            case IME_CMODE_ALPHANUMERIC:
                State::GetInstance().Clear(State::IN_ALPHANUMERIC);
                log_debug("CMODE:ALPHANUMERIC, Disable IME");
                break;
            default:
                State::GetInstance().Clear(State::IN_ALPHANUMERIC);
                break;
        }
    }
}
}
}