//
// Created by jamie on 25-1-21.
//
#pragma once
#ifndef HELLOWORLD_IMEUI_H
    #define HELLOWORLD_IMEUI_H

    #define IMEUI_HEAP_INIT_SIZE 512
    #define IMEUI_HEAP_MAX_SIZE  2048
    #include "windows.h"

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

    private:
        static inline float g_caretPos[] = {0.0f, 0.0f};

    private:
        HANDLE      m_pHeap;
        WcharBuf   *m_CompStr;
        WcharBuf   *m_CompResult;
        std::string m_langNameStr;
        bool        m_enableIME;

    public:
        ImeUI();
        ~ImeUI();

        void StartComposition();
        void EndComposition();
        void CompositionString(HWND hwnd, LPARAM lParam);
        // Render To ImGui
        void               RenderImGui();
        void               UpdateLanguage(HKL hkl);
        static void        UpdateCaretPos(float x, float y);
        [[nodiscard]] bool IsEnabled() const;

    private:
        // return true if got str from IMM, otherwise false;
        static bool GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf);
        static void RenderCompWindow(WcharBuf *compStrBuf);
        void        SendResultString();
        void        SendResultStringToSkyrim();
    };
}

#endif // HELLOWORLD_IMEUI_H
