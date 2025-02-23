#include "tsf/TsfCompartment.h"
#include "common/log.h"
#include <atlcomcli.h>

namespace LIBC_NAMESPACE_DECL
{
    auto Tsf::TsfCompartment::Initialize(ITfThreadMgr *pThreadMgr, REFGUID guidCompartment,
                                         const CompartmentChangeCallback &callback) -> HRESULT
    {
        if (IsEqualGUID(m_guidCompartment, GUID_NULL) == 0)
        {
            log_warn("TsfCompartment already initialized.");
            return S_FALSE;
        }
        HRESULT hresult   = E_FAIL;
        m_guidCompartment = guidCompartment;
        m_callback        = callback;
        if (const CComQIPtr<ITfCompartmentMgr> tfCompartmentMgr(pThreadMgr); tfCompartmentMgr != nullptr)
        {
            hresult = tfCompartmentMgr->GetCompartment(guidCompartment, &m_tfCompartment);
            if (SUCCEEDED(hresult))
            {
                if (const CComQIPtr<ITfSource> tfSource(m_tfCompartment); tfSource != nullptr)
                {
                    m_guidCompartment = guidCompartment;
                    return tfSource->AdviseSink(IID_ITfCompartmentEventSink, this, &m_dwCookie);
                }
            }
        }

        return hresult;
    }

    auto Tsf::TsfCompartment::UnInitialize() -> HRESULT
    {
        HRESULT hr = S_OK;
        if (m_tfCompartment != nullptr)
        {
            CComPtr<ITfSource> pSource;

            hr = m_tfCompartment.QueryInterface(&pSource);

            if (SUCCEEDED(hr))
            {
                hr = pSource->UnadviseSink(m_dwCookie);
            }

            m_tfCompartment.Release();
        }
        m_guidCompartment = GUID_NULL;
        return hr;
    }

    auto Tsf::TsfCompartment::GetValue(__out ULONG &pValue) const -> HRESULT
    {
        if (m_tfCompartment != nullptr)
        {
            CComVariant variant;
            HRESULT hr = m_tfCompartment->GetValue(&variant);
            if (SUCCEEDED(hr) && variant.vt == VT_I4)
            {
                pValue = variant.ulVal;
                return S_OK;
            }
        }
        return E_FAIL;
    }

    auto Tsf::TsfCompartment::QueryInterface(const IID &riid, void **ppvObject) -> HRESULT
    {
        *ppvObject = nullptr;

        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfCompartmentEventSink))
        {
            *ppvObject = this;
        }

        if (*ppvObject != nullptr)
        {
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    auto Tsf::TsfCompartment::AddRef() -> ULONG
    {
        return ++m_refCount;
    }

    auto Tsf::TsfCompartment::Release() -> ULONG
    {
        --m_refCount;
        if (m_refCount == 0)
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    auto Tsf::TsfCompartment::OnChange(const GUID &rguid) -> HRESULT
    {
        HRESULT hresult = S_OK;
        if (IsEqualGUID(rguid, m_guidCompartment) != 0)
        {
            if (ULONG value = 0; SUCCEEDED(GetValue(value)))
            {
                return m_callback(&rguid, value);
            }
        }
        return hresult;
    }
} // namespace LIBC_NAMESPACE_DECL
