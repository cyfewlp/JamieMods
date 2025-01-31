#include "ImeApp.h"
#include "Hooks.hpp"
#include "SimpleIni.h"
#include <thread>

namespace SimpleIME
{
    using namespace Hooks;
    char key_state_buffer[256];

    void ImeApp::Init()
    {
        g_pState.reset(new State());
        g_pState->Initialized.store(false);
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
        fontConfig->toolWindowShortcutKey = (uint32_t)(ini.GetLongValue("General", "Tool_Window_Shortcut_Key", DIK_F2));
        fontConfig->debug                 = ini.GetBoolValue("General", "debug", false);
        ImeUI::fontSize                   = fontConfig->fontSize;
        g_pFontConfig.reset(fontConfig);
        return fontConfig;
    }

    static CallHook<D3DInit::FuncType>            D3DInitHook(D3DInit::Address, ImeApp::D3DInit);
    static CallHook<D3DPresent::FuncType>         D3DPresentHook(D3DPresent::Address, ImeApp::D3DPresent);
    static CallHook<DispatchInputEvent::FuncType> DispatchInputEventHook(DispatchInputEvent::Address,
                                                                         ImeApp::DispatchEvent);

    void                                          ImeApp::D3DInit()
    {
        D3DInitHook();
        if (g_pState->Initialized.load()) return;

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

        g_hWnd = sd.OutputWindow;
        g_pKeyboard.reset(new KeyboardDevice(g_hWnd));

        try
        {
            g_pImeWnd.reset(new ImeWnd());
            g_pImeWnd->Initialize(g_hWnd);
            g_pImeWnd->Focus();
        }
        catch (SimpleIMEException exce)
        {
            logv(err, "Thread ImeWnd failed. {}", exce.what());
            g_pImeWnd.reset();
            g_pImeWnd = nullptr;
            return;
        }

        auto device  = render_data.forwarder;
        auto context = render_data.context;
        g_pImeWnd->InitImGui(device, context, g_pFontConfig.get());

        g_pState->Initialized.store(true);

        SKSE::GetTaskInterface()->AddUITask([]() { g_pImeWnd->SetImeOpenStatus(false); });

        logv(debug, "Hooking Skyrim WndProc...");
        RealWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
        if (!RealWndProc)
        {
            logv(err, "Hook WndProc failed!");
        }
    }

    void ImeApp::D3DPresent(std::uint32_t ptr)
    {
        D3DPresentHook(ptr);
        if (!g_pState->Initialized.load())
        {
            return;
        }

        if (g_pKeyboard && g_pKeyboard->GetState(key_state_buffer, sizeof(key_state_buffer)))
        {
            /*if (key_state_buffer[DIK_F2] & 0x80)
            {
                g_pImeWnd->ShowToolWindow();
            }*/
        }
        g_pImeWnd->RenderIme();
    }

    // we need set our keyboard to non-exclusive after game default.
    static bool firstEvent = true;

    void        ImeApp::DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events)
    {
        if (firstEvent)
        {
            ImeApp::ResetExclusiveMode();
            firstEvent = false;
        }
        static RE::InputEvent *dummy[] = {nullptr};
        ProcessEvent(a_events);
        auto discard = g_pImeWnd->IsDiscardGameInputEvents(a_events);
        if (discard) // Disable Game Input
        {
            DispatchInputEventHook(a_dispatcher, dummy);
        }
        else
        {
            DispatchInputEventHook(a_dispatcher, a_events);
        }
    }

    bool ImeApp::CheckAppState()
    {
        return g_pKeyboard->acquired.load();
    }

    // if sppecify recreate param, current g_pKeyboard will be delete
    // This is a insurance if we lost keyboard control
    bool ImeApp::ResetExclusiveMode()
    {
        try
        {
            g_pKeyboard->SetNonExclusive();
            logv(info, "Keyboard device now is non-exclusive.");
            return true;
        }
        catch (SimpleIMEException exception)
        {
            logv(err, "Change keyboard cooperative level failed: {}", exception.what());
            g_pKeyboard.release();
        }
        return false;
    }

    void ImeApp::ProcessEvent(RE::InputEvent **events)
    {
        for (auto event = *events; event; event = event->next)
        {
            auto eventType = event->GetEventType();
            switch (eventType)
            {
                case RE::INPUT_EVENT_TYPE::kButton: {
                    const auto buttonEvent = event->AsButtonEvent();
                    if (!buttonEvent) continue;

                    if (event->GetDevice() == RE::INPUT_DEVICE::kKeyboard)
                    {
                        if (buttonEvent->GetIDCode() == g_pFontConfig->toolWindowShortcutKey && buttonEvent->IsDown())
                        {
                            logv(debug, "show widnow pressed");
                            g_pImeWnd->ShowToolWindow();
                        }
                    }

                    if (event->GetDevice() != RE::INPUT_DEVICE::kMouse) continue;
                    ProcessMouseEvent(buttonEvent);
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
                g_pImeWnd->Focus();
                return S_OK;
            default:
                break;
        }

        return RealWndProc(hWnd, msg, wParam, lParam);
    }
}