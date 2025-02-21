#pragma once

#include "common/common.h"

#include <atlcomcli.h>
#include <msctf.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Tsf
    {
        class TsfCompartment : public ITfCompartmentEventSink
        {
        public:
            TsfCompartment() = default;
            virtual ~TsfCompartment() = default;
            TsfCompartment(const TsfCompartment &other)                = delete;
            TsfCompartment(TsfCompartment &&other) noexcept            = delete;
            TsfCompartment &operator=(const TsfCompartment &other)     = delete;
            TsfCompartment &operator=(TsfCompartment &&other) noexcept = delete;

            auto    Initialize(ITfThreadMgr *pThreadMgr, const GUID &guidCompartment) -> HRESULT;
            HRESULT UnInitialize();

            auto    AddRef() -> ULONG override;
            auto    Release() -> ULONG override;

        private:
            HRESULT                 QueryInterface(const IID &riid, void **ppvObject) override;
            HRESULT                 OnChange(const GUID &rguid) override;

            CComPtr<ITfCompartment> m_tfCompartment;
            DWORD                   m_dwCookie = TF_INVALID_COOKIE;
            DWORD                   m_refCount = 0;
            GUID                    m_guidCompartment;
        };
    }
}