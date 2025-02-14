#ifndef LANGPROFILEUTIL_H
#define LANGPROFILEUTIL_H

#pragma once

#include "configs/Configs.h"
#include "TsfSupport.h"
#include <msctf.h>
#include <unordered_map>
#include <windows.h>

namespace std
{
    template <>
    struct hash<GUID>
    {
        std::size_t operator()(const GUID &guid) const noexcept
        {
            std::size_t h1            = std::hash<uint32_t>()(guid.Data1);
            std::size_t h2            = std::hash<uint16_t>()(guid.Data2);
            std::size_t h3            = std::hash<uint16_t>()(guid.Data3);
            uint64_t    data4Combined = 0;
            std::memcpy(&data4Combined, guid.Data4, sizeof(data4Combined));
            std::size_t h4 = std::hash<uint64_t>()(data4Combined);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
}

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {

        struct LangProfile
        {
            CLSID       clsid;
            LANGID      langid;
            GUID        guidProfile;
            std::string desc;
        } __attribute__((packed)) __attribute__((aligned(128)));

        class LangProfileUtil final : public ITfInputProcessorProfileActivationSink
        {
        public:
            LangProfileUtil() = default;
            virtual ~LangProfileUtil();

            bool                  Initialize(TsfSupport &tsfSupport);

            bool                  LoadAllLangProfiles() noexcept;
            auto                  LoadActiveIme() noexcept -> bool;
            bool                  ActivateProfile(_In_ const GUID *guidProfile) noexcept;

            auto                  GetActivatedLangProfile() -> GUID &;

            [[nodiscard]] auto    GetLangProfiles() -> std::unordered_map<GUID, LangProfile>;

            static constexpr auto LANGID_ENG = 0x409;

        private:
            HRESULT QueryInterface(const IID &riid, void **ppvObject) override;
            ULONG   AddRef() override;
            ULONG   Release() override;
            HRESULT OnActivated(DWORD dwProfileType, LANGID langid, const IID &clsid, const GUID &catid,
                                const GUID &guidProfile, HKL hkl, DWORD dwFlags) override;

            bool    initialized_ = false;
            DWORD   refCount_;
            CComPtr<ITfInputProcessorProfileMgr>  lpProfileMgr_;
            CComPtr<ITfThreadMgr>                 lpThreadMgr_;
            DWORD                                 textCookie_;
            std::unordered_map<GUID, LangProfile> m_langProfiles;
            GUID                                  m_activatedProfile = GUID_NULL;
        };
    }
}
#endif
