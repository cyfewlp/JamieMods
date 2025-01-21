//
// Created by jamie on 25-1-21.
//

#ifndef HELLOWORLD_WCHARUTILS_H
#define HELLOWORLD_WCHARUTILS_H

#include "winnls.h"

class WCharUtils
{
public:
    static std::string ToString(const std::wstring &wstr, UINT codePage = CP_UTF8)
    {
        int         size_needed = WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }
};

#endif // HELLOWORLD_WCHARUTILS_H
