//
// Created by jamie on 2025/1/28.
//

#include "SystemImeLoader.h"
#include "WCharUtils.h"
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
    SystemImeLoader::SystemImeLoader()
    {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr))
        {
            throw std::runtime_error("Initlalize COM failed.");
        }

        initialized = TRUE;
    }

    SystemImeLoader::~SystemImeLoader()
    {
        logv(info, "Release COM resourses\n");
        if (initialized)
        {
            CoUninitialize();
        }
    }

    BOOL SystemImeLoader::LoadIme(__out std::vector<ImeProfile> &imeNames) noexcept(false)
    {
        ITfInputProcessorProfileMgr   *lpProfileMgr = nullptr;
        ITfInputProcessorProfiles     *lpProfiles   = nullptr;
        IEnumTfInputProcessorProfiles *lpEnum       = nullptr;
        BOOL                           result       = TRUE;
        try
        {
            HRESULT hr = TRUE;
            SAFE_CALL(CO_CREATE(ITfInputProcessorProfileMgr, lpProfileMgr), "TSF: Create profile manager failed.");
            SAFE_CALL(CO_CREATE(ITfInputProcessorProfiles, lpProfiles), "TSF: Create profile failed.");
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
                        ImeProfile imeProfile  = {};
                        imeProfile.clsid       = profile.clsid;
                        imeProfile.langid      = profile.langid;
                        imeProfile.guidProfile = profile.guidProfile;
                        imeProfile.desc        = WCharUtils::ToString(bstrDesc);
                        imeNames.push_back(imeProfile);
                        SysFreeString(bstrDesc);
                    }
                }
            }
        }
        catch (std::runtime_error error)
        {
            logv(err, "LoadIme failed: {}", error.what());
        }
        SAFE_RELEASE_PTR(lpProfileMgr);
        SAFE_RELEASE_PTR(lpProfiles);
        SAFE_RELEASE_PTR(lpEnum);
        return result;
    }

    BOOL SystemImeLoader::ActiveProfile(_In_ ImeProfile &profile)
    {
        BOOL                         result       = FALSE;
        ITfInputProcessorProfileMgr *lpProfileMgr = nullptr;
        HRESULT                      hr           = TRUE;
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
        SAFE_RELEASE_PTR(lpProfileMgr);
        return SUCCEEDED(hr);
    }

    BOOL SystemImeLoader::LoadActiveIme()
    {
        HRESULT                    hr         = TRUE;
        ITfInputProcessorProfiles *lpProfiles = nullptr;
        hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                              IID_ITfInputProcessorProfiles, (VOID **)&lpProfiles);
        SAFE_CALL(CO_CREATE(ITfInputProcessorProfiles, lpProfiles), "create nputProcessorProfiles failed.");
        LANGID langId = NULL;
        GUID   guidProfile;
        hr = lpProfiles->GetActiveLanguageProfile(GUID_TFCAT_TIP_KEYBOARD, &langId, &guidProfile);
        if (SUCCEEDED(hr))
        {
            logv(debug, "active lang {}", langId);
        }

        return 0;
    }
}
