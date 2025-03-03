#ifndef IMEAPP_H
#define IMEAPP_H

//
// Created by jamie on 25-1-22.
//
#pragma once

#include "ImeWnd.hpp"

#include <RE/B/BSTEvent.h>
#include <RE/I/InputEvent.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
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
            static void DoD3DInit();
            static void HookAddMessage(RE::UIMessageQueue *self, RE::BSFixedString &, RE::UI_MESSAGE_TYPE,
                                       RE::IUIMessageData *);
            static auto MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static void ProcessEvent(RE::InputEvent **a_events);
            static void ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent);
            static void ProcessMouseEvent(const RE::ButtonEvent *btnEvent);

            static inline WNDPROC                 RealWndProc;
            static inline std::unique_ptr<State>  g_pState  = nullptr;
            static inline std::unique_ptr<ImeWnd> g_pImeWnd = nullptr;
            static inline HWND                    g_hWnd    = nullptr;
        };

    }
}

#endif
