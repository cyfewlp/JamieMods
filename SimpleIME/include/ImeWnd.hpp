#ifndef IMEWND_HPP
#define IMEWND_HPP

#pragma once

#include "ImeUI.h"
#include "tsf/TextStore.h"
#include "tsf/TsfCompartment.h"

#include <d3d11.h>
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class ImmImeHandler;
        static inline auto g_tMainClassName = L"SimpleIME";

        class ImeWnd
        {
            static constexpr WORD ID_EDIT_COPY  = 1;
            static constexpr WORD ID_EDIT_PASTE = 2;

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
            auto IsFocused() const -> bool;
            auto SendMessage_(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> LRESULT;
            auto EnableIme(bool enable) const -> void;
            auto EnableMod(bool enable) const -> bool;

            /**
             * Focus to parent window to abort ime
             */
            void AbortIme() const;
            void RenderIme() const;
            void ShowToolWindow() const;
            auto IsDiscardGameInputEvents(__in RE::InputEvent ** /*events*/) const -> bool;

        private:
            static auto WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static auto GetThis(HWND hWnd) -> ImeWnd *;
            static void ForwardKeyboardAndImeMessage(HWND hWndTarget, UINT uMsg, WPARAM wParam, LPARAM lParam);
            static void NewFrame();
            static auto IsWillTriggerIme(std::uint32_t code) -> bool;

            constexpr auto IsImeDisabledOrGameLoading() const -> bool;
            constexpr auto IsImeNotActive() const -> bool;
            constexpr auto IsImeWantCaptureInput() const -> bool;

            void OnStart();
            auto OnCreate() const -> LRESULT;
            auto OnDestroy() const -> LRESULT;
            void InitializeTextService(const AppConfig &pAppConfig);

            std::unique_ptr<ITextService> m_pTextService = nullptr;
            CComPtr<LangProfileUtil>      m_pLangProfileUtil{new LangProfileUtil()};
            bool                          m_fEnableTsf  = false;
            bool                          m_fFocused    = false;
            HWND                          m_hWnd        = nullptr;
            HWND                          m_hWndParent  = nullptr;
            HACCEL                        m_hAccelTable = nullptr;
            std::unique_ptr<ImeUI>        m_pImeUi      = nullptr;
            WNDCLASSEXW                   wc{};
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
