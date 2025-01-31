//
// Created by jamie on 25-1-21.
//
#pragma once

#include "LangProfileUtil.h"
#include "imgui.h"
#include "windows.h"
#include <vector>

#define MAKE_CONTEXT(hWnd, fn, ...)                                                                                    \
    {                                                                                                                  \
        HIMC hIMC;                                                                                                     \
        if ((hIMC = ImmGetContext(hWnd)))                                                                              \
        {                                                                                                              \
            fn(hIMC __VA_OPT__(, ) __VA_ARGS__);                                                                       \
        }                                                                                                              \
        ImmReleaseContext(hWnd, hIMC);                                                                                 \
    }

constexpr auto IMEUI_HEAP_INIT_SIZE      = 512;
constexpr auto IMEUI_HEAP_MAX_SIZE       = 2048;
constexpr auto MAX_CAND_LIST             = 32;
constexpr auto DEFAULT_CAND_NUM_PER_PAGE = 5;

namespace SimpleIME
{
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
        DWORD                    dwWidth;
        DWORD                    dwNumPerPage;
        DWORD                    dwSelecttion;
        std::vector<std::string> candList;

        ImeCandidate()
        {
            dwWidth = dwSelecttion = dwNumPerPage = 0;
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

        public:
            WcharBuf(HANDLE heap, DWORD initSize);
            ~WcharBuf();
            bool               TryReAlloc(DWORD bufLen);
            void               Clear();
            [[nodiscard]] bool IsEmpty() const;
        };

    public:
        ImeUI(HWND hWnd);
        ~ImeUI();

        void StartComposition();
        void EndComposition();
        void CompositionString(HIMC hIMC, LPARAM lParam);
        void QueryAllInstalledIME();
        // Render To ImGui
        void                                           RenderIme();
        void                                           ShowToolWindow();
        void                                           UpdateLanguage();
        void                                           UpdateActiveLangProfile();
        void                                           OnSetOpenStatus(HIMC);
        void                                           UpdateConversionMode(HIMC);
        bool                                           ImeNotify(HWND, WPARAM, LPARAM);
        [[nodiscard]] bool                             IsEnabled() const;
        [[nodiscard]] SKSE::stl::enumeration<ImeState> GetImeState() const;

    private:
        // return true if got str from IMM, otherwise false;
        static bool GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf);
        void        SendResultString();
        void        SendResultStringToSkyrim();
        void        RenderToolWindow();
        void        RenderCompWindow(WcharBuf *compStrBuf);
        void        OpenCandidate(HIMC, LPARAM);
        void        ChangeCandidate(HIMC, LPARAM);
        void        CloseCandidate(LPARAM);
        void        RenderCandWindow(ImVec2 &wndPos) const;
        void        UpdateImeCandidate(ImeCandidate *, LPCANDIDATELIST);

    public:
        static inline uint32_t fontSize = 14;

    private:
        HANDLE                           m_pHeap;
        WcharBuf                        *m_CompStr;
        WcharBuf                        *m_CompResult;
        std::string                      m_langNameStr;
        UINT32                           keyboardCodePage{CP_ACP};
        SKSE::stl::enumeration<ImeState> m_imeState;
        bool                             m_enableCandWindow;
        ImeCandidate                    *m_imeCandidates[MAX_CAND_LIST]{nullptr};
        std::vector<LangProfile>         m_imeProfiles;
        LangProfileUtil                  langProfileUtil{};
        bool                             m_showToolWindow = false;
        HWND                             m_hWnd;
    };
}
