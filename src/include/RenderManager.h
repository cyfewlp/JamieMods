#include "Hooks.h"
#include <d3d11.h>

#include <atomic>
#include <mutex>

class RenderManager
{
public:
    static void InstallHooks();
    static void D3DInit();
    static void D3DPresent(std::uint32_t a_p1);
    static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void ConfigImGui(HWND);
    static void render(bool* show_demo_window, bool* show_another_window);

    static void SwitchShowWindow();

    static void cleanup();

protected:
    RenderManager() = default;
    ~RenderManager() = default;

private:
    static inline std::atomic<bool> showWindow = false;
    static inline std::atomic<bool> initialized = false;
    static inline REL::Relocation<Hooks::D3DInitHook::D3DInitFunc> RealD3dInitFunc;
    static inline REL::Relocation<Hooks::D3DPresentHook::D3DPresentFunc> RealD3dPresentFunc;
    static inline WNDPROC RealWndProc;
};
