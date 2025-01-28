//
// Created by jamie on 2025/1/28.
//
#pragma once

#ifndef HELLOWORLD_SYSTEMIMELOADER_H
    #define HELLOWORLD_SYSTEMIMELOADER_H

    #include <msctf.h>
    #include <vector>
    #include <windows.h>

namespace SimpleIME
{
    class SystemImeLoader
    {
    public:
        SystemImeLoader();
        ~SystemImeLoader();
        BOOL LoadIme(__in int layoutId, __out std::vector<std::wstring> &imeNames);
        BOOL SetIme(_In_ LPCTSTR name);

    private:
        BOOL initialized = FALSE;
    };
}

#endif // HELLOWORLD_SYSTEMIMELOADER_H
