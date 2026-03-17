//
// Created by jamie on 2025/2/22.
//
#include "core/State.h"
#include "log.h"
#include "tsf/TextStore.h"
#include "tsf/TsfSupport.h"

namespace Tsf
{
auto TextService::Initialize(ITfThreadMgr *threadMgr, TfClientId clientId) -> HRESULT
{
    HRESULT hr = threadMgr->QueryInterface(__uuidof(ITfThreadMgr), reinterpret_cast<void **>(&m_threadMgr));
    if (FAILED(hr))
    {
        logger::error("Can't get ITfThreadMgr: {:#X}", static_cast<uint32_t>(hr));
        return hr;
    }
    m_clientId                     = clientId;
    m_textStore                    = new TextStore(this);
    m_conversionModeCompartment    = new TsfCompartment();
    m_keyboardOpenCloseCompartment = new TsfCompartment();

    hr = m_conversionModeCompartment->Initialize(
        m_threadMgr, clientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, [](const GUID * /*guid*/, const ULONG ulong) {
            UpdateConversionMode(ulong);
            return S_OK;
        }
    );

    if (SUCCEEDED(hr))
    {
        hr = m_keyboardOpenCloseCompartment->Initialize(
            m_threadMgr, clientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, [](const GUID *, const ULONG ulong) {
                State::GetInstance().Set(State::KEYBOARD_OPEN, ulong != 0);
                return S_OK;
            }
        );
    }
    if (SUCCEEDED(hr))
    {
        hr = m_textStore->Initialize(threadMgr, clientId);
    }

    return hr;
}

void TextService::UpdateConversionMode() const
{
    ULONG conversionMode = 0;
    if (SUCCEEDED(m_conversionModeCompartment->GetValue(conversionMode)))
    {
        UpdateConversionMode(conversionMode);
    }
}

void TextService::UpdateConversionMode(const ULONG conversionMode)
{
    State::GetInstance().GetConversionMode().Set(conversionMode);
}

auto TextService::OnFocus(bool focus) -> bool
{
    HRESULT hr = E_FAIL;
    if (focus)
    {
        hr = m_textStore->Focus();
        UpdateConversionMode();
    }
    else
    {
        hr = m_textStore->ClearFocus();
    }
    m_keyboardOpenCloseCompartment->SetValue(static_cast<DWORD>(focus)); // true -> open, false -> close
    const auto succeeded = SUCCEEDED(hr);
    if (succeeded)
    {
        State::GetInstance().Set(State::TEXT_SERVICE_FOCUS, focus);
    }
    return succeeded;
}

auto TextService::SetConversionMode(DWORD conversionMode) -> bool
{
    return SUCCEEDED(m_conversionModeCompartment->SetValue(conversionMode));
}

auto TextService::ProcessImeMessage(HWND /*hWnd*/, UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) -> bool
{
    // This fallback is practically unnecessary.
    // If the current environment supports TSF APIs but fails to provide a Candidate UI (UIElement),
    // it is likely a severe system-level anomaly rather than a supported configuration.
    //
    // Logic: Our architecture already separates concerns by initializing a legacy Imm32
    // TextService if TSF is disabled or fails to initialize. Therefore, within the
    // TSF-enabled service, we assume UIElement support is guaranteed.
    // switch (uMsg)
    // {
    //     case WM_IME_NOTIFY:
    //         if (!m_pTextStore->IsSupportCandidateUi())
    //         {
    //             if (m_fallbackTextService.ProcessImeMessage(hWnd, uMsg, wParam, lParam))
    //             {
    //                 const std::unique_lock lock(m_mutex);
    //                 m_candidateUi = m_fallbackTextService.GetCandidateUi();
    //                 MarkDirty();
    //             }
    //         }
    //         break;
    //     default:
    //         break;
    // }
    return false;
}
} // namespace Tsf
