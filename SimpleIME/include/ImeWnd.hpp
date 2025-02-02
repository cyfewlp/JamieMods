#pragma once

#include "Configs.h"
#include "ImeUI.h"
#include <d3d11.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

constexpr auto WM_CUSTOM = 0x7000;
#define WM_CUSTOM_CHAR             (WM_CUSTOM + 1)
#define WM_CUSTOM_IME_CHAR         (WM_CUSTOM_CHAR + 1)
#define WM_CUSTOM_IME_COMPPOSITION (WM_CUSTOM_IME_CHAR + 1)

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        const static inline wchar_t *g_tMainClassName = L"SimpleIME";

        class ImeWnd
        {
        public:
            HWND m_hWnd;
            ImeWnd();
            ~ImeWnd();
            void Initialize(HWND a_parent) noexcept(false);
            void InitImGui(ID3D11Device * /*device*/, ID3D11DeviceContext * /*context*/, FontConfig *fontConfig) const
                noexcept(false);
            void Focus() const;
            void RenderIme();
            void ShowToolWindow();
            void SetImeOpenStatus(bool open) const;
            bool IsImeEnabled() const;
            void SendMessage(UINT msg, WPARAM wParam, LPARAM lParam);
            bool IsDiscardGameInputEvents(__in RE::InputEvent ** /*events*/);

        private:
            static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
            LRESULT        OnCreate();
            static LRESULT OnDestroy();
            LRESULT        OnStartComposition();
            LRESULT        OnEndComposition();
            LRESULT        OnComposition(HWND hWnd, LPARAM lParam);

            HWND        m_hWndParent;
            ImeUI      *m_pImeUI;
            WNDCLASSEXW wc;
        };
    } // namespace SimpleIME
}
