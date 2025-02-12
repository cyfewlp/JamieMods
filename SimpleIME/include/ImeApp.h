#ifndef IMEAPP_H
#define IMEAPP_H

//
// Created by jamie on 25-1-22.
//
#pragma once

#include "Configs.h"
#include "ImeWnd.hpp"
#include "d3d11.h"
#include <RE/B/BSTEvent.h>
#include <RE/I/InputEvent.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        struct State
        {
            std::atomic<bool> Initialized = false;
        };

        class ImeApp
        {

        public:
            static void Init();

            static void D3DInit();
            static void D3DPresent(std::uint32_t ptr);
            static void DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events);
            static auto GetImeWnd() -> ImeWnd *;

        private:
            static auto           MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static void           ProcessEvent(RE::InputEvent **a_events);
            static void           ProcessMouseEvent(RE::ButtonEvent *btnEvent);

            static inline WNDPROC RealWndProc;
            static inline auto    g_pState  = std::make_unique<State>();
            static inline auto    g_pImeWnd = std::make_unique<ImeWnd>();
            static inline HWND    g_hWnd    = nullptr;
        };

    }
}

#endif
