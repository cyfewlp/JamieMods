#include "utils.hpp"

std::string
Transmogrify::utils::WideStringToString(const std::wstring wstr)
{
    LPSTR buffer = nullptr;
    int targetLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buffer, 0, NULL, NULL);
    buffer = new CHAR[targetLen + 1];
    buffer[targetLen] = 0;
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buffer, targetLen, NULL, NULL);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

void
Transmogrify::utils::GetCandStrings(HWND hWnd, LPCANDIDATELIST lpCandList, std::vector<std::wstring>* candStrList)
{
    LPWCH lpStr;        // Storage for LP to a string.
    DWORD dwNumPerPage; // Storage for num per page
    DWORD dwStartIndex; // Storage for candidate index
    DWORD dwEndIndex;   // Storage for candidate index
    int y = 0;          //

    dwNumPerPage = (!lpCandList->dwPageSize) ? DEFAULT_CAND_NUM_PER_PAGE : lpCandList->dwPageSize;
    dwStartIndex = lpCandList->dwPageStart;
    dwEndIndex = dwStartIndex + dwNumPerPage;

    for (; dwStartIndex < dwEndIndex; dwStartIndex++, y++) {
        lpStr = (LPWCH)((LPSTR)lpCandList + lpCandList->dwOffset[dwStartIndex]);
        candStrList->push_back(lpStr);
    }
}

std::string
Transmogrify::utils::WideStrToStrCodePage(const std::wstring& wstr, UINT CodePage)
{
    int size_needed = WideCharToMultiByte(CodePage, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CodePage, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::string
Transmogrify::utils::WideStrToStrUTF8(const std::wstring& wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

void
Transmogrify::utils::NormalizeTest(LPCWSTR lpStr, UINT CodePage)
{
    NORM_FORM form = NORM_FORM::NormalizationC;
    const int maxIterations = 10;
    LPWSTR strResult = NULL;
    HANDLE hHeap = GetProcessHeap();

    int iSizeEstimated = NormalizeString(form, lpStr, -1, NULL, 0);
    for (int i = 0; i < maxIterations; i++) {
        if (strResult)
            HeapFree(hHeap, 0, strResult);
        strResult = (LPWSTR)HeapAlloc(hHeap, 0, iSizeEstimated * sizeof(WCHAR));
        iSizeEstimated = NormalizeString(form, lpStr, -1, strResult, iSizeEstimated);

        if (iSizeEstimated > 0)
            break; // success

        if (iSizeEstimated <= 0) {
            DWORD dwError = GetLastError();
            if (dwError != ERROR_INSUFFICIENT_BUFFER)
                break; // Real error, not buffer error

            // New guess is negative of the return value.
            iSizeEstimated = -iSizeEstimated;
        }
    }
    printf("normalized: %s\n", WideStrToStrCodePage(strResult, CodePage).c_str());
    HeapFree(hHeap, 0, strResult);
}
