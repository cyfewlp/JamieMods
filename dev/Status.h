#pragma once

#include "Windows.h"
#include <commctrl.h>

#define STATUS_ID 1000

const char     szImeNull[]       = "";
const wchar_t  szImeOpen[]       = L"On";
const wchar_t  szImeClose[]      = L"Off";
const wchar_t  szImeAlpha[]      = L"Alpha";
const wchar_t  szImeNative[]     = L"Native";
const wchar_t  szImeKatakana[]   = L"Kata";
const wchar_t  szImeHalf[]       = L"Half";
const wchar_t  szImeFull[]       = L"Full";
const wchar_t  szImeRoman[]      = L"Roman";
const wchar_t  szImeCode[]       = L"Code";
const wchar_t  szImeHanja[]      = L"Hanja";
const wchar_t  szImeSoftKbd[]    = L"SoftKbd";
const wchar_t  szImeNoConv[]     = L"NoConv";
const wchar_t  szImeEUDC[]       = L"EUDC";
const wchar_t  szImeSymbol[]     = L"Symbol";

constexpr auto SBITEM_OPENSTATUS = 0;
#define SBITEM_NATIVEMODE   1
#define SBITEM_FULLSHAPE    2
#define SBITEM_ROMAN        3
#define SBITEM_CHARCODE     4
#define SBITEM_HANJA        5
#define SBITEM_SOFTKBD      6
#define SBITEM_NOCONVERSION 7
#define SBITEM_EUDC         8
#define SBITEM_SYMBOL       9

#define SBITEM_CMODEFIRST   1
#define SBITEM_CMODELAST    9

#define NUM_PARTS           10
int            nPartsWidthTbl[NUM_PARTS] = {30, 50, 30, 30, 30, 30, 30, 30, 30, 30};

HWND           hStatusWnd;

static LRESULT CreateStatus(HWND hwnd, HINSTANCE hinst)
{
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccx.dwICC  = ICC_BAR_CLASSES;
    if (!InitCommonControlsEx(&iccx)) return FALSE;

    hStatusWnd = CreateWindowExW(0,                                           // no extended styles
                                 STATUSCLASSNAME,                             // name of status bar class
                                 (PCTSTR)NULL,                                // no text when first created
                                 SBARS_SIZEGRIP |                             // includes a sizing grip
                                     WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, // creates a visible child window
                                 0, 0, 0, 0,                                  // ignores size and position
                                 hwnd,                                        // handle to parent window
                                 (HMENU)STATUS_ID,                            // child window identifier
                                 hinst,                                       // handle to application instance
                                 NULL);                                       // no window creation data
    if (!hStatusWnd) return false;
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int *pPartsWidth = (int *)LocalAlloc(LPTR, sizeof(int) * NUM_PARTS);
    if (pPartsWidth == nullptr) return false;
    int nWidth    = rcClient.right / NUM_PARTS;
    int rightEdge = nWidth;
    for (int i = 0; i < NUM_PARTS; i++)
    {
        rightEdge += nPartsWidthTbl[i];
        pPartsWidth[i] = nWidth;
    }

    SendMessage(hStatusWnd, SB_SETPARTS, (WPARAM)NUM_PARTS, (LPARAM)pPartsWidth);

    LocalFree((HANDLE)pPartsWidth);
    return true;
}

static void SetStatusItems(HWND hwnd);
static void SetOpenStatusParts(BOOL fOpen);
static void SetConvModeParts(DWORD dwConvMode);
static void ClearConvModeParts();

static void SetStatusItems(HWND hwnd)
{
    HIMC  himc;
    DWORD dwConvMode, dwSentMode;
    if (!(himc = ImmGetContext(hwnd))) return;
    bool open = ImmGetOpenStatus(himc);
    SetOpenStatusParts(open);
    if (ImmGetOpenStatus(himc))
    {
        ImmGetConversionStatus(himc, &dwConvMode, &dwSentMode);
        SetConvModeParts(dwConvMode);
    }
    else
    {
        ClearConvModeParts();
    }

    ImmReleaseContext(hwnd, himc);
}

void SetOpenStatusParts(BOOL fOpen)
{
    if (fOpen)
    {
        SendMessageW(hStatusWnd, SB_SETTEXT, SBITEM_OPENSTATUS, (LPARAM)&szImeOpen);
    }
    else
    {
        SendMessageW(hStatusWnd, SB_SETTEXT, SBITEM_OPENSTATUS, (LPARAM)&szImeClose);
    }
}

static void SetConvModeParts(DWORD dwConvMode)
{
    switch (dwConvMode & IME_CMODE_LANGUAGE)
    {
        case IME_CMODE_ALPHANUMERIC:
            SendMessage(hStatusWnd, SB_SETTEXT, SBITEM_NATIVEMODE, (LPARAM)&szImeAlpha);
            break;

        case IME_CMODE_NATIVE:
            SendMessage(hStatusWnd, SB_SETTEXT, SBITEM_NATIVEMODE, (LPARAM)&szImeNative);
            break;

        case (IME_CMODE_NATIVE | IME_CMODE_KATAKANA):
            SendMessage(hStatusWnd, SB_SETTEXT, SBITEM_NATIVEMODE, (LPARAM)&szImeKatakana);
            break;

        default:
            SendMessage(hStatusWnd, SB_SETTEXT, SBITEM_NATIVEMODE, (LPARAM)&szImeNull);
            break;
    }

    auto setPartData = [dwConvMode](DWORD falg, int part, LPARAM onTrue, LPARAM onFalse = (LPARAM)&szImeNull) {
        if (dwConvMode & falg)
        {
            SendMessage(hStatusWnd, SB_SETTEXT, part, onTrue);
        }
        else
        {
            SendMessage(hStatusWnd, SB_SETTEXT, part, onFalse);
        }
    };

    setPartData(IME_CMODE_ROMAN, SBITEM_ROMAN, (LPARAM)&szImeRoman);
    setPartData(IME_CMODE_CHARCODE, SBITEM_CHARCODE, (LPARAM)&szImeRoman);
    setPartData(IME_CMODE_FULLSHAPE, SBITEM_FULLSHAPE, (LPARAM)&szImeFull, (LPARAM)&szImeHalf);
    setPartData(IME_CMODE_HANJACONVERT, SBITEM_HANJA, (LPARAM)&szImeHanja);
    setPartData(IME_CMODE_SOFTKBD, SBITEM_SOFTKBD, (LPARAM)&szImeSoftKbd);
    setPartData(IME_CMODE_EUDC, SBITEM_EUDC, (LPARAM)&szImeEUDC);
    setPartData(IME_CMODE_SYMBOL, SBITEM_SYMBOL, (LPARAM)&szImeSymbol);
}

static void ClearConvModeParts()
{

    for (int i = SBITEM_CMODEFIRST; i <= SBITEM_CMODELAST; i++)
    {
        SendMessage(hStatusWnd, SB_SETTEXT, i, (LPARAM)&szImeNull);
    }
}