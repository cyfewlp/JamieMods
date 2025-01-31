//
// Created by jamie on 2025/1/28.
//

#include "LangProfileUtil.h"
#include "WCharUtils.h"
#include <atlbase.h>
#include <tchar.h>
#include <wincodec.h>

#define SAFE_CALL(call, error)                                                                                         \
    if (FAILED(hr = call)) throw std::runtime_error(error);
#define SAFE_RELEASE_PTR(resource)                                                                                     \
    if (resource != nullptr)                                                                                           \
    {                                                                                                                  \
        resource->Release();                                                                                           \
        resource = nullptr;                                                                                            \
    }
#define CO_CREATE(Type, var, IID)                                                                                      \
    CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID, (VOID **)&var)

#define CO_CREATE(Type, var)                                                                                           \
    CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_##Type, (VOID **)&var)

namespace SimpleIME
{
    LangProfileUtil::LangProfileUtil()
    {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr))
        {
            throw std::runtime_error("Initlalize COM failed.");
        }

        initialized = TRUE;
    }

    LangProfileUtil::~LangProfileUtil()
    {
        if (initialized)
        {
            CoUninitialize();
        }
    }

    BOOL LangProfileUtil::LoadIme(__in std::vector<LangProfile> &langProfiles) noexcept(false)
    {
        BOOL    result = TRUE;
        HRESULT hr     = TRUE;
        try
        {
            CComPtr<ITfInputProcessorProfileMgr> lpProfileMgr;
            SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpProfileMgr), "TSF: Create profile manager failed.");

            CComPtr<ITfInputProcessorProfiles> lpProfiles;
            SAFE_CALL(CO_CREATE(ITfInputProcessorProfiles, lpProfiles), "TSF: Create profile failed.");

            CComPtr<IEnumTfInputProcessorProfiles> lpEnum;
            SAFE_CALL(lpProfileMgr->EnumProfiles(0, &lpEnum), "Can't enum language profiles");

            TF_INPUTPROCESSORPROFILE profile = {0};
            ULONG                    fetched = 0;
            while (lpEnum->Next(1, &profile, &fetched) == S_OK)
            {
                BOOL bEnabled = FALSE;
                BSTR bstrDesc = NULL;
                hr =
                    lpProfiles->IsEnabledLanguageProfile(profile.clsid, profile.langid, profile.guidProfile, &bEnabled);

                if (SUCCEEDED(hr) && bEnabled)
                {
                    hr = lpProfiles->GetLanguageProfileDescription(profile.clsid, profile.langid, profile.guidProfile,
                                                                   &bstrDesc);
                    if (SUCCEEDED(hr))
                    {
                        LangProfile langProfile = {};
                        langProfile.clsid       = profile.clsid;
                        langProfile.langid      = profile.langid;
                        langProfile.guidProfile = profile.guidProfile;
                        langProfile.desc        = WCharUtils::ToString(bstrDesc);
                        langProfiles.push_back(langProfile);
                        logv(info, "Load installed ime: {}", langProfile.desc.c_str());
                        SysFreeString(bstrDesc);
                    }
                }
            }
        }
        catch (std::runtime_error error)
        {
            logv(err, "LoadIme failed: {}", error.what());
        }
        return result;
    }

    BOOL LangProfileUtil::ActiveProfile(_In_ LangProfile &profile)
    {
        BOOL                                 result = FALSE;
        CComPtr<ITfInputProcessorProfileMgr> lpProfileMgr;
        HRESULT                              hr = TRUE;
        try
        {
            SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpProfileMgr), "TSF: Create profile failed.");

            hr = lpProfileMgr->ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, profile.langid, profile.clsid,
                                               profile.guidProfile, NULL,
                                               TF_IPPMF_FORSESSION | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE);
            if (FAILED(hr))
            {
                throw std::runtime_error("Active profile failed.");
            }
        }
        catch (std::runtime_error error)
        {
            logv(err, "Set Ime failed: {}", error.what());
        }
        return SUCCEEDED(hr);
    }

    BOOL LangProfileUtil::LoadActiveIme(__in GUID &a_guidProfile)
    {
        HRESULT                              hr = TRUE;
        CComPtr<ITfInputProcessorProfileMgr> lpMgr;
        try
        {
            SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpMgr), "create profile manager failed.");

            LANGID                   langId = NULL;
            TF_INPUTPROCESSORPROFILE profile;
            hr = lpMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile);
            if (SUCCEEDED(hr))
            {
                a_guidProfile = profile.guidProfile;
                return true;
            }
        }
        catch (std::runtime_error error)
        {
            logv(err, "load active ime failed: {}", error.what());
        }
        return false;
    }
}
