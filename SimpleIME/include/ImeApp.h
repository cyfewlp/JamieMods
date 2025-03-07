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

        private:
            std::optional<Hooks::D3DInitHookData>            D3DInitHook            = std::nullopt;
            std::optional<Hooks::D3DPresentHookData>         D3DPresentHook         = std::nullopt;
            std::optional<Hooks::DispatchInputEventHookData> DispatchInputEventHook = std::nullopt;

            void Start(RE::BSGraphics::RendererData &renderData);
            void ProcessEvent(RE::InputEvent **a_events);
            void ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent);
            void ProcessMouseEvent(const RE::ButtonEvent *btnEvent);

            ImeWnd m_imeWnd{};
            HWND   m_hWnd = nullptr;
            State  m_state = {};

            static void D3DInit();
            static void DoD3DInit();
            static void InstallHooks();
            static void D3DPresent(std::uint32_t ptr);
            static void DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events);
            static void HookAddMessage(RE::UIMessageQueue *self, RE::BSFixedString &, RE::UI_MESSAGE_TYPE,
                                       RE::IUIMessageData *);
            static auto MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;
            static inline WNDPROC RealWndProc;
        };

    }
}

#endif
