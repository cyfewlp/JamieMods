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

        // language id for english keyboard
        constexpr auto ASCII_GRAVE_ACCENT = 0x60; // `
        constexpr auto ASCII_MIDDLE_DOT   = 0xB7; // ·

        class GFxCharEvent : public RE::GFxEvent
        {
        public:
            GFxCharEvent() = default;

            explicit GFxCharEvent(UINT32 a_wcharCode, UINT8 a_keyboardIndex = 0)
                : GFxEvent(EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
            {
            }

            // @members
            std::uint32_t wcharCode{};     // 04
            std::uint32_t keyboardIndex{}; // 08
        };

        static_assert(sizeof(GFxCharEvent) == 0x0C);

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
            static auto                   WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static auto                   GetThis(HWND hWnd) -> ImeWnd *;
            auto                          OnCreate() const -> LRESULT;
            auto                          OnDestroy() const -> LRESULT;
            void                          InitializeTextService(const AppConfig &pAppConfig);

            std::unique_ptr<ITextService> m_pTextService = nullptr;
            CComPtr<LangProfileUtil>      m_pLangProfileUtil{new LangProfileUtil()};

            bool                          m_fEnableTsf = false;

            HWND                          m_hWnd       = nullptr;
            HWND                          m_hWndParent = nullptr;
            std::unique_ptr<ImeUI>        m_pImeUi     = nullptr;
            WNDCLASSEXW                   wc{};
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif
