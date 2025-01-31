//
// Created by jamie on 25-1-22.
//
#pragma once

#ifndef HELLOWORLD_IMEAPP_H
    #define HELLOWORLD_IMEAPP_H

    #include "Configs.h"
    #include "ImeWnd.hpp"
    #include "d3d11.h"
    #include <Device.h>

namespace SimpleIME
{

    struct State
    {
        std::atomic<bool> Initialized   = false;
    };

    class ImeApp
    {

    public:
        static void        Init();
        static FontConfig *LoadConfig();

        static void        D3DInit();
        static void        D3DPresent(std::uint32_t ptr);
        static void        DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *, RE::InputEvent **);
        static bool        CheckAppState();
        static bool        ResetExclusiveMode();

    private:
        static LRESULT        MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static void           ProcessEvent(RE::InputEvent **);
        static void           ProcessMouseEvent(RE::ButtonEvent *btnEvent);

        static inline WNDPROC RealWndProc;

    private:
        static inline auto g_pKeyboard   = std::make_unique<KeyboardDevice>(nullptr);
        static inline auto g_pState      = std::make_unique<State>();
        static inline auto g_pFontConfig = std::make_unique<FontConfig>();
        static inline auto g_pImeWnd     = std::make_unique<ImeWnd>();
        static inline HWND g_hWnd        = nullptr;

    private:
    };

}

#endif // HELLOWORLD_IMEAPP_H
