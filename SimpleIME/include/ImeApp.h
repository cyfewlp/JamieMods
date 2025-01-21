//
// Created by jamie on 25-1-22.
//
#pragma once

#ifndef HELLOWORLD_IMEAPP_H
    #define HELLOWORLD_IMEAPP_H

    #include "d3d11.h"
    #include "windows.h"
    #include "Configs.h"
    #include "ImeWnd.hpp"

namespace SimpleIME
{
    struct State
    {
        std::atomic<bool> Initialized = false;
    };

    class ImeApp
    {
    private:
        static inline State *gState;
        static inline FontConfig *gFontConfig;
        static inline ImeWnd gImeWnd;

    public:
        static void Init();
        static FontConfig *LoadConfig();

    private:
        static void InstallHooks();
        static void D3DInit();
        static void D3DPresent();
        static void Render();
        static void InitImGui(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);
    };

}

#endif // HELLOWORLD_IMEAPP_H
