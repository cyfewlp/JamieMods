#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "configs/Configs.h"

#include "WcharBuf.h"
#include "configs/AppConfig.h"
#include "ime/ImmImeHandler.h"
#include "tsf/LangProfileUtil.h"
#include "tsf/TsfSupport.h"

#include "imgui.h"

#include <RE/G/GFxEvent.h>
#include <cstdint>
#include <vector>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {

        constexpr auto IME_UI_HEAP_INIT_SIZE   = 512;
        constexpr auto IME_UI_HEAP_MAX_SIZE    = 2048;
        constexpr auto WCHAR_BUF_INIT_SIZE     = 64;
        constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

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

        class ImeUI
        {
        public:
            explicit ImeUI(const AppUiConfig &uiConfig, InputMethod *pInputMethod);
            ~ImeUI();
            ImeUI(const ImeUI &other)                = delete;
            ImeUI(ImeUI &&other) noexcept            = delete;
            ImeUI &operator=(const ImeUI &other)     = delete;
            ImeUI &operator=(ImeUI &&other) noexcept = delete;

            bool   Initialize(LangProfileUtil *pLangProfileUtil);
            void   SetHWND(HWND hWnd);

            void   CompositionString(HIMC hIMC, LPARAM compFlag) const;
            void   UpdateLanguage();

            // Render To ImGui
            void RenderIme();
            void ShowToolWindow();

            template <typename... Args>
            void PushErrorMessage(std::format_string<Args...> fmt, Args &&...args);

        private:
            // return true if got str from IMM, otherwise false;
            static auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf) -> bool;

            void        SendResultStringToSkyrim() const;
            void        RenderToolWindow();
            void        RenderCompWindow() const;
            void        RenderCandidateWindows() const;

            static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");
            //
            HANDLE m_pHeap;
            /**
             * ImeWnd works hWnd.
             */
            HWND                     m_hWndIme = nullptr;
            WcharBuf                *m_pCompStr;
            WcharBuf                *m_pCompResult;
            UINT32                   m_keyboardCodePage{CP_ACP};
            AppUiConfig              m_pUiConfig;
            LangProfileUtil         *m_langProfileUtil = nullptr;

            InputMethod             *m_pInputMethod    = nullptr;
            TextEditor              *m_pTextEditor     = nullptr;

            bool                     m_showToolWindow  = false;
            std::vector<std::string> m_errorMessages;
            int  m_toolWindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;
            bool m_pinToolWindow   = false;
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
