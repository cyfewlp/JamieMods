#include "tsf/InputMethodManager.h"

#include "common/WCharUtils.h"
#include "common/log.h"
#include "core/State.h"

#include <atlcomcli.h>
#include <future>
#include <msctf.h>
#include <stdexcept>

namespace
{
auto GetProfileCachedIndex(const std::vector<Ime::LangProfile> &langProfiles, const GUID &guidProfile) -> std::uint32_t
{
    const auto it = std::ranges::find_if(langProfiles, [&](const auto &p) {
        return p.guidProfile == guidProfile;
    });

    if (it != langProfiles.end())
    {
        return static_cast<std::uint32_t>(std::ranges::distance(langProfiles.begin(), it));
    }
    return UINT32_MAX;
}
} // namespace

auto Ime::InputMethodManager::Initialize(ITfThreadMgrEx *lpThreadMgr) -> HRESULT
{
    _ATL_COM_BEGIN
    logger::debug("Initializing LangProfileUtil...");
    HRESULT hresult = lpThreadMgr->QueryInterface(IID_PPV_ARGS(&m_lpThreadMgr));
    ATLENSURE_SUCCEEDED(hresult);

    if (CComQIPtr<ITfSource> const lpSource(m_lpThreadMgr); lpSource != nullptr)
    {
        hresult = lpSource->AdviseSink(IID_ITfInputProcessorProfileActivationSink, this, &m_dwCookie);
        ATLENSURE_RETURN(SUCCEEDED(hresult));

        hresult = m_tfProfileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER);
        ATLENSURE_RETURN(SUCCEEDED(hresult));
        m_initialized = true;
        RefreshProfiles();
        UpdateActiveProfile();
        return S_OK;
    }
    _ATL_COM_END
}

auto Ime::InputMethodManager::UnInitialize() -> void
{
    if (m_lpThreadMgr != nullptr)
    {
        if (CComQIPtr<ITfSource> const lpSource(m_lpThreadMgr); lpSource != nullptr)
        {
            lpSource->UnadviseSink(m_dwCookie);
        }
        m_tfProfileMgr.Release();
        m_lpThreadMgr.Release();
    }
    m_initialized = false;
}

auto Ime::InputMethodManager::RefreshProfiles() -> bool
{
    m_langProfiles.clear();
    m_langProfiles.push_back(DEFAULT_LANG_PROFILE);

    HRESULT hresult = TRUE;
    try
    {
        _tsetlocale(LC_ALL, _T(""));
        CComPtr<ITfInputProcessorProfiles> lpProfiles;
        hresult = lpProfiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER);
        Tsf::throw_fail(hresult, "Failed create ITfInputProcessorProfiles");

        CComPtr<IEnumTfInputProcessorProfiles> lpEnum;
        hresult = m_tfProfileMgr->EnumProfiles(0, &lpEnum);
        Tsf::throw_fail(hresult, "Failed enum language profiles");

        TF_INPUTPROCESSORPROFILE profile = {};
        ULONG                    fetched = 0;
        while (lpEnum->Next(1, &profile, &fetched) == S_OK)
        {
            if ((profile.dwFlags & TF_IPP_FLAG_ENABLED) == 0) continue;

            BOOL     bEnabled = FALSE;
            CComBSTR bStrDesc = nullptr;

            // Skip profile that failed to load.
            auto hr =
                lpProfiles->IsEnabledLanguageProfile(profile.clsid, profile.langid, profile.guidProfile, &bEnabled);

            if (SUCCEEDED(hr) && bEnabled == TRUE)
            {
                hr = lpProfiles->GetLanguageProfileDescription(
                    profile.clsid, profile.langid, profile.guidProfile, &bStrDesc
                );
                if (SUCCEEDED(hr))
                {
                    std::string desc;
                    if (WCharUtils::ToString(bStrDesc, bStrDesc.Length(), desc))
                    {
                        m_langProfiles.emplace_back(
                            std::move(desc), profile.clsid, profile.guidProfile, profile.langid
                        );
                        logger::info("Load installed ime: {}", desc.c_str());
                    }
                }
            }
        }
    }
    catch (std::runtime_error &error)
    {
        logger::error("LoadIme failed: {}", error.what());
    }
    return SUCCEEDED(hresult);
}

auto Ime::InputMethodManager::UpdateActiveProfile() noexcept -> bool
{
    m_activatedProfile = 0;
    TF_INPUTPROCESSORPROFILE profile;
    if (SUCCEEDED(m_tfProfileMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile)))
    {
        m_activatedProfile = GetProfileCachedIndex(m_langProfiles, profile.guidProfile);
        Core::State::GetInstance().Set(State::LANG_PROFILE_ACTIVATED, m_activatedProfile < m_langProfiles.size());
        return true;
    }

    logger::error("Load active language profile failed.");
    return false;
}

auto Ime::InputMethodManager::ActivateProfile(const GUID &guidProfile) -> HRESULT
{
    const auto index = GetProfileCachedIndex(m_langProfiles, guidProfile);
    if (index > m_langProfiles.size())
    {
        return E_INVALIDARG;
    }
    auto &activeLangProfile = GetActiveLangProfile();

    auto &langProfile = m_langProfiles[index];
    if (IsEqualGUID(langProfile.guidProfile, activeLangProfile.guidProfile))
    {
        return S_OK;
    }
    const HRESULT hresult = m_tfProfileMgr->ActivateProfile(
        TF_PROFILETYPE_INPUTPROCESSOR,
        langProfile.langid,
        langProfile.clsid,
        langProfile.guidProfile,
        nullptr,
        TF_IPPMF_FORSESSION | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE
    );
    if (FAILED(hresult))
    {
        logger::error("Active profile {} failed: {}", langProfile.desc, Tsf::ToErrorMessage(hresult));
    }
    return hresult;
}

auto Ime::InputMethodManager::GetActiveLangProfile() -> const LangProfile &
{
    if (m_activatedProfile > m_langProfiles.size())
    {
        return DEFAULT_LANG_PROFILE;
    }
    return m_langProfiles[m_activatedProfile];
}

auto Ime::InputMethodManager::QueryInterface(const IID &riid, void **ppvObject) -> HRESULT
{
    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfInputProcessorProfileActivationSink))
    {
        *ppvObject = static_cast<ITfInputProcessorProfileActivationSink *>(this);
    }

    if (*ppvObject != nullptr)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

auto Ime::InputMethodManager::AddRef() -> ULONG
{
    return ++m_refCount;
}

auto Ime::InputMethodManager::Release() -> ULONG
{
    --m_refCount;
    if (m_refCount == 0)
    {
        delete this;
        return 0;
    }
    return m_refCount;
}

auto Ime::InputMethodManager::OnActivated(
    [[maybe_unused]] DWORD dwProfileType, [[maybe_unused]] LANGID langid, [[maybe_unused]] const IID &clsid,
    [[maybe_unused]] const GUID &catid, const GUID &guidProfile, [[maybe_unused]] HKL hkl, DWORD dwFlags
) -> HRESULT
{
    if ((dwFlags & TF_IPSINK_FLAG_ACTIVE) != 0)
    {
        auto &state = State::GetInstance();

        m_activatedProfile = GetProfileCachedIndex(m_langProfiles, guidProfile);

        state.Set(State::LANG_PROFILE_ACTIVATED, m_activatedProfile < m_langProfiles.size());
        state.Clear(State::IN_ALPHANUMERIC);
        state.Clear(State::IN_CAND_CHOOSING);
        state.Clear(State::IN_COMPOSING);
    }
    return S_OK;
}
