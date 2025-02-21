#include "tsf/TsfSupport.h"
#include "common/WCharUtils.h"
#include <comdef.h>

auto LIBC_NAMESPACE::Tsf::ToErrorMessage(const HRESULT hresult) -> std::string
{
    const _com_error err(hresult);
    return WCharUtils::ToString(err.ErrorMessage());
}

auto LIBC_NAMESPACE::Tsf::TsfSupport::InitializeTsf(const bool uiLessMode) -> HRESULT
{
    if (m_initialized)
    {
        return S_OK;
    }

    try
    {
        HRESULT hresult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        ATLENSURE_SUCCEEDED(hresult);

        hresult = m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER);
        ATLENSURE_SUCCEEDED(hresult);

        if (uiLessMode)
        {
            hresult = m_pThreadMgr->ActivateEx(&m_tfClientId, TF_TMAE_UIELEMENTENABLEDONLY);
        }
        else
        {
            hresult = m_pThreadMgr->Activate(&m_tfClientId);
        }
        ATLENSURE_SUCCEEDED(hresult);
        m_initialized = true;
        return S_OK;
    }
    catch (CAtlException &atlException)
    {
        log_error("Fatal error: Initialize TSF failed: {}", ToErrorMessage(atlException.m_hr));
    }
    return E_FAIL;
}

LIBC_NAMESPACE::Tsf::TsfSupport::~TsfSupport()
{
    CoUninitialize();
}