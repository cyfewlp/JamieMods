#pragma once

#include "FunctionHook.h"
#include "SosGui.h"
#include "hook.h"

#include <atomic>
#include <memory>
#include <stdexcept>
#include <windows.h>

namespace SosGui
{
class App
{
    std::unique_ptr<Hooks::D3DInitHookData> D3DInitHook = nullptr;

    struct InitFail : std::runtime_error
    {
        explicit InitFail(const char *msg) : std::runtime_error(msg) {}
    };

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
    static inline WNDPROC RealWndProc;

    static auto MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

    static void D3DInit();

    void DoD3DInit();
};
} // namespace SosGui
