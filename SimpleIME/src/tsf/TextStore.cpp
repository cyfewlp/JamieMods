#include "tsf/TextStore.h"
#include "tsf/TsfSupport.h"

#include "common/WCharUtils.h"
#include "common/log.h"

#include <algorithm>
#include <atlcomcli.h>
#include <olectl.h>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        TextStore::~TextStore()
        {
            m_adviseSinkCache.Clear();
        }

        auto TextStore::InitSinks() -> HRESULT
        {
            auto                *pUnknownThis = reinterpret_cast<IUnknown *>(this);
            CComQIPtr<ITfSource> pSource(m_uiElementMgr);
            if (pSource != nullptr)
            {
                HRESULT hresult = pSource->AdviseSink(IID_ITfUIElementSink, pUnknownThis, &m_uiElementCookie);
                if (SUCCEEDED(hresult) && m_uiElementCookie != TF_INVALID_COOKIE)
                {
                    pSource.Release();
                    if (hresult = m_context.QueryInterface(&pSource); SUCCEEDED(hresult))
                    {
                        return pSource->AdviseSink(IID_ITfTextEditSink, pUnknownThis, &m_textEditCookie);
                    }
                }
            }
            return E_FAIL;
        }

        auto TextStore::Initialize(const CComPtr<ITfThreadMgrEx> &lpThreadMgr, const TfClientId &tfClientId) -> HRESULT
        {
            HRESULT __hrAtlComMethod;
            try
            {
                log_debug("Initializing TextStore...");
                ATLENSURE_SUCCEEDED(lpThreadMgr.QueryInterface(&m_threadMgr));
                ATLENSURE_SUCCEEDED(m_threadMgr->CreateDocumentMgr(&m_documentMgr));

                __hrAtlComMethod = m_documentMgr->CreateContext(tfClientId, 0, static_cast<ITextStoreACP *>(this), //
                                                                &m_context, &m_editCookie);
                ATLENSURE_SUCCEEDED(__hrAtlComMethod);
                ATLENSURE_SUCCEEDED(m_documentMgr->Push(m_context));
                ATLENSURE_SUCCEEDED(m_threadMgr.QueryInterface(&m_uiElementMgr));
                ATLENSURE_SUCCEEDED(InitSinks());
                return S_OK;
            }
            _AFX_COM_END_PART catch (...)
            {
                __hrAtlComMethod = E_FAIL;
            }
            log_error("Failed initialize TextStore: {}", ToErrorMessage(__hrAtlComMethod));
            return __hrAtlComMethod;
        }

        auto TextStore::SetHWND(HWND hWnd) -> bool
        {
            m_hWnd = hWnd;
            return true;
        }

        void TextStore::UnInitialize()
        {
            if (m_threadMgr != nullptr)
            {
                CComPtr<ITfDocumentMgr> tempDocMgr;
                m_threadMgr->AssociateFocus(m_hWnd, m_pPrevDocMgr, &tempDocMgr);
                m_pPrevDocMgr.Release();

                if (m_documentMgr != nullptr)
                {
                    m_documentMgr->Pop(0);
                    m_documentMgr.Release();
                }

                CComPtr<ITfSource> pSource;
                if (SUCCEEDED(m_uiElementMgr.QueryInterface(&pSource)))
                {
                    pSource->UnadviseSink(m_uiElementCookie);
                    pSource.Release();
                }
                if (SUCCEEDED(m_context.QueryInterface(&pSource)))
                {
                    pSource->UnadviseSink(m_textEditCookie);
                    pSource.Release();
                }

                m_textStoreAcpServices.Release();
                m_context.Release();
                m_uiElementMgr.Release();
                m_threadMgr.Release();
            }
        }

        auto TextStore::Focus() -> HRESULT
        {
            if (m_threadMgr == nullptr || m_hWnd == nullptr || m_documentMgr == nullptr)
            {
                log_debug("Can't associate focus. Please first Initialize & set hwnd");
                return E_FAIL;
            }
            log_trace("Associate Focus");
            m_pPrevDocMgr.Release();
            return m_threadMgr->AssociateFocus(m_hWnd, m_documentMgr, &m_pPrevDocMgr);
        }

        auto TextStore::ClearFocus() const -> HRESULT
        {
            log_trace("Clear Focus");
            CComPtr<ITfDocumentMgr> tempDocMgr;
            return m_threadMgr->AssociateFocus(m_hWnd, nullptr, &tempDocMgr);
        }

        auto TextStore::QueryInterface(REFIID riid, void **ppvObject) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            *ppvObject  = nullptr;

            // IUnknown
            if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITextStoreACP))
            {
                *ppvObject = static_cast<ITextStoreACP *>(this);
            }
            else if (IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
            {
                *ppvObject = static_cast<ITfContextOwnerCompositionSink *>(this);
            }
            else if (IsEqualIID(riid, IID_ITfUIElementSink))
            {
                *ppvObject = static_cast<ITfUIElementSink *>(this);
            }
            else if (IsEqualIID(riid, IID_ITfTextEditSink))
            {
                *ppvObject = static_cast<ITfTextEditSink *>(this);
            }

            if (*ppvObject != nullptr)
            {
                AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }

        auto TextStore::AddRef() -> ULONG
        {
            return ++m_refCount;
        }

        auto TextStore::Release() -> ULONG
        {
            --m_refCount;
            if (m_refCount == 0)
            {
                delete this;
                return 0;
            }
            return m_refCount;
        }

        auto TextStore::AdviseSink(REFIID riid, IUnknown *pUnknown, DWORD dwMask) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            CComPtr<IUnknown> punkID;

            // Get the "real" IUnknown pointer. This needs to be done for comparison purposes.

            HRESULT hresult = pUnknown->QueryInterface(IID_PPV_ARGS(&punkID));
            if (FAILED(hresult))
            {
                return hresult;
            }

            hresult = E_INVALIDARG;
            if (punkID == m_adviseSinkCache.punkId)
            {
                m_adviseSinkCache.dwMask = dwMask;

                return S_OK;
            }
            if (nullptr != m_adviseSinkCache.punkId)
            {
                return CONNECT_E_ADVISELIMIT;
            }
            if (IsEqualIID(riid, IID_ITextStoreACPSink))
            {
                m_adviseSinkCache.dwMask = dwMask;
                m_adviseSinkCache.punkId = punkID;
                pUnknown->QueryInterface(IID_PPV_ARGS(&m_adviseSinkCache.pTextStoreAcpSink));
                pUnknown->QueryInterface(IID_PPV_ARGS(&m_textStoreAcpServices));
                hresult = S_OK;
            }

            return hresult;
        }

        auto TextStore::UnadviseSink(IUnknown *pUnknown) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            CComPtr<IUnknown>       punkID;
            CComPtr<IUnknown> const pUnknown1(pUnknown);

            /*
            Get the "real" IUnknown pointer. This needs to be done for comparison
            purposes.
            */
            HRESULT hresult = pUnknown1.QueryInterface(&punkID);
            if (FAILED(hresult))
            {
                return hresult;
            }

            if (punkID == m_adviseSinkCache.punkId)
            {
                m_adviseSinkCache.Clear();
                m_textStoreAcpServices.Release();
                hresult = S_OK;
            }
            else
            {
                hresult = CONNECT_E_NOCONNECTION;
            }
            return hresult;
        }

        auto TextStore::RequestLock(DWORD dwLockFlags, HRESULT *phrSession) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            if (nullptr == m_adviseSinkCache.pTextStoreAcpSink)
            {
                return E_UNEXPECTED;
            }

            if (nullptr == phrSession)
            {
                return E_INVALIDARG;
            }

            *phrSession = E_FAIL;

            if (m_fLocked)
            {
                if ((dwLockFlags & TS_LF_SYNC) != 0U)
                {
                    *phrSession = TS_E_SYNCHRONOUS;
                    return S_OK;
                }
                if ((m_dwLockType & TS_LF_READWRITE) == TS_LF_READ &&
                    ((dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE))
                {
                    m_fPendingLockUpgrade = TRUE;
                    *phrSession           = TS_S_ASYNC;
                    return S_OK;
                }
                return E_FAIL;
            }

            LockDocument(dwLockFlags);
            *phrSession = m_adviseSinkCache.pTextStoreAcpSink->OnLockGranted(dwLockFlags);
            UnlockDocument();

            return S_OK;
        }

        auto TextStore::GetStatus(TS_STATUS *pdcs) -> HRESULT
        {
            if (nullptr == pdcs)
            {
                return E_INVALIDARG;
            }

            pdcs->dwDynamicFlags = 0;
            pdcs->dwStaticFlags  = TS_SS_REGIONS;

            return S_OK;
        }

        auto TextStore::QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG /*cch*/, LONG *pacpResultStart,
                                    LONG *pacpResultEnd) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            if (const uint32_t lTextLength = m_pTextEditor->GetTextSize();
                acpTestStart > acpTestEnd || static_cast<ULONG>(acpTestEnd) > lTextLength)
            {
                return E_INVALIDARG;
            }

            *pacpResultStart = acpTestStart;
            *pacpResultEnd   = acpTestEnd;

            return S_OK;
        }

        auto TextStore::GetSelection(ULONG ulIndex, ULONG /*ulCount*/, TS_SELECTION_ACP *pSelection, ULONG *pcFetched)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            if (nullptr == pSelection || nullptr == pcFetched)
            {
                return E_INVALIDARG;
            }

            *pcFetched = 0;
            if (!IsLocked(TS_LF_READ))
            {
                return TS_E_NOLOCK;
            }

            if (TF_DEFAULT_SELECTION == ulIndex)
            {
                ulIndex = 0;
            }
            else if (ulIndex > 1)
            {
                return E_INVALIDARG;
            }

            m_pTextEditor->GetSelection(pSelection);
            tracer.log("AcpStart: {}, AcpEnd: {}", pSelection->acpStart, pSelection->acpEnd);
            *pcFetched = 1;
            return S_OK;
        }

        auto TextStore::SetSelection(const ULONG ulCount, const TS_SELECTION_ACP *pSelection) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            if (nullptr == pSelection || ulCount > 1)
            {
                return E_INVALIDARG;
            }

            if (!IsLocked(TS_LF_READWRITE))
            {
                return TS_E_NOLOCK;
            }

            m_pTextEditor->Select(pSelection);
            tracer.log("set acpStart {}, acpEnd {}", pSelection->acpStart, pSelection->acpEnd);
            // do not reverse TS_AE_START (not support choose selection dir)
            return S_OK;
        }

        auto TextStore::GetText(const LONG acpStart, const LONG acpEnd, WCHAR *pchPlain, const ULONG cchPlainReq,
                                ULONG *pcchPlainRet, TS_RUNINFO *prgRunInfo, ULONG ulRunInfoReq, ULONG *pcRunInfoRet,
                                LONG *pacpNext) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}, {}, {}", __func__, acpStart, acpEnd);
            if (!IsLocked(TS_LF_READ))
            {
                return TS_E_NOLOCK;
            }

            if (pcchPlainRet != nullptr)
            {
                *pcchPlainRet = 0;
            }

            if (ulRunInfoReq > 0)
            {
                *pcRunInfoRet = 0;
            }

            if (pacpNext != nullptr)
            {
                *pacpNext = acpStart;
            }

            LONG charSize = 0;
            m_pTextEditor->GetTextSize(charSize);

            // validate the start pos
            if (acpStart < 0 || acpStart > charSize)
            {
                return TS_E_INVALIDPOS;
            }

            if (acpStart == charSize)
            {
                return S_OK;
            }
            ULONG cchToCopy = 0;
            if (acpEnd >= acpStart)
            {
                cchToCopy = acpEnd - acpStart;
            }
            else // acpEnd will be -1 if all the text up to the end is being requested.
            {
                cchToCopy = charSize - acpStart;
            }

            if (cchPlainReq > 0)
            {
                cchToCopy = std::min(cchToCopy, cchPlainReq);
                m_pTextEditor->UnsafeGetText(pchPlain, cchPlainReq, acpStart, cchToCopy);
            }

            if (pcchPlainRet != nullptr)
            {
                *pcchPlainRet = cchToCopy;
            }

            if (ulRunInfoReq > 0)
            {
                *pcRunInfoRet      = 1;
                prgRunInfo->type   = TS_RT_PLAIN;
                prgRunInfo->uCount = cchToCopy;
            }

            if (pacpNext != nullptr)
            {
                *pacpNext = acpStart + static_cast<LONG>(cchToCopy);
            }

            return S_OK;
        }

        auto TextStore::SetText(DWORD /*dwFlags*/, LONG acpStart, LONG acpEnd, const WCHAR *pchText, ULONG cch,
                                TS_TEXTCHANGE *pChange) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            tracer.log("acpStart {}, acpEnd {}", acpStart, acpEnd);
            //
            TS_SELECTION_ACP tsa;
            tsa.acpStart           = acpStart;
            tsa.acpEnd             = acpEnd;
            tsa.style.ase          = TS_AE_END; // not support control selection direction
            tsa.style.fInterimChar = FALSE;

            HRESULT hresult        = SetSelection(1, &tsa);

            if (SUCCEEDED(hresult))
            {
                hresult = InsertTextAtSelection(TS_IAS_NOQUERY, pchText, cch, nullptr, nullptr, pChange);
            }

            return hresult;
        }

        auto TextStore::InsertTextAtSelection(DWORD dwFlags, const WCHAR *pwszText, ULONG cch, LONG *pacpStart,
                                              LONG *pacpEnd, TS_TEXTCHANGE *pChange) -> HRESULT
        {
            LONG lTemp = 0;
            if (nullptr == pacpStart)
            {
                pacpStart = &lTemp;
            }

            if (nullptr == pacpEnd)
            {
                pacpEnd = &lTemp;
            }

            if (!IsLocked(TS_LF_READWRITE))
            {
                return TS_E_NOLOCK;
            }

            LONG acpStart  = 0;
            LONG acpOldEnd = 0;
            m_pTextEditor->GetSelection(&acpStart, &acpOldEnd);

            if ((dwFlags & TS_IAS_QUERYONLY) == TS_IAS_QUERYONLY)
            {
                *pacpStart = acpStart;
                *pacpEnd   = acpOldEnd;
                return S_OK;
            }

            if (nullptr == pwszText)
            {
                return E_INVALIDARG;
            }

            LONG acpNewEnd = m_pTextEditor->InsertText(pwszText, cch);
            log_trace("TextStore::{} {}, {}, {}, count {}", __func__, acpStart, acpOldEnd, acpNewEnd, cch);

            if ((dwFlags & TS_IAS_NOQUERY) != TS_IAS_NOQUERY)
            {
                *pacpStart = acpStart;
                *pacpEnd   = acpNewEnd;
            }

            pChange->acpStart  = acpStart;
            pChange->acpOldEnd = acpOldEnd;
            pChange->acpNewEnd = acpNewEnd;
            m_fLayoutChanged   = true;
            return S_OK;
        }

        auto TextStore::GetFormattedText(LONG /*acpStart*/, LONG /*acpEnd*/, IDataObject **ppDataObject) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            if (nullptr == ppDataObject)
            {
                return E_INVALIDARG;
            }

            *ppDataObject = nullptr;

            if (!IsLocked(TS_LF_READ))
            {
                return TS_E_NOLOCK;
            }
            return E_NOTIMPL;
        }

        auto TextStore::GetEmbedded(LONG /*acpPos*/, REFGUID /*rguidService*/, REFIID /*riid*/, IUnknown ** /*ppunk*/)
            -> HRESULT
        {
            OutputDebugString(TEXT("TextStore::GetEmbedded \n"));

            // this implementation doesn't support embedded objects
            return E_NOTIMPL;
        }

        auto TextStore::QueryInsertEmbedded(const GUID * /*pguidService*/, const FORMATETC * /*pFormatEtc*/,
                                            BOOL *pfInsertable) -> HRESULT
        {
            OutputDebugString(TEXT("TextStore::QueryInsertEmbedded \n"));

            // this implementation doesn't support embedded objects
            *pfInsertable = FALSE;

            return S_OK;
        }

        auto TextStore::InsertEmbedded(DWORD /*dwFlags*/, LONG /*acpStart*/, LONG /*acpEnd*/,
                                       IDataObject * /*pDataObject*/, TS_TEXTCHANGE * /*pChange*/) -> HRESULT
        {
            OutputDebugString(TEXT("TextStore::InsertEmbedded \n"));

            // this implementation doesn't support embedded objects
            return E_NOTIMPL;
        }

        auto TextStore::RequestSupportedAttrs(DWORD /*dwFlags*/, ULONG /*cFilterAttrs*/,
                                              const TS_ATTRID * /*paFilterAttrs*/) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            return S_OK;
        }

        auto TextStore::RequestAttrsAtPosition(LONG /*acpPos*/, ULONG /*cFilterAttrs*/,
                                               const TS_ATTRID * /*paFilterAttrs*/, DWORD /*dwFlags*/) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            return S_OK;
        }

        auto TextStore::RequestAttrsTransitioningAtPosition(LONG /*acpPos*/, ULONG /*cFilterAttrs*/,
                                                            const TS_ATTRID * /*paFilterAttrs*/, DWORD /*dwFlags*/)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            return E_NOTIMPL;
        }

        auto TextStore::FindNextAttrTransition(LONG /*acpStart*/, LONG /*acpHalt*/, ULONG /*cFilterAttrs*/,
                                               const TS_ATTRID * /*paFilterAttrs*/, DWORD /*dwFlags*/,
                                               LONG * /*pacpNext*/, BOOL * /*pfFound*/, LONG * /*plFoundOffset*/)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            return E_NOTIMPL;
        }

        auto TextStore::RetrieveRequestedAttrs(ULONG /*ulCount*/, TS_ATTRVAL * /*paAttrVals*/, ULONG * /*pcFetched*/)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            return E_NOTIMPL;
        }

        auto TextStore::GetEndACP(LONG *pacp) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            if (!IsLocked(TS_LF_READWRITE))
            {
                return TS_E_NOLOCK;
            }

            if (nullptr == pacp)
            {
                return E_INVALIDARG;
            }

            m_pTextEditor->GetSelection(nullptr, pacp);
            return S_OK;
        }

        auto TextStore::GetActiveView(TsViewCookie *pvcView) -> HRESULT
        {
            *pvcView = EDIT_VIEW_COOKIE;
            return S_OK;
        }

        auto TextStore::GetACPFromPoint(TsViewCookie /*vcView*/, const POINT * /*pt*/, DWORD /*dwFlags*/,
                                        LONG * /*pacp*/) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            return E_NOTIMPL;
        }

        auto TextStore::GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            if (nullptr == prc || nullptr == pfClipped)
            {
                return E_INVALIDARG;
            }

            *pfClipped = FALSE;
            ZeroMemory(prc, sizeof(RECT));

            if (EDIT_VIEW_COOKIE != vcView)
            {
                return E_INVALIDARG;
            }

            if (!IsLocked(TS_LF_READ))
            {
                return TS_E_NOLOCK;
            }

            if (acpStart == acpEnd)
            {
                return E_INVALIDARG;
            }

            GetClientRect(m_hWnd, prc);
            MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<LPPOINT>(prc), 2);
            return S_OK;
        }

        auto TextStore::GetScreenExt(TsViewCookie vcView, RECT *prc) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            if (nullptr == prc || EDIT_VIEW_COOKIE != vcView)
            {
                return E_INVALIDARG;
            }

            GetClientRect(m_hWnd, prc);
            MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<LPPOINT>(prc), 2);
            return S_OK;
        }

        auto TextStore::GetWnd(TsViewCookie vcView, HWND *phwnd) -> HRESULT
        {
            if (EDIT_VIEW_COOKIE == vcView)
            {
                *phwnd = m_hWnd;
                return S_OK;
            }
            return E_INVALIDARG;
        }

        auto TextStore::InsertEmbeddedAtSelection(DWORD /*dwFlags*/, IDataObject * /*pDataObject*/,
                                                  LONG * /*pacpStart*/, LONG * /*pacpEnd*/, TS_TEXTCHANGE * /*pChange*/)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

            return E_NOTIMPL;
        }

        auto TextStore::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) -> HRESULT
        {
            log_trace("TextStore::{} {}", __func__, m_cCompositions);
            *pfOk = TRUE;
            if (m_cCompositions >= MAX_COMPOSITIONS)
            {
                *pfOk = FALSE;
                return S_OK;
            }

            m_cCompositions++;
            pComposition->AddRef();

#pragma unroll MAX_COMPOSITIONS
            for (auto &compositionView : m_rgCompositions)
            {
                if (!compositionView)
                {
                    compositionView.Attach(pComposition);
                    break;
                }
            }
            m_pTextService->SetState(Ime::ImeState::IN_COMPOSING);
            return S_OK;
        }

        auto TextStore::OnUpdateComposition(ITfCompositionView * /*pComposition*/, ITfRange * /*pRangeNew*/) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            m_pTextService->SetState(Ime::ImeState::IN_COMPOSING);
            return S_OK;
        }

        auto TextStore::OnEndComposition(ITfCompositionView *pComposition) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);

#pragma unroll MAX_COMPOSITIONS
            for (auto &compositionView : m_rgCompositions)
            {
                if (compositionView == pComposition)
                {
                    compositionView.Release();
                    m_cCompositions--;
                    break;
                }
            }
            if (m_OnEndCompositionCallback != nullptr)
            {
                m_OnEndCompositionCallback(m_pTextEditor->GetText());
            }
            m_pTextEditor->Select(0, 0);
            m_pTextEditor->ClearText();
            m_pTextService->ClearState(Ime::ImeState::IN_COMPOSING);
            return S_OK;
        }

        auto TextStore::BeginUIElement(DWORD dwUIElementId, BOOL *pbShow) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            *pbShow     = TRUE;
            m_pTextService->SetState(Ime::ImeState::IN_CAND_CHOOSING);
            if (FAILED(GetCandidateInterface(dwUIElementId, &m_currentCandidateUi)))
            {
                m_supportCandidateUi = false;
            }
            else if (SUCCEEDED(DoUpdateUIElement()))
            {
                *pbShow = FALSE;
            }
            else
            {
                m_currentCandidateUi.Release();
            }
            return S_OK;
        }

        auto TextStore::UpdateUIElement(const DWORD dwUIElementId) -> HRESULT
        {
            auto    tracer  = FuncTracer("TextStore::{}", __func__);
            HRESULT hresult = S_OK;
            m_currentCandidateUi.Release();
            if (FAILED(GetCandidateInterface(dwUIElementId, &m_currentCandidateUi)))
            {
                m_supportCandidateUi = false;
            }
            else
            {
                hresult = DoUpdateUIElement();
            }
            return hresult;
        }

        auto TextStore::EndUIElement(DWORD /*dwUIElementId*/) -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            m_currentCandidateUi.Release();
            m_pTextService->GetCandidateUi().Close();
            m_pTextService->ClearState(Ime::ImeState::IN_CAND_CHOOSING);
            return S_OK;
        }

        auto TextStore::GetCandidateInterface(const DWORD                         dwUIElementId,
                                              ITfCandidateListUIElementBehavior **pInterface) const -> HRESULT
        {
            CComPtr<ITfUIElement> tfUiElement;
            const HRESULT         hresult = m_uiElementMgr->GetUIElement(dwUIElementId, &tfUiElement);
            ATLENSURE_SUCCEEDED(hresult);
            return tfUiElement.QueryInterface(pInterface);
        }

        auto TextStore::GetCandInfo(ITfCandidateListUIElementBehavior *pCandidateList, CandidateInfo &candidateInfo)
            -> HRESULT
        {
            if (pCandidateList == nullptr)
            {
                return E_INVALIDARG;
            }

            UINT    pageCount = 0;
            HRESULT hresult   = pCandidateList->GetPageIndex(nullptr, 0, &pageCount);
            if (FAILED(hresult))
            {
                return hresult;
            }
            if (SUCCEEDED(hresult) && pageCount == 0)
            {
                return S_FALSE;
            }

            std::vector<UINT> candidateIdxPerPage(pageCount);
            hresult = pCandidateList->GetPageIndex(candidateIdxPerPage.data(), pageCount, &pageCount);
            ATLENSURE_RETURN(SUCCEEDED(hresult));

            UINT currentPage = 0;
            hresult          = pCandidateList->GetCount(&candidateInfo.candidateCount);
            ATLENSURE_RETURN(SUCCEEDED(hresult) && candidateInfo.candidateCount > 0U);
            hresult = pCandidateList->GetCurrentPage(&currentPage);

            if (SUCCEEDED(hresult))
            {
                candidateInfo.firstIndex = candidateIdxPerPage[currentPage];
                if (currentPage < pageCount - 1)
                {
                    candidateInfo.pageSize = candidateIdxPerPage[currentPage + 1] - candidateInfo.firstIndex;
                }
                else
                {
                    candidateInfo.pageSize = candidateInfo.candidateCount - candidateInfo.firstIndex;
                }
                return S_OK;
            }

            return E_FAIL;
        }

        auto TextStore::DoUpdateUIElement() -> HRESULT
        {
            m_supportCandidateUi = true;

            auto &candidateUi    = m_pTextService->GetCandidateUi();
            candidateUi.Close();
            if (CandidateInfo info{}; SUCCEEDED(GetCandInfo(m_currentCandidateUi, info)))
            {
                UINT selection = 0;
                candidateUi.SetPageSize(info.pageSize);
                if (SUCCEEDED(m_currentCandidateUi->GetSelection(&selection)))
                {
                    candidateUi.SetSelection(selection);
                }
                for (UINT index = info.firstIndex, j = 0; index < info.candidateCount && j < info.pageSize;
                     ++j, ++index)
                {
                    CComBSTR candidateStr;
                    if (FAILED(m_currentCandidateUi->GetString(index, &candidateStr)) || candidateStr == nullptr)
                    {
                        m_currentCandidateUi->Abort();
                        return E_FAIL;
                    }

                    auto fmt = std::format(L"{}. ", j + 1);
                    fmt.append(candidateStr);
                    auto str = WCharUtils::ToString(fmt);
                    candidateUi.PushBack(str);
                }
                return S_OK;
            }
            m_currentCandidateUi->Abort();
            return E_FAIL;
        }

        auto TextStore::OnEndEdit(ITfContext * /*pic*/, TfEditCookie /*ecReadOnly*/, ITfEditRecord * /*pEditRecord*/)
            -> HRESULT
        {
            auto tracer = FuncTracer("TextStore::{}", __func__);
            return S_OK;
        }

        void TextStore::LockDocument(const DWORD dwLockFlags)
        {
            m_fLocked    = TRUE;
            m_dwLockType = dwLockFlags;
        }

        void TextStore::UnlockDocument()
        {
            HRESULT hresult = S_OK;

            m_fLocked       = FALSE;
            m_dwLockType    = 0;

            // if there is a pending lock upgrade, grant it
            if (m_fPendingLockUpgrade)
            {
                m_fPendingLockUpgrade = FALSE;
                RequestLock(TS_LF_READWRITE, &hresult);
            }

            // if any layout changes occurred during the lock, notify the manager
            if (m_fLayoutChanged)
            {
                m_fLayoutChanged = FALSE;
                m_adviseSinkCache.pTextStoreAcpSink->OnLayoutChange(TS_LC_CHANGE, EDIT_VIEW_COOKIE);
            }
        }

        auto TextStore::IsLocked(const DWORD dwLockType) const -> bool
        {
            return m_fLocked && (m_dwLockType & dwLockType) != 0U;
        }
    } // namespace Tsf
} // namespace LIBC_NAMESPACE_DECL
