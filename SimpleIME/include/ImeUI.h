#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "LangProfileUtil.h"
#include "TsfSupport.h"
#include "configs/AppConfig.h"
#include "configs/Configs.h"
#include "enumeration.h"
#include "WcharBuf.h"
#include "imgui.h"
#include <RE/G/GFxEvent.h>
#include <array>
#include <cstdint>
#include <vector>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    class ImmContextGuard
    {
    public:
        explicit ImmContextGuard(HWND hWnd) : hWnd_(hWnd), hIMC_(ImmGetContext(hWnd_))
        {
            if (hIMC_ == nullptr)
            {
                log_error("Failed to get IME context!");
            }
        }

        ~ImmContextGuard()
        {
            if (hIMC_ != nullptr)
            {
                ImmReleaseContext(hWnd_, hIMC_);
            }
        }

        [[nodiscard]] auto get() const -> HIMC
        {
            return hIMC_;
        }

    private:
        HWND hWnd_;
        HIMC hIMC_{nullptr};
    };

    namespace SimpleIME
    {

        constexpr auto IME_UI_HEAP_INIT_SIZE   = 512;
        constexpr auto IME_UI_HEAP_MAX_SIZE    = 2048;
        constexpr auto WCHAR_BUF_INIT_SIZE     = 64;
        constexpr auto MAX_ERROR_MESSAGE_COUNT = 10;

        struct CandWindowProp
        {
            static constexpr std::uint8_t MAX_COUNT         = 32;
            static constexpr std::uint8_t DEFAULT_PAGE_SIZE = 5;
            static constexpr float        PADDING           = 10.0F;
        };

        // language id for english keyboard
        constexpr auto ASCII_GRAVE_ACCENT = 0x60; // `
        constexpr auto ASCII_MIDDLE_DOT   = 0xB7; // ·

        class GFxCharEvent : public RE::GFxEvent
        {
        public:
            GFxCharEvent() = default;

            explicit GFxCharEvent(UINT32 a_wcharCode, UINT8 a_keyboardIndex = 0)
                : GFxEvent(RE::GFxEvent::EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
            {
            }

            // @members
            std::uint32_t wcharCode{};     // 04
            std::uint32_t keyboardIndex{}; // 08
        };

        static_assert(sizeof(GFxCharEvent) == 0x0C);

        struct ImeCandidateList
        {
        public:
            ImeCandidateList() = default;

            void SetPageSize(const DWORD dwPageSize)
            {
                m_dwPageSize = (dwPageSize == 0U) ? CandWindowProp::DEFAULT_PAGE_SIZE : dwPageSize;
            }

            [[nodiscard]] constexpr auto Selection() const -> DWORD
            {
                return m_dwSelection;
            }

            [[nodiscard]] constexpr auto CandidateList() const -> std::vector<std::string>
            {
                return m_candidateList;
            }

            // use provided lpCandList flush candidate cache
            // @fontSize be used calculate singlie line width
            void Flush(LPCANDIDATELIST lpCandList);

        private:
            DWORD                    m_dwPageSize{CandWindowProp::DEFAULT_PAGE_SIZE};
            DWORD                    m_dwSelection{0};
            std::vector<std::string> m_candidateList;
        } __attribute__((packed)) __attribute__((aligned(64)));

        enum class ImeState : std::uint16_t
        {
            IN_COMPOSITION  = 0x1,
            IN_CANDCHOOSEN  = 0x2,
            IN_ALPHANUMERIC = 0x4,
            OPEN            = 0x8,
            IME_ALL         = 0xFFFF
        };

        class ImeUI
        {
        public:
            explicit ImeUI(const AppUiConfig &uiConfig);
            ~ImeUI();
            ImeUI(const ImeUI &other)                = delete;
            ImeUI(ImeUI &&other) noexcept            = delete;
            ImeUI &operator=(const ImeUI &other)     = delete;
            ImeUI &operator=(ImeUI &&other) noexcept = delete;

            bool   Initialize(TsfSupport &tsfSupport);
            void   SetHWND(const HWND hWnd);

            void   StartComposition();
            void   EndComposition();
            void   CompositionString(HIMC hIMC, LPARAM compFlag) const;
            void   UpdateLanguage();
            void   OnSetOpenStatus(HIMC hIMC);
            void   UpdateConversionMode(HIMC hIMC);
            auto   ImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam) -> bool;
            void   ActivateProfile(const GUID *guidProfile);

            auto   IsEnabled() const -> bool;
            // Render To ImGui
            void RenderIme();
            void ShowToolWindow();

            template <typename... Args>
            void PushErrorMessage(std::format_string<Args...> fmt, Args &&...args);

            //
            [[nodiscard]] auto GetImeState() const -> Enumeration<ImeState>;

        private:
            // return true if got str from IMM, otherwise false;
            static auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf) -> bool;

            void        SendResultStringToSkyrim() const;
            void        RenderToolWindow();
            void        RenderCompWindow(const WcharBuf *compStrBuf) const;
            void        OpenCandidate(HIMC hIMC, LPARAM candListFlag);
            void        ChangeCandidate(HIMC hIMC, LPARAM candListFlag);
            void        ChangeCandidateAt(HIMC hIMC, DWORD dwIndex);
            void        CloseCandidate(LPARAM candListFlag);
            void        RenderCandidateWindows() const;

            static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");
            std::array<std::unique_ptr<ImeCandidateList>, CandWindowProp::MAX_COUNT> m_imeCandidates{};
            //
            HANDLE m_pHeap;
            /**
             * ImeWnd works hWnd.
             */
            HWND                     m_hWndIme;
            WcharBuf                *m_pCompStr;
            WcharBuf                *m_pCompResult;
            UINT32                   m_keyboardCodePage{CP_ACP};
            AppUiConfig              m_pUiConfig;

            Enumeration<ImeState>    m_imeState;
            LangProfileUtil          m_langProfileUtil;
            bool                     m_showToolWindow = false;
            std::vector<std::string> m_errorMessages;
            int  m_toolWindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;
            bool m_pinToolWindow   = false;
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
