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
    class App
    {
        std::unique_ptr<Hooks::D3DInitHookData> D3DInitHook = nullptr;

        struct InitFail : std::runtime_error
        {
            explicit InitFail(const char *msg) : std::runtime_error(msg)
            {
            }
        };

        SosGui           m_SosGui;
        HWND             m_hWnd         = nullptr;
        std::atomic_bool m_fInitialized = false;

    public:
        auto Init() -> void;

        static auto GetInstance() -> App &
        {
            static App g_instance;
            return g_instance;
        }

    private:
        static void D3DInit();

        void DoD3DInit();
    };
}