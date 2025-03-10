#ifndef TSF_LANGPROFILEUTIL_H
#define TSF_LANGPROFILEUTIL_H

#pragma once

#include "TsfSupport.h"
#include "core/State.h"

#include <msctf.h>
#include <unordered_map>
#include <windows.h>

namespace std
{
    template <>
    struct hash<GUID>
    {
        auto operator()(const GUID &guid) const noexcept -> std::size_t
        {
            std::size_t const h1            = std::hash<uint32_t>()(guid.Data1);
            std::size_t const h2            = std::hash<uint16_t>()(guid.Data2);
            std::size_t const h3            = std::hash<uint16_t>()(guid.Data3);
            uint64_t          data4Combined = 0;
            std::memcpy(&data4Combined, guid.Data4, sizeof(data4Combined));
            std::size_t const h4 = std::hash<uint64_t>()(data4Combined);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
} // namespace std

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        struct LangProfile
        {
            CLSID       clsid{};
            LANGID      langid{};
            GUID        guidProfile{};
            std::string desc{};
        } PACKED_ALIGN(128);

        class LangProfileUtil : public ITfInputProcessorProfileActivationSink
        {
            using State = Ime::Core::State;

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
            auto LoadActiveIme() noexcept -> bool;
            auto ActivateProfile(_In_ const GUID *guidProfile) -> bool;

            auto GetActivatedLangProfile() -> GUID &;
            auto AddRef() -> ULONG override;
            auto Release() -> ULONG override;

            [[nodiscard]] auto GetLangProfiles() -> std::unordered_map<GUID, LangProfile>;

            [[nodiscard]] constexpr auto IsAnyProfileActivated() const -> bool
            {
                return m_activatedProfile != GUID_NULL;
            }

            static constexpr auto LANGID_ENG = 0x409;

        private:
            auto QueryInterface(const IID &riid, void **ppvObject) -> HRESULT override;
            auto OnActivated(DWORD dwProfileType, LANGID langid, const IID &clsid, const GUID &catid,
                             const GUID &guidProfile, HKL hkl, DWORD dwFlags) -> HRESULT override;
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
} // namespace LIBC_NAMESPACE_DECL
#endif
