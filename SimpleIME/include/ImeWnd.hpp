#pragma once

#include "ImeUI.h"
#include "imgui.h"
#include <windows.h>

constexpr auto WM_CUSTOM = 0x7000;
#define WM_CUSTOM_CHAR             (WM_CUSTOM + 1)
#define WM_CUSTOM_IME_CHAR         (WM_CUSTOM_CHAR + 1)
#define WM_CUSTOM_IME_COMPPOSITION (WM_CUSTOM_IME_CHAR + 1)

namespace SimpleIME
{
    const static inline wchar_t *g_tMainClassName = L"SimpleIME";

    class ImeWnd
    {
    private:
        HWND m_hWnd;
        HINSTANCE m_hInst;
        HWND      m_hParentWnd;
        ImeUI    *m_pImeUI;

    public:
        ImeWnd();
        ~ImeWnd();
        BOOL             Initialize(HWND a_parent) noexcept(false);
        void             Focus() const;
        void             RenderImGui();
        bool             IsShow() const;
        RE::InputEvent **FilterInputEvent(RE::InputEvent **);

    private:
        static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT        OnCreate();
        LRESULT        OnDestroy();
        LRESULT        OnStartComposition();
        LRESULT        OnEndComposition();
        LRESULT        OnComposition(HWND hWnd, LPARAM lParam);
    };
} // namespace SimpleIME
