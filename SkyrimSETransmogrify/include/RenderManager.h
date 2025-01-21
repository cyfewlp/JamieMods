#pragma once

#include <d3d11.h>

#include "Config.h"
#include "Hooks.h"
#include "Transmogrify.h"
#include "Window.hpp"
#include "imgui.h"
#include <atomic>
#include <mutex>


typedef std::vector<std::wstring> CandStrings;
extern IMGUI_IMPL_API LRESULT     ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                                 LPARAM lParam); // Use ImGui::GetCurrentContext()
static Transmogrify::HideWnd     *hideChildWnd;

class RenderManager
{
private:
    static inline std::atomic<bool>                                      showWindow  = false;
    static inline std::atomic<bool>                                      initialized = false;
    static inline REL::Relocation<Hooks::D3DInitHook::D3DInitFunc>       RealD3dInitFunc;
    static inline REL::Relocation<Hooks::D3DPresentHook::D3DPresentFunc> RealD3dPresentFunc;
    static inline WNDPROC                                                RealWndProc;
    static inline Config                                                *config;

public:
    static void Init(Config *config);
    static void D3DInit();
    static void D3DPresent(std::uint32_t a_p1);
    static void ConfigImGui(HWND);
    static void render();
    static void RenderArmors();

    static void SwitchShowWindow();
    static void cleanup();
    static bool IsShowWindow();
    static void UpdateCandStrings(HWND hWnd);

    struct UIState
    {
        bool                     showArmors  = false;
        int                      selectedMod = 0;
        std::vector<std::string> mods;
        MapToSet                 modToArmors;
    };

protected:
    RenderManager()  = default;
    ~RenderManager() = default;

private:
    static inline UIState uiState;
    static LRESULT        WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
