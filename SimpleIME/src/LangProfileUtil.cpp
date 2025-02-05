//
// Created by jamie on 2025/1/28.
//

#include "LangProfileUtil.h"
#include "Configs.h"
#include "WCharUtils.h"
#include "spdlog/common.h"
#include <atlcomcli.h>
#include <combaseapi.h>
#include <cstddef>
#include <intsafe.h>
#include <minwindef.h>
#include <msctf.h>
#include <objbase.h>
#include <oleauto.h>
#include <sal.h>
#include <specstrings_strict.h>
#include <stdexcept>
#include <winnt.h>
#include <wtypes.h>
#include <wtypesbase.h>

#define SAFE_CALL(call, error)                                                                                         \
    if (FAILED(hresult = (call))) throw std::runtime_error(error);
#define SAFE_RELEASE_PTR(resource)                                                                                     \
    if ((resource) != nullptr)                                                                                         \
    {                                                                                                                  \
        (resource)->Release();                                                                                         \
        (resource) = nullptr;                                                                                          \
    }
#define CO_CREATE(Type, var, IID)                                                                                      \
    CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID, (VOID **)&(var))

#define CO_CREATE(Type, var)                                                                                           \
    CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_##Type, (VOID **)&(var))

using namespace LIBC_NAMESPACE::SimpleIME;

LangProfileUtil::LangProfileUtil()
{
    if (FAILED(CoInitialize(nullptr)))
    {
        throw std::runtime_error("Initlalize COM failed.");
    }

    initialized = true;
}

LangProfileUtil::~LangProfileUtil()
{
    if (initialized)
    {
        CoUninitialize();
    }
}

void LangProfileUtil::LoadIme(__in std::vector<LangProfile> &langProfiles) noexcept
{
    HRESULT hresult = TRUE;
    try
    {
        CComPtr<ITfInputProcessorProfileMgr> lpProfileMgr;
        SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpProfileMgr), "TSF: Create profile manager failed.");

        CComPtr<ITfInputProcessorProfiles> lpProfiles;
        SAFE_CALL(CO_CREATE(ITfInputProcessorProfiles, lpProfiles), "TSF: Create profile failed.");

        CComPtr<IEnumTfInputProcessorProfiles> lpEnum;
        SAFE_CALL(lpProfileMgr->EnumProfiles(0, &lpEnum), "Can't enum language profiles");

        TF_INPUTPROCESSORPROFILE profile = {};
        ULONG                    fetched = 0;
        while (lpEnum->Next(1, &profile, &fetched) == S_OK)
        {
            BOOL bEnabled = FALSE;
            BSTR bstrDesc = nullptr;
            hresult =
                lpProfiles->IsEnabledLanguageProfile(profile.clsid, profile.langid, profile.guidProfile, &bEnabled);

            if (SUCCEEDED(hresult) && bEnabled == TRUE)
            {
                hresult = lpProfiles->GetLanguageProfileDescription(profile.clsid, profile.langid, profile.guidProfile,
                                                                    &bstrDesc);
                if (SUCCEEDED(hresult))
                {
                    LangProfile langProfile = {};
                    langProfile.clsid       = profile.clsid;
                    langProfile.langid      = profile.langid;
                    langProfile.guidProfile = profile.guidProfile;
                    langProfile.desc        = WCharUtils::ToString(bstrDesc);
                    langProfiles.push_back(langProfile);
                    log_info("Load installed ime: {}", langProfile.desc.c_str());
                    SysFreeString(bstrDesc);
                }
            }
        }
    }
    catch (std::runtime_error error)
    {
        log_error("LoadIme failed: {}", error.what());
    }
}

void LangProfileUtil::ActivateProfile(_In_ LangProfile &profile) noexcept
{
    CComPtr<ITfInputProcessorProfileMgr> lpProfileMgr;
    HRESULT                              hresult = TRUE;
    try
    {
        SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpProfileMgr), "TSF: Create profile failed.");

        hresult = lpProfileMgr->ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, profile.langid, profile.clsid,
                                                profile.guidProfile, NULL,
                                                TF_IPPMF_FORSESSION | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE);
        if (FAILED(hresult))
        {
            throw std::runtime_error("Active profile failed.");
        }
    }
    catch (std::runtime_error &error)
    {
        log_error("Activate lang profile {} failed: {}", profile.desc.c_str(), error.what());
    }
}

auto LangProfileUtil::LoadActiveIme(__in GUID &a_guidProfile) noexcept -> bool
{
    HRESULT                              hresult = TRUE;
    CComPtr<ITfInputProcessorProfileMgr> lpMgr;
    try
    {
        SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpMgr), "create profile manager failed.");

        TF_INPUTPROCESSORPROFILE profile;
        hresult = lpMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile);
        if (SUCCEEDED(hresult))
        {
            a_guidProfile = profile.guidProfile;
            return true;
        }
    }
    catch (std::runtime_error error)
    {
        log_error("load active ime failed: {}", error.what());
    }
    return false;
}
