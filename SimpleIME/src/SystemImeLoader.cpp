//
// Created by jamie on 2025/1/28.
//

#include "SystemImeLoader.h"
#include <tchar.h>

SimpleIME::SystemImeLoader::SystemImeLoader()
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        throw std::runtime_error("Initlalize COM failed.");
    }

    initialized = TRUE;
}

SimpleIME::SystemImeLoader::~SystemImeLoader()
{
    logv(info, "Release COM resourses\n");
    if (initialized)
    {
        CoUninitialize();
    }
}

BOOL SimpleIME::SystemImeLoader::LoadIme(__in int layoutId, __out std::vector<std::wstring> &imeNames)
{
    logv(info, "Loading system installed ime for layoutL {}.", layoutId);
    ITfInputProcessorProfiles *lpProfiles = nullptr;
    IEnumTfLanguageProfiles   *lpEnum     = nullptr;
    BOOL                       result     = TRUE;
    try
    {
        HRESULT hr = TRUE;
        hr         = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_ITfInputProcessorProfiles, (VOID **)&lpProfiles);
        if (FAILED(hr))
        {
            throw FALSE;
        }

        hr = lpProfiles->EnumLanguageProfiles(layoutId, &lpEnum);
        if (FAILED(hr))
        {
            throw FALSE;
        }

        if (lpEnum != nullptr)
        {
            TF_LANGUAGEPROFILE profile;
            ULONG              fetched = 0;

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
                        imeNames.push_back(bstrDesc);
                        SysFreeString(bstrDesc);
                    }
                }
            }
        }
    }
    catch (int r)
    {
        if (lpProfiles != nullptr)
        {
            lpProfiles->Release();
            lpProfiles = nullptr;
        }

        if (lpEnum != nullptr)
        {
            lpEnum->Release();
            lpEnum = nullptr;
        }
    }
    return result;
}

BOOL SimpleIME::SystemImeLoader::SetIme(_In_ LPCTSTR name)
{
    BOOL                           result     = FALSE;
    ITfInputProcessorProfiles     *lpProfiles = nullptr;
    ITfInputProcessorProfileMgr   *lpMgr      = nullptr;
    IEnumTfInputProcessorProfiles *lpEnum     = nullptr;

    try
    {
        HRESULT hr = TRUE;
        hr         = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_ITfInputProcessorProfileMgr, (LPVOID *)&lpMgr);
        if (FAILED(hr))
        {
            throw FALSE;
        }

        hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                              IID_ITfInputProcessorProfiles, (LPVOID *)&lpProfiles);
        if (FAILED(hr))
        {
            throw FALSE;
        }

        hr = lpMgr->EnumProfiles(0, &lpEnum);
        if (FAILED(hr))
        {
            throw FALSE;
        }

        TF_INPUTPROCESSORPROFILE profile = {0};
        ULONG                    fetched = 0;
        while (lpEnum->Next(1, &profile, &fetched) == S_OK)
        {
            BSTR bstrDest = nullptr;
            hr = lpProfiles->GetLanguageProfileDescription(profile.clsid, profile.langid, profile.guidProfile,
                                                           &bstrDest);
            if (SUCCEEDED(hr))
            {
                if (_tcscmp(name, bstrDest) == 0)
                {
                    hr = lpMgr->ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, profile.langid, profile.clsid,
                                                profile.guidProfile, NULL,
                                                TF_IPPMF_FORSESSION | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE);

                    if (SUCCEEDED(hr))
                    {
                        result = TRUE;
                        break;
                    }
                    else
                    {
                        throw FALSE;
                    }
                }
                SysFreeString(bstrDest);
            }
            ZeroMemory(&profile, sizeof(TF_INPUTPROCESSORPROFILE));
        }
    }
    catch (int r)
    {
        if (lpProfiles != nullptr)
        {
            lpProfiles->Release();
            lpProfiles = nullptr;
        }
        if (lpEnum != nullptr)
        {
            lpEnum->Release();
            lpEnum = nullptr;
        }
        if (lpMgr != nullptr)
        {
            lpMgr->Release();
            lpMgr = nullptr;
        }
        throw r;
    }
    return result;
}
