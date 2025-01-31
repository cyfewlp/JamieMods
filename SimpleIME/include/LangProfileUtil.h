//
// Created by jamie on 2025/1/28.
//
#pragma once

#include <msctf.h>
#include <vector>
#include <windows.h>

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
        BOOL LoadIme(__in std::vector<LangProfile> &langProfiles) noexcept(false);
        BOOL ActiveProfile(_In_ LangProfile &profile);
        BOOL LoadActiveIme(__in GUID &a_guidProfile);

    private:
        BOOL initialized = FALSE;
    };
}
