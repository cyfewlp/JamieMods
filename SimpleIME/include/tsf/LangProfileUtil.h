#ifndef TSF_LANGPROFILEUTIL_H
#define TSF_LANGPROFILEUTIL_H

#pragma once

#include "LangProfile.h"
#include "TsfSupport.h"
#include "core/State.h"

#include <msctf.h>
#include <unordered_map>
#include <windows.h>

namespace Ime
{
class LangProfileUtil : public ITfInputProcessorProfileActivationSink
{
    using State = Core::State;

public:
    LangProfileUtil()                                                     = default;
    virtual ~LangProfileUtil()                                            = default;
    LangProfileUtil(const LangProfileUtil &other)                         = delete;
    LangProfileUtil(LangProfileUtil &&other) noexcept                     = delete;
    auto operator=(const LangProfileUtil &other) -> LangProfileUtil &     = delete;
    auto operator=(LangProfileUtil &&other) noexcept -> LangProfileUtil & = delete;

    auto Initialize(ITfThreadMgrEx *lpThreadMgr) -> HRESULT;
    auto UnInitialize() -> void;

    auto LoadAllLangProfiles() -> bool;
    auto UpdateActiveProfile() noexcept -> bool;
    auto ActivateProfile(_In_ const GUID *guidProfile) -> bool;

    auto GetActivatedLangProfile() -> GUID &;
    auto AddRef() -> ULONG override;
    auto Release() -> ULONG override;

    [[nodiscard]] auto GetLangProfiles() -> std::unordered_map<GUID, LangProfile> &;

    [[nodiscard]] constexpr auto IsAnyProfileActivated() const -> bool
    {
        return m_activatedProfile != GUID_NULL;
    }

    static constexpr auto LANGID_ENG = 0x409;

private:
    auto QueryInterface(const IID &riid, void **ppvObject) -> HRESULT override;
    auto OnActivated(
        DWORD dwProfileType, LANGID langid, const IID &clsid, const GUID &catid, const GUID &guidProfile, HKL hkl,
        DWORD dwFlags
    ) -> HRESULT override;
    void UpdateLangProfileState() const;

    bool                                  m_initialized = false;
    DWORD                                 m_refCount{};
    CComPtr<ITfInputProcessorProfileMgr>  m_tfProfileMgr = nullptr;
    CComPtr<ITfThreadMgr>                 m_lpThreadMgr  = nullptr;
    DWORD                                 m_dwCookie{};
    std::unordered_map<GUID, LangProfile> m_langProfiles;
    GUID                                  m_activatedProfile = GUID_NULL;
};
} // namespace Ime

#endif
