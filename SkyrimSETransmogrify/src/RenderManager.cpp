// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "RenderManager.h"
#include "Hooks.h"
#include "ImeUI.hpp"
#include "Window.hpp"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "utils.hpp"
#include <unordered_set>

// void            RenderManager::UpdateCandStrings(HWND hWnd)
// {
//     HIMC hIMC = ImmGetContext(hWnd);
//     if (!hIMC) return;
//     DWORD dwBufLen = ImmGetCandidateList(hIMC, 0, nullptr, 0); // Get size first
//     if (dwBufLen == 0)
//     {
//         ImmReleaseContext(hWnd, hIMC);
//         return;
//     }
//     std::vector<BYTE> buffer(dwBufLen);
//     LPCANDIDATELIST   lpCandList = reinterpret_cast<LPCANDIDATELIST>(buffer.data());
//     if (!lpCandList)
//     {
//         LOG(err, "Alloc for LPCANDIDATELIST failed!");
//         return;
//     }
//     ImmGetCandidateList(hIMC, 0, lpCandList, dwBufLen);
//     if (lpCandList->dwStyle == IME_CAND_CODE) return;
//     uiState.candStrList.clear();
//     uiState.selectedCand = lpCandList->dwSelection;
//     utils::GetCandStrings(hWnd, lpCandList, &uiState.candStrList);
//     ImmReleaseContext(hWnd, hIMC);
// }

LRESULT
RenderManager::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SETFOCUS: {
        if (hideChildWnd)
        {
            LOG(debug, "focus to hide child window.");
            hideChildWnd->Focus();
        }
        break;
    }
    }
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;
    return RealWndProc(hWnd, uMsg, wParam, lParam);
}

void RenderManager::Init(Config *a_config)
{
    RealD3dInitFunc    = Hooks::D3DInitHook::Install(&D3DInit);
    RealD3dPresentFunc = Hooks::D3DPresentHook::Install(&D3DPresent);
    config             = a_config;
    SKSE::GetTaskInterface()->AddTask([] {
        Transmogrify::GetAllArmors(&uiState.modToArmors);
        for (auto &pair : uiState.modToArmors)
        {
            uiState.mods.push_back(pair.first);
        }
    });
}

LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code < 0)
    {
        return CallNextHookEx(0, code, wParam, lParam);
    }
    MSG *msg = (MSG *)(lParam);
    switch (msg->message)
    {
    case WM_CREATE:
        LOG(debug, "SkyrimSE Window creating!");
        break;
    case WM_IME_COMPOSITION:
    case WM_IME_CHAR:
    case WM_CHAR:
        LOG(trace, "MSG Hook: {:#x}, {:#x}, {:#x}", msg->message, msg->wParam, msg->lParam);
        if (msg->wParam >= 0xd800 && msg->wParam <= 0xdfff)
        {
            UINT original = msg->message;
            // msg->lParam = msg->message; // store original message
            switch (msg->message)
            {
            case WM_IME_COMPOSITION:
                msg->message = WM_CUSTOM_IME_COMPPOSITION;
                break;
            case WM_IME_CHAR:
                msg->message = WM_CUSTOM_IME_CHAR;
                break;
            case WM_CHAR:
                msg->message = WM_CUSTOM_CHAR;
                break;
            }

            LOG(debug, "Replace {:#x} to {:#x}: {:#x}", original, msg->message, msg->wParam);
        }
        break;
    }
    return CallNextHookEx(0, code, wParam, lParam);
}

void RenderManager::D3DInit()
{
    RealD3dInitFunc();

    LOG(info, "D3DInit Hooked!");
    auto render_manager = RE::BSGraphics::Renderer::GetSingleton();
    if (!render_manager)
    {
        LOG(err, "Cannot find render manager. Initialization failed!");
        return;
    }

    auto render_data = render_manager->data;

    LOG(info, "Getting SwapChain...");
    auto pSwapChain = render_data.renderWindows->swapChain;
    if (!pSwapChain)
    {
        LOG(err, "Cannot find SwapChain. Initialization failed!");
        return;
    }

    LOG(info, "Getting SwapChain desc...");
    DXGI_SWAP_CHAIN_DESC sd{};
    if (pSwapChain->GetDesc(std::addressof(sd)) < 0)
    {
        LOG(err, "IDXGISwapChain::GetDesc failed.");
        return;
    }

    auto device  = render_data.forwarder;
    auto context = render_data.context;

    LOG(info, "Initializing ImGui...");
    ImGui::CreateContext();
    ConfigImGui(sd.OutputWindow);

    hideChildWnd = new HideWnd();
    if (!hideChildWnd->Initialize(sd.OutputWindow))
    {
        LOG(err, "Can't initialize HideWnd.");
        return;
    }

    if (!ImGui_ImplWin32_Init(sd.OutputWindow))
    {
        LOG(err, "ImGui initialization failed (Win32)");
        return;
    }
    if (!ImGui_ImplDX11_Init(device, context))
    {
        LOG(err, "ImGui initialization failed (DX11)");
        return;
    }

    initialized.store(true);

    LOG(debug, "Hooking Skyrim WndProc...");
    RealWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(RenderManager::WndProc)));
    if (!RealWndProc)
    {
        LOG(err, "Hook WndProc failed!");
    }

    // focus to hiden child window
    hideChildWnd->Focus();
    LOG(debug, "Hooking Skyrim GetMsgProc...");
    HHOOK hhk = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId());
    if (!hhk)
    {
        LOG(err, "Hook MsgProc failed! error code: {}", GetLastError());
    }
}

void RenderManager::ConfigImGui(HWND hwnd)
{
    RECT     rect = {0, 0, 0, 0};
    ImGuiIO &io   = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    GetClientRect(hwnd, &rect);
    io.DisplaySize              = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
    io.MouseDrawCursor          = true;
    io.ConfigNavMoveSetMousePos = true;

    io.Fonts->AddFontFromFileTTF(config->eastAsiaFontFile.c_str(), config->fontSize, nullptr,
                                 io.Fonts->GetGlyphRangesChineseFull());

    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode                     = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar icons_ranges[] = {0x1F000, 0x1FFFF, 0}; // Will not be copied
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", config->fontSize, &cfg, icons_ranges);
    io.Fonts->Build();
    ImGui::GetMainViewport()->PlatformHandleRaw = (void *)hwnd;
    ImGuiStyle &style                           = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    LOG(info, "ImGui initialized!");
}

void RenderManager::D3DPresent(std::uint32_t a_p1)
{
    RealD3dPresentFunc(a_p1);
    if (!initialized.load() || !RenderManager::showWindow)
    {
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    render();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void RenderManager::render()
{
    ImGui::Begin("SkyrimTransmogrify");
    ImGui::Checkbox("Show Mods", &RenderManager::uiState.showArmors);
    if (RenderManager::uiState.showArmors)
    {
        RenderArmors();
    }

    if (ImGui::Button("Access player inventory"))
    {
        Transmogrify::AccessPlayerInventory();
    }
    ImGui::End();

    hideChildWnd->RenderImGui();
}

void RenderManager::RenderArmors()
{
    if (ImGui::BeginCombo("Mods", uiState.mods[uiState.selectedMod].c_str()))
    {
        for (int n = 0; n < uiState.mods.size(); n++)
        {
            const bool is_selected = (uiState.selectedMod == n);
            if (ImGui::Selectable(uiState.mods[n].c_str(), is_selected)) uiState.selectedMod = n;

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void RenderManager::SwitchShowWindow()
{
    showWindow = !showWindow;
}

void RenderManager::cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool RenderManager::IsShowWindow()
{
    return showWindow;
}
