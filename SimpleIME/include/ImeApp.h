#ifndef IMEAPP_H
#define IMEAPP_H

//
// Created by jamie on 25-1-22.
//
#pragma once

#include "ImeWnd.hpp"
#include "common/hook.h"

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
            static ImeApp instance;

        public:
            ImeApp()                              = default;
            ~ImeApp()                             = default;
            ImeApp(const ImeApp &other)           = delete;
            ImeApp(ImeApp &&other)                = delete;
            ImeApp operator=(const ImeApp &other) = delete;
            ImeApp operator=(ImeApp &&other)      = delete;

            static auto GetInstance() -> ImeApp &;

            void Initialize();
            void Uninitialize();

        private:
            std::unique_ptr<Hooks::D3DInitHookData>            D3DInitHook            = nullptr;
            std::unique_ptr<Hooks::D3DPresentHookData>         D3DPresentHook         = nullptr;
            std::unique_ptr<Hooks::DispatchInputEventHookData> DispatchInputEventHook = nullptr;

            void OnD3DInit();
            void Start(RE::BSGraphics::RendererData &renderData);
            void ProcessEvent(RE::InputEvent **a_events, bool &discard);
            void ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent, bool &discard);
            void ProcessMouseEvent(const RE::ButtonEvent *btnEvent);
            void InstallHooks();
            void UninstallHooks();

            ImeWnd m_imeWnd;
            HWND   m_hWnd  = nullptr;
            State  m_state = {};

            static void D3DInit();
            static void DoD3DInit();
            static void D3DPresent(std::uint32_t ptr);
            static void DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events);
            static auto MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static inline WNDPROC RealWndProc;
        };

    }
}

#endif
