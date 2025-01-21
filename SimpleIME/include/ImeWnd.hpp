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
        HINSTANCE m_hInst;
        HWND      m_hWnd;
        HWND      m_hParentWnd;
        ImeUI    *m_pImeUI;
        bool      m_show;

    public:
        ImeWnd();
        ~ImeWnd();
        BOOL Initialize(HWND a_parent);
        void Focus();
        void RenderImGui();
        bool IsImeEnabled();

    private:
        static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void    MyPlatform_SetImeDataFn(ImGuiContext *ctx, ImGuiViewport *viewport, ImGuiPlatformImeData *data);
        LRESULT        OnCreate();
        LRESULT        OnDestroy();
        LRESULT        OnStartComposition();
        LRESULT        OnEndComposition();
        LRESULT        OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam);
    };
} // namespace SimpleIME
