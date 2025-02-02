//
// Created by jamie on 25-1-21.
//
#pragma once

#include "Configs.h"
#include "LangProfileUtil.h"
#include "windows.h"
#include <imgui.h>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{

#define MAKE_CONTEXT(hWnd, fn, ...)                                                                                    \
    {                                                                                                                  \
        HIMC hIMC;                                                                                                     \
        if ((hIMC = ImmGetContext(hWnd)))                                                                              \
        {                                                                                                              \
            fn(hIMC __VA_OPT__(, ) __VA_ARGS__);                                                                       \
        }                                                                                                              \
        ImmReleaseContext(hWnd, hIMC);                                                                                 \
    }

    namespace SimpleIME
    {

        constexpr auto IMEUI_HEAP_INIT_SIZE      = 512;
        constexpr auto IMEUI_HEAP_MAX_SIZE       = 2048;
        constexpr auto WCHAR_BUF_INIT_SIZE       = 64;

        constexpr auto CAND_MAX_WINDOW_COUNT     = 32;
        constexpr auto CAND_DEFAULT_NUM_PER_PAGE = 5;
        constexpr auto CAND_WINDOW_OFFSET_X      = 2;
        constexpr auto CAND_WINDOW_OFFSET_Y      = 5;
        constexpr auto CAND_WINDOW_PADDING       = 10.0F;
        constexpr auto CAND_PADDING              = 5.0F;

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

        struct ImeCandidate
        {
            float                    lineWidth;
            DWORD                    dwNumPerPage;
            DWORD                    dwSelecttion;
            std::vector<std::string> candList;

            ImeCandidate()
            {
                lineWidth    = 0.0F;
                dwSelecttion = dwNumPerPage = 0;
            }
        };

        enum ImeState
        {
            IME_IN_COMPOSITION  = 0x1,
            IME_IN_CANDCHOOSEN  = 0x2,
            IME_IN_ALPHANUMERIC = 0x4,
            IME_OPEN            = 0x8,
            IME_UI_FOCUSED      = 0x10,
            IME_ALL             = 0xFFFF
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
                bool               TryReAlloc(DWORD bufLen);
                void               Clear();
                [[nodiscard]] bool IsEmpty() const;
            };

        public:
            ImeUI();
            ~ImeUI();

            void StartComposition();
            void EndComposition();
            void CompositionString(HIMC hIMC, LPARAM compFlag);
            void QueryAllInstalledIME();
            // Render To ImGui
            void               RenderIme();
            void               ShowToolWindow();
            void               UpdateLanguage();
            void               UpdateActiveLangProfile();
            void               OnSetOpenStatus(HIMC /*hIMC*/);
            void               UpdateConversionMode(HIMC /*hIMC*/);
            bool               ImeNotify(HWND /*hwnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
            [[nodiscard]] bool IsEnabled() const;
            [[nodiscard]] SKSE::stl::enumeration<ImeState> GetImeState() const;

        private:
            // return true if got str from IMM, otherwise false;
            static bool GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf);
            void        SendResultString();
            void        SendResultStringToSkyrim();
            void        RenderToolWindow();
            static void RenderCompWindow(WcharBuf *compStrBuf);
            void        OpenCandidate(HIMC /*hIMC*/, LPARAM /*candListFlag*/);
            void        ChangeCandidate(HIMC /*hIMC*/, LPARAM /*candListFlag*/);
            void        CloseCandidate(LPARAM /*candListFlag*/);
            void        RenderCandWindow(ImVec2 &wndPos) const;
            void        UpdateImeCandidate(ImeCandidate        */*candidate*/, LPCANDIDATELIST /*lpCandList*/) const;

            HANDLE      m_pHeap;
            WcharBuf   *m_compStr;
            WcharBuf   *m_compResult;
            UINT32      keyboardCodePage{CP_ACP};
            SKSE::stl::enumeration<ImeState> m_imeState;
            ImeCandidate                    *m_imeCandidates[CAND_MAX_WINDOW_COUNT]{nullptr};
            std::vector<LangProfile>         m_imeProfiles;
            LangProfileUtil                  langProfileUtil;
            bool                             m_showToolWindow = false;
            float                            m_fontSize;
        };
    }
}