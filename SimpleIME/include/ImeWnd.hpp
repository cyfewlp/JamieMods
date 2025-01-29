#pragma once

#include "Configs.h"
#include "ImeUI.h"
#include "imgui.h"
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

constexpr auto WM_CUSTOM = 0x7000;
#define WM_CUSTOM_CHAR             (WM_CUSTOM + 1)
#define WM_CUSTOM_IME_CHAR         (WM_CUSTOM_CHAR + 1)
#define WM_CUSTOM_IME_COMPPOSITION (WM_CUSTOM_IME_CHAR + 1)

namespace SimpleIME
{
    const static inline wchar_t *g_tMainClassName = L"SimpleIME";

    class ImeWnd
    {
    public:
        ImeWnd();
        ~ImeWnd();
        void             Initialize(HWND a_parent, FontConfig *fontConfig) noexcept(false);
        void             Focus() const;
        void             RenderImGui();
        bool             IsImeEnabled() const;
        void             SendMessage(UINT msg, WPARAM wParam, LPARAM lParam);
        RE::InputEvent **FilterInputEvent(RE::InputEvent **);

    private:
        static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT        OnCreate();
        LRESULT        OnDestroy();
        LRESULT        OnStartComposition();
        LRESULT        OnEndComposition();
        LRESULT        OnComposition(HWND hWnd, LPARAM lParam);
        void           InitImGui(FontConfig *fontConfig) noexcept(false);
        bool           CreateDeviceD3D() noexcept;
        void           CreateRenderTarget();
        void           CleanupDeviceD3D();
        void           CleanupRenderTarget();

    private:
        HWND                    m_hWnd;
        HINSTANCE               m_hInst;
        HWND                    m_hWndParent;
        ImeUI                  *m_pImeUI;
        WNDCLASSEXW             wc;
        ID3D11RenderTargetView *m_mainRenderTargetView = nullptr;
        IDXGISwapChain         *m_pSwapChain           = nullptr;
        ID3D11Device           *m_pd3dDevice           = nullptr;
        ID3D11DeviceContext    *m_pd3dDeviceContext    = nullptr;
        UINT                    m_ResizeWidth = 0, m_ResizeHeight = 0;
    };
} // namespace SimpleIME
