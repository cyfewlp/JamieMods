#pragma once

#include <string>
#include <windows.h>

#define WM_CUSTOM                  0x7000
#define WM_CUSTOM_CHAR             WM_CUSTOM + 1
#define WM_CUSTOM_IME_CHAR         WM_CUSTOM_CHAR + 1
#define WM_CUSTOM_IME_COMPPOSITION WM_CUSTOM_IME_CHAR + 1
#define IMEUI_HEAP_INIT            512
#define IMEUI_HEAP_MAX             2048

namespace Transmogrify
{
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
            bool TryReAlloc(DWORD bufLen);
            void Clear();
            bool IsEmpty();
        };

    private:
        static inline float g_caretPos[] = {0.0f, 0.0f};

    private:
        HANDLE      m_pHeap;
        WcharBuf   *m_CompStr;
        WcharBuf   *m_CompResult;
        std::string m_langNameStr;
        bool        m_enableSkyrimIME;

    public:
        ImeUI();
        ~ImeUI();

        void StartComposition();
        void EndComposition();
        void CompositionString(HWND hwnd, LPARAM lParam);
        // Render To ImGui
        void RenderCompStr();
        void UpdateLanguage(HKL hkl);
        void UpdateCaretPos(float x, float y);
        bool IsSkyrimIMEEnabled();

    private:
        // return true if got str from IMM, otherwise false;
        bool GetCompStr(HIMC hIMC, LPARAM compFlag, LPARAM flagToCheck, WcharBuf *pWcharBuf);
        void SendResultString();
        void SendResultStringToSkyrim();
        void RenderCompWindow(WcharBuf *compStrBuf);
    };

} // namespace Transmogrify
