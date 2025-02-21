#include "tsf/LangProfileUtil.h"

#include "common/WCharUtils.h"
#include "configs/Configs.h"
#include "tsf/TsfSupport.h"

#include <atlcomcli.h>
#include <future>
#include <msctf.h>
#include <stdexcept>
#include <unordered_map>

namespace LIBC_NAMESPACE_DECL
{
    auto Ime::LangProfileUtil::Initialize(ITfThreadMgrEx *lpThreadMgr) -> HRESULT
    {
        _ATL_COM_BEGIN
        log_debug("Initializing LangProfileUtil...");
        HRESULT hresult = lpThreadMgr->QueryInterface(IID_PPV_ARGS(&m_lpThreadMgr));
        ATLENSURE_SUCCEEDED(hresult);

        if (CComQIPtr<ITfSource> lpSource(m_lpThreadMgr); lpSource != nullptr)
        {
            hresult = lpSource->AdviseSink(IID_ITfInputProcessorProfileActivationSink, this, &m_dwCookie);
            ATLENSURE_RETURN(SUCCEEDED(hresult));

            hresult = m_tfProfileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER);
            ATLENSURE_RETURN(SUCCEEDED(hresult));
            initialized_ = true;
            return S_OK;
        }
        _ATL_COM_END
    }

    auto Ime::LangProfileUtil::UnInitialize() -> void
    {
        if (m_lpThreadMgr != nullptr)
        {
            if (CComQIPtr<ITfSource> lpSource(m_lpThreadMgr); lpSource != nullptr)
            {
                lpSource->UnadviseSink(m_dwCookie);
            }
            m_tfProfileMgr.Release();
            m_lpThreadMgr.Release();
        }
        initialized_ = false;
    }

    bool Ime::LangProfileUtil::LoadAllLangProfiles() noexcept
    {
        HRESULT hresult = TRUE;
        try
        {
            _tsetlocale(LC_ALL, _T(""));
            CComPtr<ITfInputProcessorProfiles> lpProfiles;
            hresult = lpProfiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER);
            Tsf::throw_fail(hresult, "TSF: Create profile failed.");

            CComPtr<IEnumTfInputProcessorProfiles> lpEnum;
            hresult = m_tfProfileMgr->EnumProfiles(0, &lpEnum);
            Tsf::throw_fail(hresult, "Can't enum language profiles");

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
                    hresult = lpProfiles->GetLanguageProfileDescription(profile.clsid, profile.langid,
                                                                        profile.guidProfile, &bstrDesc);
                    if (SUCCEEDED(hresult))
                    {
                        LangProfile langProfile             = {};
                        langProfile.clsid                   = profile.clsid;
                        langProfile.langid                  = profile.langid;
                        langProfile.guidProfile             = profile.guidProfile;
                        langProfile.desc                    = WCharUtils::ToString(bstrDesc);
                        m_langProfiles[profile.guidProfile] = langProfile;
                        log_info("Load installed ime: {}", langProfile.desc.c_str());
                        SysFreeString(bstrDesc);
                    }
                }
            }
            LangProfile engProfile = {};
            ZeroMemory(&engProfile, sizeof(engProfile));
            engProfile.clsid          = CLSID_NULL;
            engProfile.langid         = LANGID_ENG; // english keyboard
            engProfile.guidProfile    = GUID_NULL;
            engProfile.desc           = std::string("ENG");
            m_langProfiles[GUID_NULL] = engProfile;
        }
        catch (std::runtime_error &error)
        {
            log_error("LoadIme failed: {}", error.what());
        }
        return SUCCEEDED(hresult);
    }

    bool Ime::LangProfileUtil::ActivateProfile(_In_ const GUID *guidProfile) noexcept
    {
        auto expected = m_langProfiles.find(*guidProfile);
        if (expected == m_langProfiles.end())
        {
            return false;
        }
        auto    profile = expected->second;
        HRESULT hresult = m_tfProfileMgr->ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, profile.langid, profile.clsid,
                                                          profile.guidProfile, nullptr,
                                                          TF_IPPMF_FORSESSION | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE);
        if (FAILED(hresult))
        {
            log_error("Active profile {} failed: {}", profile.desc, Tsf::ToErrorMessage(hresult));
        }
        m_activatedProfile = profile.guidProfile;
        return SUCCEEDED(hresult);
    }

    auto Ime::LangProfileUtil::LoadActiveIme() noexcept -> bool
    {
        TF_INPUTPROCESSORPROFILE profile;
        if (SUCCEEDED(m_tfProfileMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile)))
        {
            m_activatedProfile = profile.guidProfile;
            return true;
        }

        log_error("Load active ime failed.");
        return false;
    }

    auto Ime::LangProfileUtil::GetActivatedLangProfile() -> GUID &
    {
        return m_activatedProfile;
    }

    auto Ime::LangProfileUtil::GetLangProfiles() -> std::unordered_map<GUID, LangProfile>
    {
        return m_langProfiles;
    }

    HRESULT Ime::LangProfileUtil::QueryInterface(const IID &riid, void **ppvObject)
    {
        *ppvObject = nullptr;

        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfInputProcessorProfileActivationSink))
        {
            *ppvObject = static_cast<ITfInputProcessorProfileActivationSink *>(this);
        }

        if (*ppvObject)
        {
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG Ime::LangProfileUtil::AddRef()
    {
        return ++refCount_;
    }

    ULONG Ime::LangProfileUtil::Release()
    {
        --refCount_;
        if (refCount_ == 0)
        {
            delete this;
            return 0;
        }
        return refCount_;
    }

    HRESULT Ime::LangProfileUtil::OnActivated(
        [[maybe_unused]] DWORD dwProfileType, [[maybe_unused]] LANGID langid, [[maybe_unused]] const IID &clsid,
        [[maybe_unused]] const GUID &catid, const GUID &guidProfile, [[maybe_unused]] HKL hkl, DWORD dwFlags)
    {
        if ((dwFlags & TF_IPSINK_FLAG_ACTIVE) != 0)
        {
            m_activatedProfile = guidProfile;
        }
        else
        {
            m_activatedProfile = GUID_NULL;
        }
        return S_OK;
    }
}
