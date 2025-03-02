//
// Created by jamie on 2025/2/22.
//
#include "common/log.h"
#include "tsf/TextStore.h"
#include "tsf/TsfSupport.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        HRESULT TextService::Initialize()
        {
            m_pTextStore           = new TextStore(this, &m_textEditor);
            auto const &tsfSupport = TsfSupport::GetSingleton();
            // callback
            auto callback = [this](const GUID * /*guid*/, const ULONG ulong) {
                if ((ulong & IME_CMODE_LANGUAGE) == TF_CONVERSIONMODE_ALPHANUMERIC)
                {
                    SetState(Ime::ImeState::IN_ALPHANUMERIC);
                }
                else
                {
                    ClearState(Ime::ImeState::IN_ALPHANUMERIC);
                }
                return S_OK;
            };
            m_pCompartment  = new TsfCompartment();
            HRESULT hresult = m_pCompartment->Initialize(tsfSupport.GetThreadMgr(),
                                                         GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, callback);
            if (SUCCEEDED(hresult))
            {
                hresult = m_pTextStore->Initialize(tsfSupport.GetThreadMgr(), tsfSupport.GetTfClientId());
            }
            return hresult;
        }

        void TextService::UpdateConversionMode()
        {
            ULONG convertionMode = 0;
            if (SUCCEEDED(m_pCompartment->GetValue(convertionMode)))
            {
                if ((convertionMode & 3) == TF_CONVERSIONMODE_ALPHANUMERIC)
                {
                    SetState(Ime::ImeState::IN_ALPHANUMERIC);
                }
                else
                {
                    ClearState(Ime::ImeState::IN_ALPHANUMERIC);
                }
            }
        }

        void TextService::Enable(const bool enable)
        {
            if (enable)
            {
                if (HasState(Ime::ImeState::IME_DISABLED))
                {
                    UpdateConversionMode();
                    m_pTextStore->Focus();
                }
            }
            else
            {
                if (HasNoStates(Ime::ImeState::IME_DISABLED))
                {
                    HRESULT hr = m_pTextStore->ClearFocus();
                    if (FAILED(hr))
                    {
                        log_warn("Unexpected error, failed clear focus");
                    }
                }
            }
            ITextService::Enable(enable);
        }

        auto TextService::ProcessImeMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> bool
        {
            switch (uMsg)
            {
                case WM_IME_NOTIFY:
                    if (!m_pTextStore->IsSupportCandidateUi())
                    {
                        if (m_fallbackTextService.ProcessImeMessage(hWnd, uMsg, wParam, lParam))
                        {
                            m_candidateUi.Close();
                            auto &candidateUi = m_fallbackTextService.GetCandidateUi();
                            if (const auto &candidateList = candidateUi.UnsafeCandidateList(); candidateList.size() > 0)
                            {
                                for (const auto &candidate : candidateList)
                                {
                                    m_candidateUi.PushBack(candidate);
                                }
                                candidateUi.Close();
                                m_candidateUi.SetSelection(candidateUi.Selection());
                                m_candidateUi.SetPageSize(candidateUi.PageSize());
                                return true;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
            return false;
        }
    } // namespace Tsf
} // namespace LIBC_NAMESPACE_DECL