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

    static constexpr auto RequiredByteLength(const std::wstring_view &wstringView, const UINT codePage = CP_UTF8) -> int
    {
        return WideCharToMultiByte(codePage, 0, wstringView.data(), wstringView.size(), nullptr, 0, nullptr, nullptr);
    }

    static constexpr auto ToString(const LPCWCH lpcwch, __out LPSTR lpStr, int length, UINT codePage = CP_UTF8)
    {
        return WideCharToMultiByte(codePage, 0, lpcwch, -1, lpStr, length, nullptr, nullptr);
    }

    static constexpr auto ToString(
        const std::wstring_view &&wstringView, __out std::string &outString, const UINT codePage = CP_UTF8
    )
    {
        if (const int length = RequiredByteLength(wstringView, codePage); length > 0)
        {
            outString.resize(length);
            return WideCharToMultiByte(
                codePage, 0, wstringView.data(), wstringView.size(), outString.data(), length, nullptr, nullptr
            );
        }
        return 0;
    }
};
} // namespace LIBC_NAMESPACE_DECL

#endif
