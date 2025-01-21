#pragma once

#ifndef _TRANSMOGRIFY_UTILS
    #define _TRANSMOGRIFY_UTILS

    #include <iostream>
    #include <vector>
    #include <windows.h>

constexpr auto DEFAULT_CAND_NUM_PER_PAGE = 5;

namespace Transmogrify {
    namespace utils {
        std::string WideStringToString(const std::wstring wstr);

        void GetCandStrings(HWND hWnd, LPCANDIDATELIST lpCandList, std::vector<std::wstring>* candStrList);

        std::string WideStrToStrCodePage(const std::wstring& wstr, UINT CodePage);

        std::string WideStrToStrUTF8(const std::wstring& wstr);

        void NormalizeTest(LPCWSTR lpStr, UINT CodePage = CP_UTF8);
    };
};

#endif // _TRANSMOGRIFY_UTILS
