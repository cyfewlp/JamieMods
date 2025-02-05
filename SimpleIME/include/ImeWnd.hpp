#ifndef IMEWND_HPP
#define IMEWND_HPP

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
            ImeWnd();
            ~ImeWnd();

            ImeWnd(ImeWnd &&a_imeWnd)                          = delete;
            ImeWnd(const ImeWnd &a_imeWnd)                     = delete;

            auto operator=(ImeWnd &&a_imeWnd) -> ImeWnd &      = delete;
            auto operator=(const ImeWnd &a_imeWnd) -> ImeWnd & = delete;

            void Initialize(HWND a_parent) noexcept(false);
            void InitImGui(ID3D11Device * /*device*/, ID3D11DeviceContext * /*context*/, FontConfig *fontConfig) const
                noexcept(false);
            void               Focus() const;
            void               RenderIme();
            void               ShowToolWindow();
            void               SetImeOpenStatus(bool open) const;
            auto               IsDiscardGameInputEvents(__in RE::InputEvent               **/*events*/) -> bool;

        private:
            static auto WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            auto        OnCreate() -> LRESULT;
            static auto OnDestroy() -> LRESULT;
            auto        OnStartComposition() -> LRESULT;
            auto        OnEndComposition() -> LRESULT;
            auto        OnComposition(HWND hWnd, LPARAM lParam) -> LRESULT;

            HWND        m_hWnd;
            HWND        m_hWndParent;
            ImeUI      *m_pImeUI;
            WNDCLASSEXW wc{};
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
