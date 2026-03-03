#ifndef COMMON_WCHARUTILS_H
#define COMMON_WCHARUTILS_H

#pragma once

#include <string>
#include <windows.h>

class WCharUtils
{
public:
    static auto ToString(const std::wstring_view &wstrv, const UINT codePage = CP_UTF8) -> std::string
    {
        const int size_needed = WideCharToMultiByte(codePage, 0, wstrv.data(), static_cast<int>(wstrv.size()), nullptr, 0, nullptr, nullptr);
        if (size_needed > 0)
        {
            std::string strTo;
            strTo.resize(static_cast<size_t>(size_needed));
            int written = WideCharToMultiByte(codePage, 0, wstrv.data(), static_cast<int>(wstrv.size()), strTo.data(), size_needed, nullptr, nullptr);
            if (written > 0)
            {
                return strTo;
            }
        }
        return {};
    }

    static auto ToString(const wchar_t *pwsz, const int charSize, const UINT codePage = CP_UTF8) -> std::string
    {
        std::string strTo;
        const int   size_needed = WideCharToMultiByte(codePage, 0, pwsz, charSize, nullptr, 0, nullptr, nullptr);
        if (size_needed > 0)
        {
            strTo.resize(static_cast<size_t>(size_needed));
            WideCharToMultiByte(codePage, 0, pwsz, charSize, strTo.data(), size_needed, nullptr, nullptr);
        }
        return strTo;
    }

    static constexpr auto RequiredByteLength(const LPCWCH lpcWch, const UINT codePage = CP_UTF8) -> int
    {
        return WideCharToMultiByte(codePage, 0, lpcWch, -1, nullptr, 0, nullptr, nullptr);
    }

    static constexpr void ToString(const LPCWCH lpcwch, __out LPSTR lpStr, const int length, UINT codePage = CP_UTF8)
    {
        WideCharToMultiByte(codePage, 0, lpcwch, -1, lpStr, length, nullptr, nullptr);
    }

    static auto ToString(const wchar_t *pwsz, const int charSize, __out std::string &outStr, const UINT codePage = CP_UTF8) -> bool
    {
        const int size_needed = WideCharToMultiByte(codePage, 0, pwsz, charSize, nullptr, 0, nullptr, nullptr);
        if (size_needed > 0)
        {
            const size_t oldSize = outStr.size();
            outStr.resize(oldSize + static_cast<size_t>(size_needed));
            return WideCharToMultiByte(codePage, 0, pwsz, charSize, outStr.data() + oldSize, size_needed, nullptr, nullptr) != 0;
        }
        return false;
    }
};

#endif
