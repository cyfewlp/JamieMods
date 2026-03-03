//
// Created by jamie on 2025/2/21.
//

#include "WCharUtils.h"
#include "configs/CustomMessage.h"
#include "ime/ITextService.h"
#include "log.h"

#include <algorithm>
#include <imm.h>

#pragma comment(lib, "imm32.lib")

namespace Ime::Imm32
{
using State = Ime::Core::State;

namespace
{

void UpdateConversionMode(HIMC hIMC)
{
    DWORD conversion = 0;
    DWORD sentence   = 0;
    if (ImmGetConversionStatus(hIMC, &conversion, &sentence) != 0)
    {
        switch (conversion & IME_CMODE_LANGUAGE)
        {
            case IME_CMODE_ALPHANUMERIC:
                State::GetInstance().Clear(State::IN_ALPHANUMERIC);
                logger::debug("CMODE:ALPHANUMERIC, Disable IME");
                break;
            default:
                State::GetInstance().Clear(State::IN_ALPHANUMERIC);
                break;
        }
    }
}

void OnSetOpenStatus(HIMC hIMC)
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

auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, std::wstring &pWcharBuf) -> bool
{
    if ((compFlag & flagToCheck) != 0)
    {
        const LONG bufLenInBytes = ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), nullptr, 0);
        if (bufLenInBytes > 0)
        {
            const auto dwBufLenInBytes = static_cast<DWORD>(bufLenInBytes);
            pWcharBuf.resize(dwBufLenInBytes / sizeof(WCHAR));
            const LONG written = ImmGetCompositionStringW(hIMC, static_cast<DWORD>(flagToCheck), pWcharBuf.data(), dwBufLenInBytes);
            if (written > 0)
            {
                if (written != bufLenInBytes)
                {
                    pWcharBuf.resize(static_cast<size_t>(written) / sizeof(WCHAR));
                }
                return true;
            }
        }
    }
    pWcharBuf.clear();
    return false;
}

} // namespace

void Imm32TextService::OnStartComposition()
{
    State::GetInstance().Set(State::IN_COMPOSING);
}

void Imm32TextService::OnEndComposition()
{
    if (m_OnEndCompositionCallback != nullptr)
    {
        m_OnEndCompositionCallback(m_textEditor.GetText());
    }
    m_textEditor.Select(0, 0);
    m_textEditor.ClearText();
    {
        std::lock_guard lock(m_mutex);
        m_candidateUi.Close();
    }
    State::GetInstance().Clear(State::IN_COMPOSING);
    State::GetInstance().Clear(State::IN_CAND_CHOOSING);
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
            OnImeNotify(hWnd, wParam, lParam);
            break;
        }
        default:
            break;
    }

    return false;
}

auto Imm32TextService::OnFocus(bool focus) -> bool
{
    if (focus)
    {
        if (m_hIMC == nullptr)
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

// This method does not work as expected
auto Imm32TextService::CommitCandidate(DWORD index) -> bool
{
    logger::debug("CommitCandidate {}", index);
    HIMC hImc = ImmGetContext(m_imeHwnd);

    bool result = true;
    if (hImc != nullptr)
    {
        result = ImmNotifyIME(hImc, NI_SELECTCANDIDATESTR, 0, index) != FALSE;
    }

    ImmReleaseContext(m_imeHwnd, hImc);
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
        m_textEditor.InsertText(compositionSting);
        if (spdlog::should_log(spdlog::level::trace))
        {
            const auto str = WCharUtils::ToString(compositionSting);
            logger::trace("IME Composition Result String: {}", str.c_str());
        }
    }
    else if (GetCompStr(hIMC, compFlag, GCS_COMPSTR, compositionSting))
    {
        const int32_t cursorPos  = ImmGetCompositionStringW(hIMC, GCS_CURSORPOS, nullptr, 0);
        const int32_t deltaStart = ImmGetCompositionStringW(hIMC, GCS_DELTASTART, nullptr, 0);
        // IMM_ERROR_NODATA or IMM_ERROR_GENERAL
        if (cursorPos == IMM_ERROR_GENERAL || deltaStart == IMM_ERROR_GENERAL)
        {
            logger::error("Get composition cursor position or delta start failed.");
            return;
        }
        if (cursorPos >= 0 && deltaStart >= 0)
        {
            UpdateComposition(compositionSting, static_cast<size_t>(cursorPos), static_cast<size_t>(deltaStart));
        }
    }
    ImmReleaseContext(hWnd, hIMC);
}

void Imm32TextService::UpdateComposition(const std::wstring &compStr, size_t cursorPos, size_t deltaStart)
{
    const auto prevSize = m_textEditor.GetTextSize();

    deltaStart = std::clamp(deltaStart, 0LLU, prevSize);

    m_textEditor.Select(static_cast<int32_t>(deltaStart), -1);

    const auto length = compStr.length();
    deltaStart        = std::min(deltaStart, length);
    m_textEditor.InsertText(compStr.substr(deltaStart));

    cursorPos = std::clamp(cursorPos, 0LLU, m_textEditor.GetTextSize());
    m_textEditor.Select(static_cast<int32_t>(cursorPos), static_cast<int32_t>(cursorPos));

    if (spdlog::should_log(spdlog::level::trace))
    {
        const auto str = WCharUtils::ToString(compStr);
        logger::trace("IME Composition String: {}", str.c_str());
    }
}

auto Imm32TextService::OnImeNotify(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/) -> void
{
    // logger::debug("ImeNotify {:#x}, {:#x}", wParam, lParam);
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
            State::GetInstance().Clear(State::IN_CAND_CHOOSING);
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
}

void Imm32TextService::OpenCandidate(HIMC hIMC)
{
    ChangeCandidateAt(hIMC);
}

void Imm32TextService::ChangeCandidate(HIMC hIMC)
{
    ChangeCandidateAt(hIMC);
}

void Imm32TextService::ChangeCandidateAt(HIMC hIMC)
{
    DWORD bufLen = ImmGetCandidateListW(hIMC, 0, nullptr, 0);
    if (bufLen == 0)
    {
        return;
    }
    HGLOBAL hGlobal = GlobalAlloc(LPTR, bufLen);
    if (hGlobal == nullptr)
    {
        logger::warn("Global alloc {} failed.", bufLen);
        return;
    }
    auto *lpCandList = static_cast<LPCANDIDATELIST>(GlobalLock(hGlobal));
    if (lpCandList == nullptr)
    {
        logger::error("Candidate alloc memory failed.");
        GlobalFree(hGlobal);
        return;
    }
    bufLen = ImmGetCandidateListW(hIMC, 0, lpCandList, bufLen);
    if (bufLen > 0)
    {
        DoUpdateCandidateList(lpCandList);
    }
    GlobalUnlock(hGlobal);
    GlobalFree(hGlobal);
}

void Imm32TextService::DoUpdateCandidateList(LPCANDIDATELIST lpCandList)
{
    DWORD dwStartIndex = lpCandList->dwPageStart;
    DWORD dwEndIndex   = dwStartIndex + lpCandList->dwPageSize;
    dwEndIndex         = std::min(dwEndIndex, lpCandList->dwCount);

    std::lock_guard lock(m_mutex);
    m_candidateUi.Close();
    m_candidateUi.Reserve(lpCandList->dwPageSize);
    auto *lpCandListByte = reinterpret_cast<LPCH>(lpCandList);
    for (DWORD index = 0; dwStartIndex < dwEndIndex; ++index, ++dwStartIndex)
    {
        auto       *pcCandidate  = lpCandListByte + lpCandList->dwOffset[dwStartIndex];
        auto       *pwcCandidate = reinterpret_cast<LPWCH>(pcCandidate);
        std::string ansiStr      = WCharUtils::ToString(pwcCandidate, -1);
        ansiStr.insert_range(ansiStr.begin(), std::format("{}. ", index + 1));
        m_candidateUi.PushBack(ansiStr);
    }
    m_candidateUi.SetSelection(lpCandList->dwSelection);
    MarkDirty(DirtyFlag::CandidateList);
}

} // namespace Ime::Imm32
