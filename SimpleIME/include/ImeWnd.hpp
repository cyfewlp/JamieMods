#ifndef IMEWND_HPP
#define IMEWND_HPP

#pragma once

#include "AppConfig.h"
#include "Configs.h"
#include "ImeUI.h"
#include "TsfSupport.h"
#include <d3d11.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

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

            void Initialize() noexcept(false);
            /**
             * Work on standalone thread and run own message loop.
             * Mainly for avoid other plugins that init COM
             * with COINIT_MULTITHREADED(crash logger) to effect our tsf code.
             *
             * @param hWndParent Main window(game window)
             */
            void Start(HWND hWndParent);
            /**
             * initialize ImGui. Work on UI thread.
             * @param pAppConfig main config file
             */
            void InitImGui(HWND hWnd, ID3D11Device * /*device*/, ID3D11DeviceContext * /*context*/) const
                noexcept(false);
            void Focus() const;
            void RenderIme();
            void ShowToolWindow();
            void SetImeOpenStatus(bool open) const;
            auto IsDiscardGameInputEvents(__in RE::InputEvent ** /*events*/) -> bool;

        private:
            static auto WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static auto GetThis(HWND hWnd) -> ImeWnd *;
            auto        OnCreate() const -> LRESULT;
            static auto OnDestroy() -> LRESULT;
            auto        OnStartComposition() const -> LRESULT;
            auto        OnEndComposition() const -> LRESULT;
            auto        OnComposition(HWND hWnd, LPARAM lParam) const -> LRESULT;

            TsfSupport  m_tsfSupport;
            HWND        m_hWnd;
            HWND        m_hWndParent;
            ImeUI      *m_pImeUI;
            WNDCLASSEXW wc{};
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
