#ifndef IMEWND_HPP
#define IMEWND_HPP

#pragma once

#include "configs/Configs.h"

#include "ImeUI.h"
#include "ime/InputMethod.h"
#include "tsf/TextStore.h"
#include "tsf/TsfCompartment.h"
#include "tsf/TsfSupport.h"

#include <d3d11.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImmImeHandler;
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
            void UnInitialize() const noexcept;

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
             */
            void InitImGui(HWND hWnd, ID3D11Device * /*device*/, ID3D11DeviceContext * /*context*/) const
                noexcept(false);
            void Focus() const;
            void RenderIme() const;
            void ShowToolWindow() const;
            auto IsDiscardGameInputEvents(__in RE::InputEvent ** /*events*/) const -> bool;

        private:
            static auto                     WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static auto                     GetThis(HWND hWnd) -> ImeWnd *;
            auto                            OnCreate() const -> LRESULT;
            auto                            OnDestroy() const -> LRESULT;
            auto                            OnStartComposition() const -> LRESULT;
            auto                            OnEndComposition() const -> LRESULT;
            auto                            OnComposition(HWND hWnd, LPARAM lParam) const -> LRESULT;

            Tsf::TsfSupport                 m_tsfSupport;
            std::unique_ptr<InputMethod>    m_pInputMethod = nullptr;

            std::unique_ptr<Tsf::TextStore> m_pTextStore   = nullptr;
            CComPtr<Tsf::TsfCompartment>    m_pTsfCompartment{new Tsf::TsfCompartment()};
            CComPtr<LangProfileUtil>        m_pLangProfileUtil{new LangProfileUtil()};

            std::unique_ptr<ImmImeHandler>  m_immImeHandler = nullptr;
            bool                            m_fEnableTsf    = false;

            HWND                            m_hWnd          = nullptr;
            HWND                            m_hWndParent    = nullptr;
            std::unique_ptr<ImeUI>          m_pImeUi        = nullptr;
            WNDCLASSEXW                     wc{};
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
