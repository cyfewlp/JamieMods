#ifndef IMEUI_H
#define IMEUI_H

#pragma once

#include "Configs.h"
#include "LangProfileUtil.h"
#include "enumeration.h"
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

        constexpr auto IMEUI_HEAP_INIT_SIZE = 512;
        constexpr auto IMEUI_HEAP_MAX_SIZE  = 2048;
        constexpr auto WCHAR_BUF_INIT_SIZE  = 64;

        struct CandWindowProp
        {
            static constexpr std::uint8_t MAX_COUNT         = 32;
            static constexpr std::uint8_t DEFAULT_PAGE_SIZE = 5;
            static constexpr float        PADDING           = 10.0F;
        };

        // language id for english keyboard
        constexpr auto LANGID_ENG         = 0x409;
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

            void setPageSize(DWORD a_dwPageSize)
            {
                dwPageSize = (a_dwPageSize == 0U) ? CandWindowProp::DEFAULT_PAGE_SIZE : a_dwPageSize;
            }

            [[nodiscard]] constexpr auto getDwSelecttion() const -> DWORD
            {
                return dwSelecttion;
            }

            [[nodiscard]] constexpr auto getCandList() const -> std::vector<std::string>
            {
                return candList;
            }

            // use provided lpCandList flush candidate cache
            // @fontSize be used calculate singlie line width
            void Flush(LPCANDIDATELIST lpCandList);

        private:
            DWORD                    dwPageSize{CandWindowProp::DEFAULT_PAGE_SIZE};
            DWORD                    dwSelecttion{0};
            std::vector<std::string> candList;
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
        private:
            class WcharBuf
            {
            public:
                LPWSTR szStr;
                DWORD  dwCapacity;
                DWORD  dwSize;
                HANDLE m_heap;

                WcharBuf(HANDLE heap, DWORD initSize);
                ~WcharBuf();
                auto               TryReAlloc(DWORD bufLen) -> bool;
                void               Clear();
                [[nodiscard]] auto IsEmpty() const -> bool;
            };

        public:
            ImeUI(AppConfig *appConfig);
            ~ImeUI();

            ImeUI(ImeUI &&a_ImeUI)                          = delete;
            ImeUI(const ImeUI &a_ImeUI)                     = delete;
            auto operator=(ImeUI &&a_ImeUI) -> ImeUI &      = delete;
            auto operator=(const ImeUI &a_ImeUI) -> ImeUI & = delete;

            void StartComposition();
            void EndComposition();
            void CompositionString(HIMC hIMC, LPARAM compFlag);
            void QueryAllInstalledIME();
            // Render To ImGui
            void               RenderIme();
            void               ShowToolWindow();
            void               UpdateLanguage();
            void               UpdateActiveLangProfile();
            void               OnSetOpenStatus(HIMC hIMC);
            void               UpdateConversionMode(HIMC hIMC);
            auto               ImeNotify(HWND hwnd, WPARAM wParam, LPARAM lParam) -> bool;
            [[nodiscard]] auto IsEnabled() const -> bool;
            [[nodiscard]] auto GetImeState() const -> Enumeration<ImeState>;

        private:
            // return true if got str from IMM, otherwise false;
            static auto GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf) -> bool;
            void        SendResultStringToSkyrim();
            void        RenderToolWindow();
            void        RenderCompWindow(WcharBuf *compStrBuf);
            void        OpenCandidate(HIMC hIMC, LPARAM candListFlag);
            void        ChangeCandidate(HIMC hIMC, LPARAM candListFlag);
            void        ChangeCandidateAt(HIMC hIMC, DWORD dwIndex);
            void        CloseCandidate(LPARAM candListFlag);
            void        RenderCandWindows() const;

            static constexpr auto TOOL_WINDOW_NAME = std::span("ToolWindow##SimpleIME");
            std::array<std::unique_ptr<ImeCandidateList>, CandWindowProp::MAX_COUNT> m_imeCandidates{};
            //
            HANDLE     m_pHeap;
            WcharBuf  *m_pCompStr;
            WcharBuf  *m_pCompResult;
            UINT32     keyboardCodePage{CP_ACP};
            AppConfig *m_pAppConfig;

            //
            Enumeration<ImeState>    m_imeState;
            std::vector<LangProfile> m_imeProfiles;
            LangProfileUtil          m_langProfileUtil;
            bool                     m_showToolWindow = false;
            int  toolWindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;
            bool m_pinToolWindow = false;
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif
