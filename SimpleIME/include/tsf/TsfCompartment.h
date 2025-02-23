#ifndef TSF_TSFCOMPARTMENT_H
    #define TSF_TSFCOMPARTMENT_H

    #pragma once

    #include <atlcomcli.h>
    #include <msctf.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        using CompartmentChangeCallback = std::function<HRESULT(const GUID *, ULONG)>;

        class TsfCompartment : public ITfCompartmentEventSink
        {
        public:
            TsfCompartment()                                                    = default;
            virtual ~TsfCompartment()                                           = default;
            TsfCompartment(const TsfCompartment &other)                         = delete;
            TsfCompartment(TsfCompartment &&other) noexcept                     = delete;
            auto operator=(const TsfCompartment &other) -> TsfCompartment &     = delete;
            auto operator=(TsfCompartment &&other) noexcept -> TsfCompartment & = delete;

            auto Initialize(ITfThreadMgr *pThreadMgr, const GUID &guidCompartment,
                            const CompartmentChangeCallback &callback = nullptr) -> HRESULT;
            auto UnInitialize() -> HRESULT;
            auto GetValue(__out ULONG &pValue) const -> HRESULT;

            auto AddRef() -> ULONG override;
            auto Release() -> ULONG override;

        private:
            auto                      QueryInterface(const IID &riid, void **ppvObject) -> HRESULT override;
            auto                      OnChange(const GUID &rguid) -> HRESULT override;

            CComPtr<ITfCompartment>   m_tfCompartment;
            DWORD                     m_dwCookie = TF_INVALID_COOKIE;
            DWORD                     m_refCount = 0;
            GUID                      m_guidCompartment{};
            CompartmentChangeCallback m_callback = nullptr;
        };
    } // namespace Tsf
}
#endif
// namespace LIBC_NAMESPACE_DECL