#ifndef TSF_TEXTSTORE_H
#define TSF_TEXTSTORE_H

#pragma once

#include "ime/ITextService.h"
#include "ime/TextEditor.h"
#include "tsf/TsfSupport.h"

#include <array>
#include <atlcomcli.h>
#include <msctf.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        struct FuncTracer
        {
            template <typename... Args>
            explicit constexpr FuncTracer(std::format_string<Args...> fmt, Args &&...args)
            {
                if (spdlog::should_log(spdlog::level::trace))
                {
                    spdlog::trace("{:>{}}{}", "", g_indent, std::format(fmt, std::forward<Args>(args)...));
                    // spdlog::log(spdlog::level::trace, "{:>{}}{}", "", g_indent, fmt, std::forward<Args>(args)...);
                    g_indent += 4;
                }
            }

            template <typename... Args>
            constexpr void log(std::format_string<Args...> fmt, Args &&...args)
            {
                if (spdlog::should_log(spdlog::level::trace))
                {
                    spdlog::trace("{:>{}}{}", "", g_indent, std::format(fmt, std::forward<Args>(args)...));
                }
            }

            ~FuncTracer()
            {
                g_indent -= 4;
            }

        private:
            static inline uint32_t g_indent;
        };

        struct AdviseSinkCache
        {
            CComPtr<IUnknown>          punkId            = nullptr;
            CComPtr<ITextStoreACPSink> pTextStoreAcpSink = nullptr;
            DWORD                      dwMask            = 0;

            void                       Clear()
            {
                punkId.Release();
                pTextStoreAcpSink.Release();
                dwMask = 0;
            }
        } __attribute__((packed)) __attribute__((aligned(32)));

        struct CandidateInfo
        {
            UINT  candidateCount = 0;
            UINT  pageSize       = 0;
            DWORD firstIndex     = 0;
        } __attribute__((aligned(16)));

        class TextStore : public ITextStoreACP, ITfContextOwnerCompositionSink, ITfUIElementSink, ITfTextEditSink
        {
            static constexpr uint32_t MAX_COMPOSITIONS = 5;
            static constexpr uint32_t EDIT_VIEW_COOKIE = 0;

        public:
            explicit TextStore(Ime::ITextService *pTextService, Ime::TextEditor *pTextEditor)
                : m_pTextService(pTextService), m_pTextEditor(pTextEditor)
            {
            }

            virtual ~TextStore();
            TextStore(const TextStore &other)                         = delete;
            TextStore(TextStore &&other) noexcept                     = delete;
            auto operator=(const TextStore &other) -> TextStore &     = delete;
            auto operator=(TextStore &&other) noexcept -> TextStore & = delete;

            auto Initialize(const CComPtr<ITfThreadMgrEx> &lpThreadMgr, const TfClientId &tfClientId) -> HRESULT;
            auto SetHWND(HWND hWnd) -> bool;
            void UnInitialize();

            auto Focus() -> HRESULT;

            [[nodiscard]] constexpr auto IsSupportCandidateUi() const -> bool
            {
                return m_supportCandidateUi;
            }

            [[nodiscard]] constexpr auto DocumentMgr() -> ITfDocumentMgr *
            {
                return m_documentMgr;
            }

            auto __stdcall AddRef() -> ULONG override;
            auto __stdcall Release() -> ULONG override;

            void SetOnEndCompositionCallback(Ime::OnEndCompositionCallback *const callback)
            {
                m_OnEndCompositionCallback = callback;
            }

        private:
            auto InitSinks() -> HRESULT;

            // ITextStoreACP functions
            // NOLINTBEGIN(*-use-trailing-return-type)
            STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override;
            STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD dwMask) override;
            STDMETHODIMP UnadviseSink(IUnknown *punk) override;
            STDMETHODIMP RequestLock(DWORD dwLockFlags, HRESULT *phrSession) override;
            STDMETHODIMP GetStatus(TS_STATUS *pdcs) override;
            /**
             * determines whether the specified start and end character positions are valid. Use this method to adjust
             * an edit to a document before executing the edit. The method must not return values outside the range of
             * the document.
             * @param acpTestStart Starting application character position for inserted text.
             * @param acpTestEnd Ending application character position for the inserted text. This value is equal to
             * acpTextStart if the text is inserted at a point instead of replacing selected text.
             * @param cch Length of replacement text.
             * @param pacpResultStart Returns the new starting application character position of the inserted text. If
             * this parameter is NULL, then text cannot be inserted at the specified position. This value cannot be
             * outside the document range.
             * @param pacpResultEnd Returns the new ending application character position of the inserted text. If this
             * parameter is NULL, then pacpResultStart is set to NULL and text cannot be inserted at the specified
             * position. This value cannot be outside the document range.
             * @return
             */
            STDMETHODIMP QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG *pacpResultStart,
                                     LONG *pacpResultEnd) override;
            /**
             * returns the character position of a text selection in a document. This method supports multiple text
             * selections. The caller must have a read-only lock on the document before calling this method.
             * @param ulIndex Specifies the text selections that start the process. If the TF_DEFAULT_SELECTION constant
             * is specified for this parameter, the input selection starts the process.
             * @param ulCount Specifies the maximum number of selections to return.
             * @param pSelection Receives the style, start, and end character positions of the selected text. These
             * values are put into the TS_SELECTION_ACP structure.
             * @param pcFetched Receives the number of pSelection structures returned.
             * @return
             */
            STDMETHODIMP GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP *pSelection,
                                      ULONG *pcFetched) override;
            STDMETHODIMP SetSelection(ULONG ulCount, const TS_SELECTION_ACP *pSelection) override;
            STDMETHODIMP GetText(LONG acpStart, LONG acpEnd, WCHAR *pchPlain, ULONG cchPlainReq, ULONG *pcchPlainRet,
                                 TS_RUNINFO *prgRunInfo, ULONG cRunInfoReq, ULONG *pcRunInfoRet,
                                 LONG *pacpNext) override;
            STDMETHODIMP SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR *pchText, ULONG cch,
                                 TS_TEXTCHANGE *pChange) override;
            STDMETHODIMP GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject **ppDataObject) override;
            STDMETHODIMP GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown **ppunk) override;
            STDMETHODIMP QueryInsertEmbedded(const GUID *pguidService, const FORMATETC *pFormatEtc,
                                             BOOL *pfInsertable) override;
            STDMETHODIMP InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject *pDataObject,
                                        TS_TEXTCHANGE *pChange) override;
            STDMETHODIMP InsertTextAtSelection(DWORD dwFlags, const WCHAR *pchText, ULONG cch, LONG *pacpStart,
                                               LONG *pacpEnd, TS_TEXTCHANGE *pChange) override;
            STDMETHODIMP InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject *pDataObject, LONG *pacpStart,
                                                   LONG *pacpEnd, TS_TEXTCHANGE *pChange) override;
            STDMETHODIMP RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs,
                                               const TS_ATTRID *paFilterAttrs) override;
            STDMETHODIMP RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID *paFilterAttrs,
                                                DWORD dwFlags) override;
            STDMETHODIMP RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs,
                                                             const TS_ATTRID *paFilterAttrs, DWORD dwFlags) override;
            STDMETHODIMP FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs,
                                                const TS_ATTRID *paFilterAttrs, DWORD dwFlags, LONG *pacpNext,
                                                BOOL *pfFound, LONG *plFoundOffset) override;
            STDMETHODIMP RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL *paAttrVals, ULONG *pcFetched) override;
            STDMETHODIMP GetEndACP(LONG *pacp) override;
            STDMETHODIMP GetActiveView(TsViewCookie *pvcView) override;
            STDMETHODIMP GetACPFromPoint(TsViewCookie vcView, const POINT *ptScreen, DWORD dwFlags,
                                         LONG *pacp) override;
            STDMETHODIMP GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT *prc,
                                    BOOL *pfClipped) override;
            STDMETHODIMP GetScreenExt(TsViewCookie vcView, RECT *prc) override;
            STDMETHODIMP GetWnd(TsViewCookie vcView, HWND *phwnd) override;

            // ITfContextOwnerCompositionSink functions
            STDMETHODIMP OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) override;
            STDMETHODIMP OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew) override;
            STDMETHODIMP OnEndComposition(ITfCompositionView *pComposition) override;

            // ITfUIElementSink functions
            STDMETHODIMP BeginUIElement(DWORD dwUIElementId, BOOL *pbShow) override;
            STDMETHODIMP UpdateUIElement(DWORD dwUIElementId) override;
            STDMETHODIMP EndUIElement(DWORD dwUIElementId) override;
            // NOLINTEND(*-use-trailing-return-type)
            auto DoUpdateUIElement() -> HRESULT;
            auto GetCandidateInterface(DWORD dwUIElementId, ITfCandidateListUIElementBehavior **pInterface) const
                -> HRESULT;
            static auto GetCandInfo(ITfCandidateListUIElementBehavior *pCandidateList, CandidateInfo &candidateInfo)
                -> HRESULT;
            auto OnEndEdit([in] ITfContext *pic, [in] TfEditCookie ecReadOnly, [in] ITfEditRecord *pEditRecord)
                -> HRESULT override;

            void               LockDocument(DWORD dwLockFlags);
            void               UnlockDocument();
            [[nodiscard]] auto IsLocked(DWORD dwLockType) const -> bool;

            // lock vars
            DWORD                          m_refCount{0};
            DWORD                          m_dwLockType{0};
            bool                           m_fPendingLockUpgrade{false};
            bool                           m_fLocked{false};
            bool                           m_fLayoutChanged{false};
            HWND                           m_hWnd{nullptr};
            bool                           m_supportCandidateUi       = true;

            Ime::ITextService             *m_pTextService             = nullptr;
            Ime::TextEditor               *m_pTextEditor              = nullptr;
            Ime::OnEndCompositionCallback *m_OnEndCompositionCallback = nullptr;

            //
            AdviseSinkCache                                           m_adviseSinkCache{};
            ULONG                                                     m_cCompositions{0};
            std::array<CComPtr<ITfCompositionView>, MAX_COMPOSITIONS> m_rgCompositions{};

            // TSF com ptr
            CComPtr<ITextStoreACPServices>             m_textStoreAcpServices   = nullptr;
            CComPtr<ITfThreadMgr>                      m_threadMgr              = nullptr;
            CComPtr<ITfDocumentMgr>                    m_documentMgr            = nullptr;
            CComPtr<ITfDocumentMgr>                    m_pPrevDocMgr            = nullptr;
            CComPtr<ITfUIElementMgr>                   m_uiElementMgr           = nullptr;
            CComPtr<ITfContext>                        m_context                = nullptr;
            CComPtr<ITfCompositionView>                m_currentCompositionView = nullptr;
            CComPtr<ITfCandidateListUIElementBehavior> m_currentCandidateUi     = nullptr;

            TfEditCookie                               m_editCookie{0};
            DWORD                                      m_uiElementCookie{0};
            DWORD                                      m_textEditCookie{0};
        };

        class TextService : public Ime::ITextService
        {
        public:
            auto Initialize() -> HRESULT override
            {
                m_pTextStore                 = new TextStore(this, &m_textEditor);
                const TsfSupport *tsfSupport = TsfSupport::GetSingleton();
                return m_pTextStore->Initialize(tsfSupport->GetThreadMgr(), tsfSupport->GetTfClientId());
            }

            void UnInitialize() override
            {
                m_pTextStore->UnInitialize();
            }

            void RegisterCallback(Ime::OnEndCompositionCallback *callback) override
            {
                m_pTextStore->SetOnEndCompositionCallback(callback);
            }

            void OnStart(HWND hWnd) override
            {
                m_pTextStore->SetHWND(hWnd);
                m_pTextStore->Focus();
            }

            [[nodiscard]] auto GetCandidateUi() -> Ime::CandidateUi & override
            {
                return m_candidateUi;
            }

            [[nodiscard]] auto GetTextEditor() -> Ime::TextEditor & override
            {
                return m_textEditor;
            }

            auto ProcessImeMessage(HWND /*hWnd*/, UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) -> bool override;

        private:
            Ime::CandidateUi             m_candidateUi;
            Ime::TextEditor              m_textEditor;
            Ime::Imm32::Imm32TextService m_fallbackTextService;
            CComPtr<TextStore>           m_pTextStore = nullptr;
        };
    } // namespace Tsf
} // namespace LIBC_NAMESPACE_DECL
#endif
