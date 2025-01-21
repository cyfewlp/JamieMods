#include "ImeApp.h"
#include "HookUtils.h"
#include "Hooks.hpp"
#include "ImeWnd.hpp"
#include "SimpleIni.h"
#include "dxgi.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

namespace SimpleIME
{
    void ImeApp::Init()
    {
        gState = {};
        InstallHooks();
        gFontConfig = LoadConfig();
    }

    FontConfig *ImeApp::LoadConfig()
    {
        CSimpleIniA ini;

        ini.SetUnicode();
        SI_Error rc = ini.LoadFile(R"(Data\SKSE\Plugins\SimpleIME.ini)");
        if (rc < 0)
        { /* handle error */
            SKSE::stl::report_and_fail("Loading config failed.");
        }

        FontConfig *fontConfig       = new FontConfig();
        fontConfig->eastAsiaFontFile = ini.GetValue("General", "EastAsia_Font_File", R"(C:\Windows\Fonts\simsun.ttc)");
        fontConfig->emojiFontFile    = ini.GetValue("General", "Emoji_Font_File", R"(C:\Windows\Fonts\seguiemj.ttf)");
        fontConfig->fontSize         = (float)(ini.GetDoubleValue("General", "Font_Size", 16.0));
        return fontConfig;
    }

    void ImeApp::InstallHooks()
    {
        SimpleIME::HookGetMsgProc();
        D3DInitHook    d3DInitHook;
        D3DPresentHook d3DPresentHook;
        // install hooks
        static HookUtils::CallHook<void()>               RealD3dInitHook(d3DInitHook.address, []() {
            RealD3dInitHook();
            D3DInit();
        });

        static HookUtils::CallHook<void(std::uintptr_t)> RealD3dPresentHook(d3DPresentHook.address,
                                                                            [](std::uintptr_t ptr) {
                                                                                RealD3dPresentHook(ptr);
                                                                                D3DPresent();
                                                                            });
    }

    void ImeApp::D3DInit()
    {
        if (gState->Initialized) return;

        auto render_manager = RE::BSGraphics::Renderer::GetSingleton();
        if (!render_manager)
        {
            logv(err, "Cannot find render manager. Initialization failed!");
            return;
        }

        auto render_data = render_manager->data;
        logv(debug, "Getting SwapChain...");
        auto pSwapChain = render_data.renderWindows->swapChain;
        if (!pSwapChain)
        {
            logv(err, "Cannot find SwapChain. Initialization failed!");
            return;
        }

        logv(debug, "Getting SwapChain desc...");
        DXGI_SWAP_CHAIN_DESC sd{};
        if (pSwapChain->GetDesc(std::addressof(sd)) < 0)
        {
            logv(err, "IDXGISwapChain::GetDesc failed.");
            return;
        }

        auto device  = render_data.forwarder;
        auto context = render_data.context;
        InitImGui(sd.OutputWindow, device, context);

        gImeWnd = {};
        gImeWnd.Initialize(sd.OutputWindow);
        if (!gImeWnd.Initialize(sd.OutputWindow))
        {
            logv(err, "Can't initialize ImeWnd.");
            return;
        }

        gState->Initialized.store(true);
    }

    void ImeApp::InitImGui(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context)
    {
        logv(info, "Initializing ImGui...");
        ImGui::CreateContext();

        if (!ImGui_ImplWin32_Init(hWnd))
        {
            logv(err, "ImGui initialization failed (Win32)");
            return;
        }

        if (!ImGui_ImplDX11_Init(device, context))
        {
            logv(err, "ImGui initialization failed (DX11)");
            return;
        }

        RECT     rect = {0, 0, 0, 0};
        ImGuiIO &io   = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        GetClientRect(hWnd, &rect);
        io.DisplaySize              = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
        io.MouseDrawCursor          = true;
        io.ConfigNavMoveSetMousePos = true;

        io.Fonts->AddFontFromFileTTF(gFontConfig->eastAsiaFontFile.c_str(), gFontConfig->fontSize, nullptr,
                                     io.Fonts->GetGlyphRangesChineseFull());

        // config font
        static ImFontConfig cfg;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.MergeMode                     = true;
        cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
        static const ImWchar icons_ranges[] = {0x1F000, 0x1FFFF, 0}; // Will not be copied
        io.Fonts->AddFontFromFileTTF(gFontConfig->emojiFontFile.c_str(), gFontConfig->fontSize, &cfg, icons_ranges);
        io.Fonts->Build();
        ImGui::GetMainViewport()->PlatformHandleRaw = (void *)hWnd;
        ImGuiStyle &style                           = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        logv(info, "ImGui initialized!");
    }

    void ImeApp::D3DPresent()
    {
        if (!gState->Initialized.load())
        {
            return;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Render();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImeApp::Render()
    {
        gImeWnd.RenderImGui();
    }
}