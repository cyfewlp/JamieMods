//
// Created by jamie on 2025/1/28.
//
#pragma once

#include <msctf.h>
#include <vector>
#include <windows.h>

namespace SimpleIME
{
    struct ImeProfile
    {
        CLSID       clsid;
        LANGID      langid;
        GUID        guidProfile;
        std::string desc;
    };

    class SystemImeLoader
    {
    public:
        SystemImeLoader();
        ~SystemImeLoader();
        BOOL LoadIme(__out std::vector<ImeProfile> &imeNames) noexcept(false);
        BOOL ActiveProfile(_In_ ImeProfile &profile);
        BOOL LoadActiveIme();

    private:
        BOOL initialized = FALSE;
    };
}
