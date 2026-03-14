#ifndef COMMON_WCHARUTILS_H
#define COMMON_WCHARUTILS_H

#pragma once

#include <string>
#include <windows.h>

namespace WCharUtils
{
inline auto ToString(const std::wstring_view &wstrv, const UINT codePage = CP_UTF8) -> std::string
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

} // namespace WCharUtils
#endif
