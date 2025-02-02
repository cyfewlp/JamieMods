#pragma once

#include <string.h>
#include <stringapiset.h>

namespace LIBC_NAMESPACE_DECL
{
    class WCharUtils
    {
    public:
        static std::string ToString(const std::wstring &wstr, UINT codePage = CP_UTF8)
        {
            int size_needed =
                ::WideCharToMultiByte(codePage, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
            std::string strTo(size_needed, 0);
            ::WideCharToMultiByte(codePage, 0, wstr.data(), (int)wstr.size(), strTo.data(), size_needed, nullptr,
                                  nullptr);
            return strTo;
        }

        static int CharLength(const std::wstring &wstr, UINT codePage = CP_UTF8)
        {
            return ::WideCharToMultiByte(codePage, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr,
                                         nullptr);
        }

        static void ToString(const std::wstring &wstr, LPSTR lpStr, int length, UINT codePage = CP_UTF8)
        {
            ::WideCharToMultiByte(codePage, 0, wstr.data(), static_cast<int>(wstr.size()), lpStr, length, nullptr,
                                  nullptr);
        }
    };
}