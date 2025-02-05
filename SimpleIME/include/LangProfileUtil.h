#ifndef LANGPROFILEUTIL_H
#define LANGPROFILEUTIL_H

#pragma once

#include "Configs.h"
#include <msctf.h>
#include <vector>
#include <windows.h>

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

        class LangProfileUtil
        {
        public:
            LangProfileUtil();
            ~LangProfileUtil();
            static void LoadIme(__in std::vector<LangProfile> &langProfiles) noexcept;
            static void ActivateProfile(_In_ LangProfile &profile) noexcept;
            static auto LoadActiveIme(__in GUID &a_guidProfile) noexcept -> bool;

        private:
            bool initialized = false;
        };
    }
}
#endif
