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
auto Imm32TextService::OnStartComposition() -> HRESULT
{
    State::GetInstance().Set(State::IN_COMPOSING);
    return S_OK;
}

auto Imm32TextService::OnEndComposition() -> HRESULT
{
    State::GetInstance().Clear(State::IN_COMPOSING);
    if (m_OnEndCompositionCallback != nullptr)
    {
        m_OnEndCompositionCallback(m_textEditor.GetText());
    }
    m_textEditor.Select(0, 0);
    m_textEditor.ClearText();
    return S_OK;
}

auto Imm32TextService::ProcessImeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> bool
{
    switch (message)
    {
        case WM_IME_STARTCOMPOSITION: {
            return SUCCEEDED(OnStartComposition());
        }
        case WM_IME_ENDCOMPOSITION: {
            return SUCCEEDED(OnEndComposition());
        }
        case CM_IME_COMPOSITION:
        case WM_IME_COMPOSITION: {
            if (SUCCEEDED(OnComposition(hWnd, lParam)))
            {
                return true;
            }
            break;
        }
        case WM_IME_NOTIFY: {
            if (ImeNotify(hWnd, wParam, lParam))
            {
                return true;
            }
            break;
        }
        case WM_IME_SETCONTEXT:
            return DefWindowProcW(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
        default:
            break;
    }

    return true;
}

auto Imm32TextService::CommitCandidate(HWND hwnd, UINT index)  -> bool
{
    HIMC hImc = ImmGetContext(hwnd);
    bool result = ImmNotifyIME(hImc, NI_SELECTCANDIDATESTR, 0, index) != FALSE;
    ImmReleaseContext(hwnd, hImc);
    return result;
}

auto Imm32TextService::OnComposition(HWND hWnd, LPARAM compFlag) -> HRESULT
{
    HIMC hIMC = ImmGetContext(hWnd);
    if (hIMC == nullptr)
    {
        return E_FAIL;
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
        if (spdlog::should_log(spdlog::level::trace))
        {
            const auto str = WCharUtils::ToString(compositionSting);
            log_trace("IME Composition String: {}", str.c_str());
        }
        m_textEditor.SelectAll();
        m_textEditor.InsertText(compositionSting.c_str(), compositionSting.length());
    }
    ImmReleaseContext(hWnd, hIMC);
    return S_OK;
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
    log_trace("ImeNotify {:#x}, {:#x}", wParam, lParam);
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
            return true;
        }
        case IMN_CLOSECANDIDATE: {
            State::GetInstance().Clear(State::IN_CAND_CHOOSING);
            CloseCandidate();
            return true;
        }
        case IMN_CHANGECANDIDATE: {
            HIMC hIMC = ImmGetContext(hWnd);
            if (hIMC != nullptr)
            {
                ChangeCandidate(hIMC);
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

void Imm32TextService::OpenCandidate(HIMC hIMC)
{
    ChangeCandidateAt(hIMC);
}

void Imm32TextService::CloseCandidate()
{
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
    m_candidateUi.SetPageSize(lpCandList->dwPageSize);

    DWORD dwStartIndex = lpCandList->dwPageStart;
    DWORD dwEndIndex   = dwStartIndex + m_candidateUi.PageSize();
    dwEndIndex         = std::min(dwEndIndex, lpCandList->dwCount);

    m_candidateUi.Close();

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