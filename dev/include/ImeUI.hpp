#pragma once

#include <windows.h>
#define MAX_LISTCAND       32
#define MAXROW             12
#define MAXCOL             60
#define IME_IN_COMPOSITION 1
#define IME_IN_CHOSECAND   2

extern BYTE textbuf[MAXROW][MAXCOL];
extern int  xPos, yPos;

typedef struct _IMEUIDATA
{
    int     ImeState; // Current Ime state.
    UINT    uCompLen; // To save previous composition string length.
    DWORD   fdwProperty;
    HWND    hListCand[MAX_LISTCAND];
    HGLOBAL hListCandMem[MAX_LISTCAND];
} IMEUIDATA;

namespace ImeUI
{
    void StartComposition(HWND hWnd);
    void EndComposition(HWND hWnd);
    void GetResultStr(HWND hWnd);
    void GetCompositionStr(HWND hwnd, LPARAM lParam, OUT std::wstring &compStr);
    //*********************************************************************
    // void DisplayCompString()
    // This displays composition string.
    // This function supports only fixed pitch font.
    //*********************************************************************
    void DisplayCompString(HWND hwnd, LPSTR lpStr, LPSTR lpStrAttr);
    void DisplayResultString(HWND hwnd, LPSTR lpStr);
}