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
        std::atomic<bool> keyboardState = false;
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
        static bool        CreateKeyboard(bool recreate = false);

    private:
        static LRESULT        MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static void           ProcessEvent(RE::InputEvent **);
        static void           ProcessMouseEvent(RE::ButtonEvent *btnEvent);

        static inline WNDPROC RealWndProc;

    private:
        static inline KeyboardDevice *g_pKeyboard = nullptr;
        static inline State          *gState;
        static inline FontConfig     *gFontConfig;
        static inline HWND            g_hWnd;
        static inline ImeWnd         *gImeWnd;

    private:
    };

}

#endif // HELLOWORLD_IMEAPP_H
