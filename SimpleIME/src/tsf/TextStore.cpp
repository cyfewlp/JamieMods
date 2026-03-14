#include "tsf/TextStore.h"

#include "WCharUtils.h"
#include "core/State.h"
#include "imgui_internal.h"
#include "log.h"
#include "tsf/TsfSupport.h"

#include <InputScope.h>
#include <atlcomcli.h>
#include <olectl.h>
#include <string>

namespace Tsf
{
using DirtyFlag = Ime::ITextService::DirtyFlag;

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
        logger::debug("Initializing TextStore...");
        ATLENSURE_SUCCEEDED(lpThreadMgr.QueryInterface(&m_threadMgr));
        ATLENSURE_SUCCEEDED(m_threadMgr->CreateDocumentMgr(&m_documentMgr));

        __hrAtlComMethod = m_documentMgr->CreateContext(
            tfClientId,
            0,
            static_cast<ITextStoreACP *>(this), //
            &m_context,
            &m_editCookie
        );
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
    logger::error("Failed initialize TextStore: {}", ToErrorMessage(__hrAtlComMethod));
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
        logger::debug("Can't associate focus. Please first Initialize & set hwnd");
        return E_FAIL;
    }
    logger::debug("Associate Focus");
    m_pPrevDocMgr.Release();
    return m_threadMgr->AssociateFocus(m_hWnd, m_documentMgr, &m_pPrevDocMgr);
}

auto TextStore::ClearFocus() const -> HRESULT
{
    logger::debug("Clear Focus");
    CComPtr<ITfDocumentMgr> tempDocMgr;
    return m_threadMgr->AssociateFocus(m_hWnd, nullptr, &tempDocMgr);
}

auto TextStore::QueryInterface(REFIID riid, void **ppvObject) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    *ppvObject = nullptr;

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
    auto tracer = FuncTracer("TextStore::{} {:#X}", __func__, dwLockFlags);

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
        if ((dwLockFlags & TS_LF_SYNC) == TS_LF_SYNC)
        {
            *phrSession = TS_E_SYNCHRONOUS;
            return S_OK;
        }
        if ((m_dwLockType & TS_LF_READWRITE) == TS_LF_READ && ((dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE))
        {
            m_fPendingLockUpgrade = TRUE;
            *phrSession           = TS_S_ASYNC;
            return S_OK;
        }
        return E_FAIL;
    }

    LockDocument(dwLockFlags);
    {
        // Hold the shared_mutex for the duration of OnLockGranted to synchronize with the UI thread.
        //
        // Two threads access m_textEditor / m_candidateUi concurrently:
        //   - TSF thread  : writes data inside OnLockGranted (SetText, InsertText, etc.)
        //   - UI/render thread: reads data via RequestUpdate() (triggered by dirty flags)
        //
        // Lock level mirrors TSF semantics onto the C++ shared_mutex:
        //   - TS_LF_READWRITE → exclusive write_lock : UI thread must wait entirely.
        //   - TS_LF_READ      → shared  read_lock   : UI thread may read concurrently.
        //
        // IMPORTANT - scope must end BEFORE UnlockDocument():
        //   UnlockDocument() may immediately call RequestLock(READWRITE) again to service
        //   a pending lock upgrade (m_fPendingLockUpgrade). That call needs GetWriteLock(),
        //   which would deadlock if the shared_lock from the read branch is still held.
        //   The closing `}` below guarantees the C++ lock is released first.
        //
        // Reentrancy during OnLockGranted is handled above (m_fLocked guard):
        //   any nested RequestLock call returns early (TS_E_SYNCHRONOUS / TS_S_ASYNC / E_FAIL)
        //   and never reaches this point, so the mutex is never acquired twice on the same thread.
        if (IsLocked(TS_LF_READWRITE))
        {
            const auto lock = m_pTextService->GetWriteLock();
            *phrSession     = m_adviseSinkCache.pTextStoreAcpSink->OnLockGranted(dwLockFlags);
        }
        else
        {
            const auto lock = m_pTextService->GetReadLock();
            *phrSession     = m_adviseSinkCache.pTextStoreAcpSink->OnLockGranted(dwLockFlags);
        }
    } // ← shared_mutex released here, before UnlockDocument()
    UnlockDocument();

    // For any candidate update, TSF will call RequestLock with TS_LF_READWRITE.
    // And may call Update/End(choose a candidate with MSPY) or only Update.
    // 1. RequestLock (MSPY: choose a candidate via number key)
    //    ---- UpdateUIElement  → sets m_pendingChangeFlags | CandidateSelection
    //    ---- EndUIElement
    //    --other calls--
    //    ---- OnEndEdit        → calls MarkDirty (composition text changed)
    // 2. RequestLock (update candidate list: composition changed, direct key, etc.)
    //    ---- UpdateUIElement  → sets m_pendingChangeFlags | CandidateSelection
    //
    // Case 1 is handled by OnEndEdit. However, selection-only changes (TF_CLUIE_SELECTION)
    // do NOT trigger OnEndEdit because the composition text itself did not change.
    // This post-lock check is the only place that catches that case.
    if (const auto flags = std::exchange(m_pendingChangeFlags, DirtyFlag{}); flags != DirtyFlag{} && m_currentUiElementId != TF_INVALID_UIELEMENTID)
    {
        m_pTextService->MarkDirty(flags);
    }
    return S_OK;
}

auto TextStore::GetStatus(TS_STATUS *pStatus) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    if (nullptr == pStatus)
    {
        return E_INVALIDARG;
    }

    pStatus->dwDynamicFlags = 0;
    pStatus->dwStaticFlags  = 0;

    return S_OK;
}

auto TextStore::QueryInsert(LONG acpStart, LONG acpEnd, ULONG /*textSize*/, LONG *pacpResultStart, LONG *pacpResultEnd) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    if (pacpResultStart == nullptr || pacpResultEnd == nullptr)
    {
        return E_INVALIDARG;
    }

    auto      &textEditor = m_pTextService->GetTextEditorWrite();
    const LONG bufferSize = static_cast<LONG>(textEditor.GetTextSize());
    if (acpStart > acpEnd || acpEnd > bufferSize)
    {
        return E_INVALIDARG;
    }

    size_t start = 0;
    size_t end   = 0;
    textEditor.GetClampedSelection(start, end);
    *pacpResultStart = std::min(std::max(acpStart, static_cast<LONG>(start)), bufferSize);
    *pacpResultEnd   = std::min(std::max(acpEnd, static_cast<LONG>(end)), bufferSize);
    return S_OK;
}

auto TextStore::GetSelection(ULONG startIndex, ULONG selectionSize, TS_SELECTION_ACP *pSelections, ULONG *pcFetched) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    if (nullptr == pSelections || nullptr == pcFetched)
    {
        return E_INVALIDARG;
    }
    if (!IsLocked(TS_LF_READ))
    {
        return TS_E_NOLOCK;
    }

    *pcFetched = 0;
    if (selectionSize > 0U && (startIndex == 0U || startIndex == TF_DEFAULT_SELECTION))
    {
        m_pTextService->GetTextEditorWrite().GetSelection(pSelections);
        tracer.log("AcpStart: {}, AcpEnd: {}", pSelections->acpStart, pSelections->acpEnd);
        *pcFetched = 1;
    }

    return S_OK;
}

auto TextStore::SetSelection(const ULONG selectionCount, const TS_SELECTION_ACP *pSelections) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    if (nullptr == pSelections || selectionCount > 1)
    {
        return E_INVALIDARG;
    }

    if (!IsLocked(TS_LF_READWRITE))
    {
        return TS_E_NOLOCK;
    }

    m_pTextService->GetTextEditorWrite().Select(pSelections);
    tracer.log("set acpStart {}, acpEnd {}", pSelections->acpStart, pSelections->acpEnd);
    // do not reverse TS_AE_START (not support choose selection dir)
    return S_OK;
}

auto TextStore::GetText(
    const LONG acpStart, LONG acpEnd, WCHAR *textBuffer, const ULONG textBufferSize, ULONG *textBufferCopied, TS_RUNINFO *runInfoBuffer,
    ULONG runInfoBufferSize, ULONG *runInfoBufferCopied, LONG *pacpNext
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}, {}, {}", __func__, acpStart, acpEnd);
    if (!IsLocked(TS_LF_READ))
    {
        return TS_E_NOLOCK;
    }

    if (textBufferCopied != nullptr)
    {
        *textBufferCopied = 0U;
    }

    if (runInfoBufferSize > 0U)
    {
        *runInfoBufferCopied = 0U;
    }

    if (pacpNext != nullptr)
    {
        *pacpNext = acpStart;
    }

    auto                   &textEditor = m_pTextService->GetTextEditorWrite();
    const std::wstring_view editorText = textEditor.GetText();
    const size_t            charSize   = editorText.size();
    if (acpEnd == -1)
    {
        acpEnd = static_cast<LONG>(charSize);
    }
    if (!((0 <= acpStart) && (acpStart <= acpEnd) && (acpEnd <= static_cast<LONG>(charSize))))
    {
        return TF_E_INVALIDPOS;
    }

    const auto uAcpStart = static_cast<size_t>(acpStart);
    const auto uAcpEnd   = static_cast<size_t>(acpEnd);

    if (uAcpStart == charSize)
    {
        return S_OK;
    }

    auto charCountToCopy = static_cast<ULONG>(uAcpEnd - uAcpStart);
    if (textBuffer != nullptr && textBufferSize > 0U)
    {
        charCountToCopy                  = std::min(charCountToCopy, textBufferSize);
        const std::wstring_view svToCopy = editorText.substr(uAcpStart, charCountToCopy);
        for (size_t i = 0; i < svToCopy.size(); i++)
        {
            textBuffer[i] = svToCopy[i];
        }
    }

    if (textBufferCopied != nullptr)
    {
        *textBufferCopied = charCountToCopy;
    }

    if (runInfoBufferSize > 0U)
    {
        *runInfoBufferCopied  = 1U;
        runInfoBuffer->type   = TS_RT_PLAIN;
        runInfoBuffer->uCount = charCountToCopy;
    }

    if (pacpNext != nullptr)
    {
        *pacpNext = static_cast<int32_t>(uAcpStart + charCountToCopy);
    }

    return S_OK;
}

auto TextStore::SetText(DWORD /*dwFlags*/, LONG acpStart, LONG acpEnd, const WCHAR *pchText, ULONG cch, TS_TEXTCHANGE *pChange) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    tracer.log("acpStart {}, acpEnd {}", acpStart, acpEnd);
    if (!IsLocked(TS_LF_READWRITE))
    {
        return TS_E_NOLOCK;
    }

    TS_SELECTION_ACP tsa;
    tsa.acpStart           = acpStart;
    tsa.acpEnd             = acpEnd;
    tsa.style.ase          = TS_AE_END; // not support control selection direction
    tsa.style.fInterimChar = FALSE;

    HRESULT hresult = SetSelection(1, &tsa);

    if (SUCCEEDED(hresult))
    {
        hresult = InsertTextAtSelection(0U, pchText, cch, &acpStart, &acpEnd, pChange);
    }

    return hresult;
}

auto TextStore::InsertTextAtSelection(
    DWORD dwFlags, const WCHAR *textBuffer, ULONG textBufferSize, LONG *pacpStart, LONG *pacpEnd, TS_TEXTCHANGE *pChange
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{} count {}", __func__, textBufferSize);
    LONG lTemp  = 0;
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
    m_pTextService->GetTextEditorWrite().GetSelection(&acpStart, &acpOldEnd);
    tracer.log("Current Selection: {}, {}", acpStart, acpOldEnd);

    if ((dwFlags & TS_IAS_QUERYONLY) == TS_IAS_QUERYONLY)
    {
        *pacpStart = acpStart;
        *pacpEnd   = acpOldEnd;
        return S_OK;
    }

    // may nullptr + 0, legal.
    if (!m_pTextService->GetTextEditorWrite().InsertText(textBuffer, textBufferSize))
    {
        return E_FAIL;
    }

    auto acpNewEnd = acpStart + static_cast<LONG>(textBufferSize);
    tracer.log("Insert Success: new end {}", acpNewEnd);

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

auto TextStore::GetEmbedded(LONG /*acpPos*/, REFGUID /*rguidService*/, REFIID /*riid*/, IUnknown ** /*ppunk*/) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    // this implementation doesn't support embedded objects
    return E_NOTIMPL;
}

auto TextStore::QueryInsertEmbedded(const GUID * /*pguidService*/, const FORMATETC * /*pFormatEtc*/, BOOL *pfInsertable) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    // this implementation doesn't support embedded objects
    *pfInsertable = FALSE;

    return S_OK;
}

auto TextStore::InsertEmbedded(
    DWORD /*dwFlags*/, LONG /*acpStart*/, LONG /*acpEnd*/, IDataObject * /*pDataObject*/, TS_TEXTCHANGE * /*pChange*/
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    // this implementation doesn't support embedded objects
    return E_NOTIMPL;
}

auto TextStore::RequestSupportedAttrs(DWORD /*dwFlags*/, ULONG /*filterAttrCount*/, const TS_ATTRID * /*filterAttrs*/) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    return S_OK;
}

auto TextStore::RequestAttrsAtPosition(
    LONG /*acpPos*/, ULONG /*filterAttrCount*/, const TS_ATTRID * /*filterAttrs*/, DWORD /*dwFlags*/
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    return S_OK;
}

auto TextStore::RequestAttrsTransitioningAtPosition(
    LONG /*acpPos*/, ULONG /*filterAttrCount*/, const TS_ATTRID * /*filterAttrs*/, DWORD /*dwFlags*/
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    return E_NOTIMPL;
}

auto TextStore::FindNextAttrTransition(
    LONG /*acpStart*/, LONG /*acpStop*/, ULONG /*filterAttrCount*/, const TS_ATTRID * /*filterAttrs*/, DWORD /*dwFlags*/, LONG * /*pacpNext*/,
    BOOL * /*pfFoundTransition*/, LONG * /*foundOffset*/
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    return E_NOTIMPL;
}

auto TextStore::RetrieveRequestedAttrs(ULONG /*attrCount*/, TS_ATTRVAL * /*attrVals*/, ULONG * /*pcFetched*/) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    return S_OK;
}

auto TextStore::GetEndACP(LONG *pacpEnd) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    if (!IsLocked(TS_LF_READ))
    {
        return TS_E_NOLOCK;
    }

    if (nullptr == pacpEnd)
    {
        return E_INVALIDARG;
    }

    *pacpEnd = static_cast<LONG>(m_pTextService->GetTextEditorWrite().GetTextSize());
    return S_OK;
}

auto TextStore::GetActiveView(TsViewCookie *pViewCookie) -> HRESULT
{
    auto tracer  = FuncTracer("TextStore::{}", __func__);
    *pViewCookie = EDIT_VIEW_COOKIE;
    return S_OK;
}

auto TextStore::GetACPFromPoint(TsViewCookie /*vcView*/, const POINT * /*ptScreenPos*/, DWORD /*dwFlags*/, LONG * /*pacpPos*/) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    return E_NOTIMPL;
}

auto TextStore::GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *pRect, BOOL *pfIsClipped) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    if (nullptr == pRect || nullptr == pfIsClipped)
    {
        return E_INVALIDARG;
    }

    *pfIsClipped = FALSE;
    ZeroMemory(pRect, sizeof(RECT));

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

    GetClientRect(m_hWnd, pRect);
    MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<LPPOINT>(pRect), 2);
    return S_OK;
}

auto TextStore::GetScreenExt(TsViewCookie vcView, RECT *pRect) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    if (nullptr == pRect || EDIT_VIEW_COOKIE != vcView)
    {
        return E_INVALIDARG;
    }

    GetClientRect(m_hWnd, pRect);
    MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<LPPOINT>(pRect), 2);
    return S_OK;
}

auto TextStore::GetWnd(TsViewCookie vcView, HWND *pHwnd) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    if (EDIT_VIEW_COOKIE == vcView)
    {
        *pHwnd = m_hWnd;
        return S_OK;
    }
    return E_INVALIDARG;
}

auto TextStore::InsertEmbeddedAtSelection(
    DWORD /*dwFlags*/, IDataObject * /*pDataObject*/, LONG * /*pacpStart*/, LONG * /*pacpEnd*/, TS_TEXTCHANGE * /*pChange*/
) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);

    return E_NOTIMPL;
}

auto TextStore::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) -> HRESULT
{
    logger::trace("TextStore::{}", __func__);
    *pfOk = TRUE;
    pComposition->AddRef();
    State::GetInstance().Set(State::IN_COMPOSING);
    return S_OK;
}

auto TextStore::OnUpdateComposition(ITfCompositionView * /*pComposition*/, ITfRange * /*pRangeNew*/) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    State::GetInstance().Set(State::IN_COMPOSING);
    return S_OK;
}

auto TextStore::OnEndComposition(ITfCompositionView * /*pComposition*/) -> HRESULT
{
    auto  tracer     = FuncTracer("TextStore::{}", __func__);
    auto &textEditor = m_pTextService->GetTextEditorWrite();
    if (m_OnEndCompositionCallback != nullptr)
    {
        m_OnEndCompositionCallback(textEditor.GetText());
    }
    textEditor.Select(0, 0);
    textEditor.ClearText();
    {
        if (auto &candidateUi = m_pTextService->GetCandidateUiWrite(); !candidateUi.empty())
        {
            candidateUi.Close();
            m_pendingChangeFlags |= DirtyFlag::CandidateList;
        }
    }
    State::GetInstance().Clear(State::IN_COMPOSING);
    State::GetInstance().Clear(State::IN_CAND_CHOOSING);
    return S_OK;
}

auto TextStore::BeginUIElement(DWORD dwUIElementId, BOOL *pbShow) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{} {}", __func__, ImGui::GetFrameCount());
    if (dwUIElementId == TF_INVALID_UIELEMENTID || pbShow == nullptr)
    {
        return E_INVALIDARG;
    }

    *pbShow              = TRUE;
    m_currentCandidateUi = nullptr;
    m_currentUiElementId = TF_INVALID_UIELEMENTID;
    State::GetInstance().Set(State::IN_CAND_CHOOSING);
    if (SUCCEEDED(GetCandidateInterface(dwUIElementId, &m_currentCandidateUi)))
    {
        m_currentUiElementId = dwUIElementId;
    }
    return S_OK;
}

auto TextStore::UpdateUIElement(const DWORD dwUIElementId) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{} {}", __func__, ImGui::GetFrameCount());
    if (dwUIElementId == TF_INVALID_UIELEMENTID)
    {
        return E_INVALIDARG;
    }
    if (m_currentUiElementId != dwUIElementId)
    {
        return S_OK;
    }

    HRESULT hresult = S_OK;
    m_currentCandidateUi.Release();
    if (FAILED(GetCandidateInterface(dwUIElementId, &m_currentCandidateUi)))
    {
        m_currentCandidateUi.Release();
    }
    else
    {
        hresult = DoUpdateUIElement();
    }
    return hresult;
}

auto TextStore::EndUIElement(DWORD dwUIElementId) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{} {}", __func__, ImGui::GetFrameCount());
    if (dwUIElementId == TF_INVALID_UIELEMENTID)
    {
        return E_INVALIDARG;
    }
    if (m_currentUiElementId != dwUIElementId)
    {
        return S_OK;
    }
    m_currentUiElementId = TF_INVALID_UIELEMENTID;
    m_currentCandidateUi.Release();
    // IMPORTANT: Do NOT close CandidateUI here - keep cache until composition ends.
    //
    // Our CandidateUI is a cache, NOT tied to system UIElement lifecycle.
    // UIElement events (e.g., EndUIElement) are just notifications.
    //
    // Example: MSPY closes/reopens UIElement after each candidate selection.
    // If we close our cache on EndUIElement, we'd lose data prematurely.
    //
    // The cache can even persist beyond composition end (e.g., for reconversion/debugging).
    //
    // m_pTextService->GetCandiDateUiWrite().Close();  // Intentionally disabled
    State::GetInstance().Clear(State::IN_CAND_CHOOSING);
    return S_OK;
}

auto TextStore::CommitCandidate(UINT index) const -> bool
{
    if ((m_currentCandidateUi != nullptr) && SUCCEEDED(m_currentCandidateUi->SetSelection(index)))
    {
        return SUCCEEDED(m_currentCandidateUi->Finalize());
    }
    return false;
}

auto TextStore::GetCandidateInterface(const DWORD dwUIElementId, ITfCandidateListUIElementBehavior **pInterface) const -> HRESULT
{
    CComPtr<ITfUIElement> tfUiElement;
    const HRESULT         hresult = m_uiElementMgr->GetUIElement(dwUIElementId, &tfUiElement);
    ATLENSURE_SUCCEEDED(hresult);
    return tfUiElement.QueryInterface(pInterface);
}

auto TextStore::GetCandInfo(ITfCandidateListUIElementBehavior *pCandidateList, CandidateInfo &candidateInfo) -> HRESULT
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
    UINT count       = 0;
    hresult          = pCandidateList->GetCount(&count);
    ATLENSURE_RETURN(SUCCEEDED(hresult) && count > 0U);
    hresult = pCandidateList->GetCurrentPage(&currentPage);

    if (SUCCEEDED(hresult))
    {
        candidateInfo.pageStart = candidateIdxPerPage[currentPage];
        candidateInfo.pageEnd   = currentPage < pageCount - 1 ? candidateIdxPerPage[currentPage + 1] : count;
        return S_OK;
    }

    return E_FAIL;
}

auto TextStore::DoUpdateUIElement() -> HRESULT
{
    DWORD updateFlags = 0;
    if (FAILED(m_currentCandidateUi->GetUpdatedFlags(&updateFlags)))
    {
        return E_FAIL;
    }

    if (CandidateInfo info{}; SUCCEEDED(GetCandInfo(m_currentCandidateUi, info)))
    {
        UINT selection = 0;
        if (FAILED(m_currentCandidateUi->GetSelection(&selection)))
        {
            return E_FAIL;
        }

        auto &candidateUi = m_pTextService->GetCandidateUiWrite();
        candidateUi.SetSelection(selection - info.pageStart);
        if (updateFlags == TF_CLUIE_SELECTION && candidateUi.FirstIndex() == info.pageStart)
        {
            m_pendingChangeFlags |= DirtyFlag::CandidateSelection;
            return S_OK; // No need to update candidate list if only selection changed.
        }

        // if only selection change but page changed, we still need to update candidate list.
        m_pendingChangeFlags |= DirtyFlag::CandidateList;
        candidateUi.Close();
        candidateUi.SetFirstIndex(info.pageStart);
        candidateUi.Reserve(info.pageEnd - info.pageStart);
        for (UINT index = info.pageStart, j = 0; index < info.pageEnd; ++j, ++index)
        {
            CComBSTR candidateStr;
            if (FAILED(m_currentCandidateUi->GetString(index, &candidateStr)) || candidateStr == nullptr)
            {
                m_currentCandidateUi->Abort();
                return E_FAIL;
            }

            auto fmt = std::format(L"{}. ", j + 1);
            fmt.append(candidateStr);
            candidateUi.PushBack(WCharUtils::ToString(fmt));
        }
        return S_OK;
    }
    m_currentCandidateUi->Abort();
    return E_FAIL;
}

auto TextStore::OnEndEdit(ITfContext * /*pic*/, TfEditCookie /*ecReadOnly*/, ITfEditRecord * /*pEditRecord*/) -> HRESULT
{
    auto tracer = FuncTracer("TextStore::{}", __func__);
    m_pTextService->MarkDirty(m_pendingChangeFlags | DirtyFlag::Composition);
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

    m_fLocked    = FALSE;
    m_dwLockType = 0;

    // if there is a pending lock upgrade, grant it
    if (m_fPendingLockUpgrade)
    {
        m_fPendingLockUpgrade = FALSE;
        RequestLock(TS_LF_READWRITE, &hresult);
    }

    // if any layout changes occurred during the lock, notify the manager
    if (m_fLayoutChanged)
    {
        m_fLayoutChanged = false;
        m_adviseSinkCache.pTextStoreAcpSink->OnLayoutChange(TS_LC_CHANGE, EDIT_VIEW_COOKIE);
    }
}

auto TextStore::IsLocked(const DWORD dwLockType) const -> bool
{
    return m_fLocked && (m_dwLockType & dwLockType) != 0U;
}
} // namespace Tsf
