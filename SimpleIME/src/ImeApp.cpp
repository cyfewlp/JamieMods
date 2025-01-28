#include "ImeApp.h"
#include "Hooks.hpp"
#include "SimpleIni.h"
#include "dxgi.h"
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "thread"
#include <Device.h>
#include <dinput.h>

namespace SimpleIME
{
    using namespace Hooks;
    auto *g_keyboard = new KeyboardDevice();
    char  key_state_buffer[256];

    void  ImeApp::Init()
    {
        gFontConfig = LoadConfig();
        gState      = new State();
        gState->Initialized.store(false);
        Hooks::InstallCreateWindowHook();
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

        auto *fontConfig             = new FontConfig();
        fontConfig->eastAsiaFontFile = ini.GetValue("General", "EastAsia_Font_File", R"(C:\Windows\Fonts\simsun.ttc)");
        fontConfig->emojiFontFile    = ini.GetValue("General", "Emoji_Font_File", R"(C:\Windows\Fonts\seguiemj.ttf)");
        fontConfig->fontSize         = (float)(ini.GetDoubleValue("General", "Font_Size", 16.0));
        ImeUI::fontSize              = fontConfig->fontSize;
        return fontConfig;
    }

    static CallHook<D3DInit::FuncType>            D3DInitHook(D3DInit::Address, ImeApp::D3DInit);
    static CallHook<D3DPresent::FuncType>         D3DPresentHook(D3DPresent::Address, ImeApp::D3DPresent);
    static CallHook<DispatchInputEvent::FuncType> DispatchInputEventHook(DispatchInputEvent::Address,
                                                                         ImeApp::DispatchEvent);

    void                                          ImeApp::D3DInit()
    {
        D3DInitHook();
        if (gState->Initialized.load()) return;

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

        try
        {
            g_keyboard->Initialize(sd.OutputWindow);
            logv(info, "create keyboard device Successful!");
        }
        catch (int code)
        {
            logv(err, "create keyboard device failed! code: {}", code);
            delete g_keyboard;
            return;
        }

        try
        {
            gImeWnd = new ImeWnd();
            gImeWnd->Initialize(sd.OutputWindow);
        }
        catch (std::runtime_error err)
        {
            logv(err, "Can't initialize ImeWnd.");
            delete gImeWnd;
            return;
        }

        gState->Initialized.store(true);

        logv(debug, "Hooking Skyrim WndProc...");
        RealWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
        if (!RealWndProc)
        {
            logv(err, "Hook WndProc failed!");
        }
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
        static const ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
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

    void ImeApp::D3DPresent(std::uint32_t ptr)
    {
        D3DPresentHook(ptr);
        if (!gState->Initialized.load() /*|| !gImeWnd->IsShow()*/)
        {
            return;
        }

        g_keyboard->Acquire(key_state_buffer, sizeof(key_state_buffer));
        if (key_state_buffer[DIK_F5] & 0x80)
        {
            logv(debug, "foucs popup window");
            gImeWnd->Focus();
        }

        // ImGui_ImplDX11_NewFrame();
        // ImGui_ImplWin32_NewFrame();
        // ImGui::NewFrame();

        // Render();

        // ImGui::Render();
        // ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        //// Update and Render additional Platform Windows
        // if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        //{
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        // }
    }

    void ImeApp::Render()
    {
        gImeWnd->RenderImGui();
    }

    void ImeApp::DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events)
    {
        ProcessEvent(a_events);
        // auto events = gImeWnd->FilterInputEvent(a_events);
        DispatchInputEventHook(a_dispatcher, a_events);
    }

    void ImeApp::ProcessEvent(RE::InputEvent **events)
    {
        for (auto event = *events; event; event = event->next)
        {
            const auto buttonEvent = event->AsButtonEvent();
            if (!buttonEvent) continue;

            RE::INPUT_DEVICE device = event->GetDevice();
            switch (device)
            {
                case RE::INPUT_DEVICE::kMouse: {
                    // ProcessMouseEvent(buttonEvent);
                    break;
                }
            }
        }
    }

    void ImeApp::ProcessMouseEvent(RE::ButtonEvent *btnEvent)
    {
        auto &io    = ImGui::GetIO();
        auto  value = btnEvent->Value();
        switch (auto mouseKey = btnEvent->GetIDCode())
        {
            case RE::BSWin32MouseDevice::Key::kWheelUp:
                io.AddMouseWheelEvent(0, value);
                break;
            case RE::BSWin32MouseDevice::Key::kWheelDown:
                io.AddMouseWheelEvent(0, value * -1);
                break;
            default:
                io.AddMouseButtonEvent(mouseKey, btnEvent->IsPressed());
                break;
        }
    }

    LRESULT ImeApp::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_SETFOCUS:
                // gImeWnd->Focus();
                return S_OK;
            default:
                break;
        }

        return RealWndProc(hWnd, msg, wParam, lParam);
    }
}