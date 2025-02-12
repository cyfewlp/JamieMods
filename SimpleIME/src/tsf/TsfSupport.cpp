#include "TsfSupport.h"
#include "WCharUtils.h"
#include <comdef.h>

#define PLUGIN_NAMESPACE LIBC_NAMESPACE::SimpleIME

std::string PLUGIN_NAMESPACE::ToErrorMessage(const HRESULT hresult)
{
    const _com_error err(hresult);
    return WCharUtils::ToString(err.ErrorMessage());
}

constexpr void PLUGIN_NAMESPACE::throw_fail(HRESULT hresult, const char *msg) noexcept(false)
{
    if (FAILED(hresult))
    {
        const auto errMsg = std::format("{} because: {}", msg, ToErrorMessage(hresult));
        throw std::runtime_error(errMsg);
    }
}

bool PLUGIN_NAMESPACE::TsfSupport::InitializeTsf()
{
    try
    {
        HRESULT hresult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hresult) && hresult != S_FALSE)
        {
            throw_fail(hresult, "Initialize LangProfileUtil failed");
            return false;
        }
        hresult = m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER);
        throw_fail(hresult, "Create ThreadMgr failed");

        hresult = m_pThreadMgr->Activate(&m_tfClientId);
        throw_fail(hresult, "Can't activate TSF for caller thread");
        return true;
    }
    catch (std::runtime_error &error)
    {
        log_error("Fatal error: Initialize TSF failed: {}", error.what());
    }
    return false;
}

PLUGIN_NAMESPACE::TsfSupport::~TsfSupport()
{
    CoUninitialize();
}