﻿//
// Created by jamie on 2025/1/28.
//
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
        };

        class LangProfileUtil
        {
        public:
            LangProfileUtil();
            ~LangProfileUtil();
            static void LoadIme(__in std::vector<LangProfile> &langProfiles) noexcept;
            static void ActivateProfile(_In_ LangProfile &profile) noexcept;
            static bool LoadActiveIme(__in GUID &a_guidProfile) noexcept;

        private:
            bool initialized = false;
        };
    }
}