#ifndef COMMON_WCHARUTILS_H
#define COMMON_WCHARUTILS_H

#pragma once

#include "common/config.h"

#include <string>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
class WCharUtils
{
public:
    static auto ToString(const std::wstring &wstr, const UINT codePage = CP_UTF8) -> std::string
    {
        int const size_needed =
            WideCharToMultiByte(codePage, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(
            codePage, 0, wstr.data(), static_cast<int>(wstr.size()), strTo.data(), size_needed, nullptr, nullptr
        );
        return strTo;
    }

    static auto ToString(const wchar_t *pwsz, const int charSize, const UINT codePage = CP_UTF8) -> std::string
    {
        int const   size_needed = WideCharToMultiByte(codePage, 0, pwsz, charSize, nullptr, 0, nullptr, nullptr);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(codePage, 0, pwsz, charSize, strTo.data(), size_needed, nullptr, nullptr);
        return strTo;
    }

    static constexpr auto RequiredByteLength(const LPCWCH lpcWch, const UINT codePage = CP_UTF8) -> int
    {
        return WideCharToMultiByte(codePage, 0, lpcWch, -1, nullptr, 0, nullptr, nullptr);
    }

    static constexpr void ToString(const LPCWCH lpcwch, __out LPSTR lpStr, int length, UINT codePage = CP_UTF8)
    {
        WideCharToMultiByte(codePage, 0, lpcwch, -1, lpStr, length, nullptr, nullptr);
    }

    static auto ToString(
        const wchar_t *pwsz, const int charSize, __out std::string &outStr, const UINT codePage = CP_UTF8
    ) -> bool
    {
        int const size_needed = WideCharToMultiByte(codePage, 0, pwsz, charSize, nullptr, 0, nullptr, nullptr);
        if (size_needed > 0)
        {
            size_t oldSize = outStr.size();
            outStr.resize(oldSize + size_needed);
            return WideCharToMultiByte(codePage, 0, pwsz, charSize, outStr.data() + oldSize, size_needed, nullptr, nullptr) != 0;
        }
        return false;
    }
};
} // namespace LIBC_NAMESPACE_DECL

#endif
