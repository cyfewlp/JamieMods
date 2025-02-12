#include "LangProfileUtil.h"
#include "Configs.h"
#include "TsfSupport.h"
#include "WCharUtils.h"
#include <atlcomcli.h>
#include <cstddef>
#include <future>
#include <msctf.h>
#include <stdexcept>
#include <unordered_map>

#define PLUGIN_NAMESPACE LIBC_NAMESPACE::SimpleIME

PLUGIN_NAMESPACE::LangProfileUtil::~LangProfileUtil()
{
    if (lpThreadMgr_ != nullptr)
    {
        CComPtr<ITfSource> lpSource;
        if (SUCCEEDED(lpThreadMgr_.QueryInterface(&lpSource)))
        {
            lpSource->UnadviseSink(textCookie_);
        }
    }
}

bool PLUGIN_NAMESPACE::LangProfileUtil::Initialize(TsfSupport &tsfSupport)
{
    try
    {
        const auto &lpThreadMgr = tsfSupport.GetThreadMgr();
        HRESULT     hresult     = lpThreadMgr.QueryInterface(&lpThreadMgr_);
        throw_fail(hresult, "Failed query interface ITfThreadMgr on thread mgr.");

        CComPtr<ITfSource> lpSource;
        hresult = lpThreadMgr.QueryInterface(&lpSource);
        throw_fail(hresult, "Failed query interface ITfSource on thread mgr.");

        hresult = lpSource->AdviseSink(IID_ITfInputProcessorProfileActivationSink, this, &textCookie_);
        throw_fail(hresult, "Can't Advise IID_ITfInputProcessorProfileActivationSink.");

        hresult = lpProfileMgr_.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER);
        throw_fail(hresult, "TSF: Create profiles manager failed");
        initialized_ = true;
        return true;
    }
    catch (std::runtime_error &error)
    {
        log_error("Can't initialize LangProfileUtil: {}", error.what());
    }
    return false;
}

bool PLUGIN_NAMESPACE::LangProfileUtil::LoadAllLangProfiles() noexcept
{
    HRESULT hresult = TRUE;
    try
    {
        _tsetlocale(LC_ALL, _T(""));
        CComPtr<ITfInputProcessorProfiles> lpProfiles;
        hresult = lpProfiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER);
        throw_fail(hresult, "TSF: Create profile failed.");

        CComPtr<IEnumTfInputProcessorProfiles> lpEnum;
        hresult = lpProfileMgr_->EnumProfiles(0, &lpEnum);
        throw_fail(hresult, "Can't enum language profiles");

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

bool PLUGIN_NAMESPACE::LangProfileUtil::ActivateProfile(_In_ const GUID *guidProfile) noexcept
{
    auto expected = m_langProfiles.find(*guidProfile);
    if (expected == m_langProfiles.end())
    {
        return false;
    }
    auto    profile = expected->second;
    HRESULT hresult = lpProfileMgr_->ActivateProfile(TF_PROFILETYPE_INPUTPROCESSOR, profile.langid, profile.clsid,
                                                     profile.guidProfile, nullptr,
                                                     TF_IPPMF_FORSESSION | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE);
    if (FAILED(hresult))
    {
        log_error("Active profile {} failed: {}", profile.desc, ToErrorMessage(hresult));
    }
    m_activatedProfile = profile.guidProfile;
    return SUCCEEDED(hresult);
}

auto PLUGIN_NAMESPACE::LangProfileUtil::LoadActiveIme() noexcept -> bool
{
    TF_INPUTPROCESSORPROFILE profile;
    HRESULT                  hresult = lpProfileMgr_->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile);
    if (SUCCEEDED(hresult))
    {
        m_activatedProfile = profile.guidProfile;
        return true;
    }

    log_error("load active ime failed: {}", ToErrorMessage(hresult));
    return false;
}

auto PLUGIN_NAMESPACE::LangProfileUtil::GetActivatedLangProfile() -> GUID &
{
    return m_activatedProfile;
}

auto PLUGIN_NAMESPACE::LangProfileUtil::GetLangProfiles() -> std::unordered_map<GUID, LangProfile>
{
    return m_langProfiles;
}

HRESULT PLUGIN_NAMESPACE::LangProfileUtil::QueryInterface(const IID &riid, void **ppvObject)
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

ULONG PLUGIN_NAMESPACE::LangProfileUtil::AddRef()
{
    return ++refCount_;
}

ULONG PLUGIN_NAMESPACE::LangProfileUtil::Release()
{
    --refCount_;
    if (refCount_ == 0)
    {
        delete this;
        return 0;
    }
    return refCount_;
}

HRESULT PLUGIN_NAMESPACE::LangProfileUtil::OnActivated(DWORD dwProfileType, LANGID langid, const IID &clsid,
                                                       const GUID &catid, const GUID &guidProfile, HKL hkl,
                                                       DWORD dwFlags)
{
    if ((dwFlags & TF_IPSINK_FLAG_ACTIVE) != 0)
    {
        m_activatedProfile = guidProfile;
    }
    return S_OK;
}
