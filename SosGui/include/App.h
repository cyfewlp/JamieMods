#pragma once

#include "SosGui.h"
#include "common/FunctionHook.h"
#include "common/hook.h"

#include <atomic>
#include <memory>
#include <stdexcept>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    struct MenuPostDisplayHook : Hooks::FunctionHook<void(RE::IMenu *)>
    {
        explicit MenuPostDisplayHook(func_type *funcPtr) : FunctionHook(RELOCATION_ID(0, 33632), funcPtr) // TODO
        {
            log_debug("{} hooked at {:#x}", __func__, m_address);
        }
    };

    class App
    {
        std::unique_ptr<Hooks::D3DInitHookData>    D3DInitHook           = nullptr;
        std::unique_ptr<Hooks::D3DPresentHookData> D3DPresentHook        = nullptr;
        std::unique_ptr<MenuPostDisplayHook>       g_MenuPostDisplayHook = nullptr;

        struct InitFail : std::runtime_error
        {
            explicit InitFail(const char *msg) : std::runtime_error(msg)
            {
            }
        };

        SosGui           m_SosGui;
        HWND             m_hWnd         = nullptr;
        WNDPROC          RealWndProc    = nullptr;
        std::atomic_bool m_fInitialized = false;

    public:
        auto Init() -> void;
        auto UnInit();
        auto InstallSinks() -> void;

        static auto GetInstance() -> App &
        {
            static App g_instance;
            return g_instance;
        }

    private:
        static void D3DInit();
        static void D3DPresent(std::uint32_t ptr);
        static void MenuPostDisplay(RE::IMenu *self);
        static auto MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT;

        void DoD3DInit();
        void InstallHooks();
    };
}