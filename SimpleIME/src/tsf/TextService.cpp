//
// Created by jamie on 2025/2/22.
//
#include "core/State.h"
#include "log.h"
#include "tsf/TextStore.h"
#include "tsf/TsfSupport.h"

namespace Tsf
{
HRESULT TextService::Initialize()
{
    m_pTextStore           = new TextStore(this);
    auto const &tsfSupport = TsfSupport::GetSingleton();
    // callback
    auto        callback   = [](const GUID          */*guid*/, const ULONG ulong) {
        DoUpdateConversionMode(ulong);
        return S_OK;
    };
    m_pCompartment         = new TsfCompartment();
    m_pCompartmentKeyBoard = new TsfCompartment();
    HRESULT hresult        = m_pCompartment->Initialize(tsfSupport.GetThreadMgr(), GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, callback);

    if (SUCCEEDED(hresult))
    {
        hresult =
            m_pCompartmentKeyBoard->Initialize(tsfSupport.GetThreadMgr(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, [](const GUID *, const ULONG ulong) {
                State::GetInstance().Set(State::KEYBOARD_OPEN, ulong != 0);
                return S_OK;
            });
    }
    if (SUCCEEDED(hresult))
    {
        hresult = m_pTextStore->Initialize(tsfSupport.GetThreadMgr(), tsfSupport.GetTfClientId());
    }

    return hresult;
}

void TextService::UpdateConversionMode() const
{
    ULONG convertionMode = 0;
    if (SUCCEEDED(m_pCompartment->GetValue(convertionMode)))
    {
        DoUpdateConversionMode(convertionMode);
    }
}

void TextService::DoUpdateConversionMode(const ULONG convertionMode)
{
    logger::trace("DoUpdateConversionMode");
    State::GetInstance().Set(State::IN_ALPHANUMERIC, (convertionMode & IME_CMODE_LANGUAGE) == TF_CONVERSIONMODE_ALPHANUMERIC);
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
