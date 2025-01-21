#pragma once

#ifndef _TRANSMOGRIFT_WINDOW_H
    #define _TRANSMOGRIFT_WINDOW_H
    #define MAX_LISTCAND 32
    #include "ImeUI.hpp"
    #include "imgui.h"
    #include <windows.h>

namespace Transmogrify
{
    const static inline wchar_t *g_tMainClassName = L"SKSE Transmogrify";
    class HideWnd
    {
        friend class WindowManger;

    private:
        HINSTANCE m_hInst;
        HWND      m_hWnd;
        HWND      m_hParentWnd;
        ImeUI    *m_pImeUI;

    public:
        HideWnd();
        ~HideWnd();
        BOOL Initialize(HWND a_parent);
        void Focus();
        void RenderImGui();
        bool IsSkyrimIMEEnabled();

    private:
        static LRESULT _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static void    MyPlatform_SetImeDataFn(ImGuiContext *ctx, ImGuiViewport *viewport, ImGuiPlatformImeData *data);
        LRESULT        OnCreate();
        LRESULT        OnDestroy();
        LRESULT        OnStartComposition();
        LRESULT        OnEndComposition();
        LRESULT        OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam);
    };
}
#endif
