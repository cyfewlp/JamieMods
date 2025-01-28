//
// Created by jamie on 25-1-22.
//
#pragma once

#ifndef HELLOWORLD_IMEAPP_H
    #define HELLOWORLD_IMEAPP_H

    #include "Configs.h"
    #include "ImeWnd.hpp"
    #include "d3d11.h"

namespace SimpleIME
{
    struct State
    {
        std::atomic<bool> Initialized = false;
    };

    class ImeApp
    {
    private:
        static inline State      *gState;
        static inline FontConfig *gFontConfig;
        static inline ImeWnd     *gImeWnd;

    public:
        static void        Init();
        static FontConfig *LoadConfig();

        static void        D3DInit();
        static void        D3DPresent(std::uint32_t ptr);
        static void        DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *, RE::InputEvent **);

    private:
        static LRESULT        MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static void           Render();
        static void           InitImGui(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);
        static void           ProcessEvent(RE::InputEvent **);
        static void           ProcessMouseEvent(RE::ButtonEvent *btnEvent);

        static inline WNDPROC RealWndProc;

    private:
    };

}

#endif // HELLOWORLD_IMEAPP_H
