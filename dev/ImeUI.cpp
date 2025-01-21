#include "ImeUI.hpp"
#include <iostream>

IMEUIDATA    gImeUIData; // IMEUI's global data.
static DWORD CompColor[4] = {RGB(255, 0, 0), RGB(255, 0, 255), RGB(0, 0, 255), RGB(0, 255, 0)};

void         ImeUI::StartComposition(HWND hwnd)
{
    gImeUIData.uCompLen = 0; // length of composition string.
    gImeUIData.ImeState |= IME_IN_COMPOSITION;
}

void ImeUI::EndComposition(HWND hWnd)
{
    gImeUIData.uCompLen = 0;
    gImeUIData.ImeState &= ~IME_IN_COMPOSITION;
}

void ImeUI::GetResultStr(HWND hWnd)
{
    LONG   bufLen;      // Storage for length of result str.
    LPSTR  lpResultStr; // Pointer to result string.
    HIMC   hIMC;        // Input context handle.
    HLOCAL hMem;        // Memory handle.

    //
    // If fail to get input context handle then do nothing.
    //

    if (!(hIMC = ImmGetContext(hWnd))) return;

    //
    // Determines how much memory space to store the result string.
    // Applications should call ImmGetCompositionString with
    // GCS_RESULTSTR flag on, buffer length zero, to get the bullfer
    // length.
    //

    if ((bufLen = ImmGetCompositionStringA(hIMC, GCS_RESULTSTR, (void *)NULL, (DWORD)0)) <= 0) goto exit2;

    //
    // Allocates memory with bufLen+1 bytes to store the result
    // string. Here we allocale on more byte to put null character.
    //

    if (!(hMem = LocalAlloc(LPTR, (int)bufLen + 1))) goto exit2;

    if (!(lpResultStr = (LPSTR)LocalLock(hMem))) goto exit1;

    //
    // Reads in the result string.
    //

    ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, lpResultStr, bufLen);

    //
    // Displays the result string.
    //
    DisplayResultString(hWnd, lpResultStr);

    LocalUnlock(hMem);

exit1:
    LocalFree(hMem);

exit2:
    ImmReleaseContext(hWnd, hIMC);
}

void ImeUI::GetCompositionStr(HWND hwnd, LPARAM lParam, std::wstring &compStr)
{
    LONG   bufLen;          // Stogare for len. of composition str
    LPWSTR lpwCompStr;      // Pointer to composition str.
    HIMC   hIMC;            // Input context handle.
    HLOCAL hMem;            // Memory handle.
    HLOCAL hMemAttr = NULL; // Memory handle for comp. str. array.
    LONG   bufLenAttr;

    if (!(hIMC = ImmGetContext(hwnd))) return;
    bufLen = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, (void *)NULL, 0l);
    if (bufLen <= 0)
    {
        goto exit2;
    }
    if (!(hMem = LocalAlloc(LPTR, (int)bufLen + 2))) goto exit2;
    if (!(lpwCompStr = (LPWSTR)LocalLock(hMem))) goto exit1;
    ImmGetCompositionStringW(hIMC, GCS_COMPSTR, lpwCompStr, bufLen);
    lpwCompStr[bufLen / 2] = 0;
    compStr.assign(lpwCompStr);
    std::wcout << compStr << L", comp size: " << compStr.size() << L", " << bufLen / 2 << std::endl;
    LocalUnlock(hMem);
exit1:
    LocalFree(hMem);
exit2:
    ImmReleaseContext(hwnd, hIMC);
}

void ImeUI::DisplayCompString(HWND hwnd, LPSTR lpStr, LPSTR lpStrAttr)
{
    LOG(info, "CompString: lpStr: {}, kpStrAttr: {}", lpStr, lpStrAttr);
}

void ImeUI::DisplayResultString(HWND hwnd, LPSTR lpStr)
{
    // MultiByteToWideChar()
    LOG(info, "result string: len {}, {}", lstrlenA(lpStr), lpStr);
    gImeUIData.uCompLen = 0;
}
