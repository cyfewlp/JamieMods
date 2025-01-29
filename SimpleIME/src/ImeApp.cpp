#include "ImeApp.h"
#include "Hooks.hpp"
#include "SimpleIni.h"
#include <Device.h>
#include <thread>

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
        catch (SimpleIMEException exce)
        {
            logv(err, "create keyboard device failed!: {}", exce.what());
            delete g_keyboard;
            return;
        }

        /*std::thread imeWndThread(
            [](HWND hWnd) {
                try
                {
                    gImeWnd = new ImeWnd();
                    gImeWnd->Initialize(hWnd, gFontConfig);
                }
                catch (SimpleIMEException exce)
                {
                    logv(err, "Thread ImeWnd failed. {}", exce.what());
                    delete gImeWnd;
                    return;
                }
                catch (...)
                {
                    logv(err, "Fatal error.");
                    delete gImeWnd;
                    return;
                }
            },
            sd.OutputWindow);
        imeWndThread.detach();*/

        try
        {
            gImeWnd = new ImeWnd();
            gImeWnd->Initialize(sd.OutputWindow, gFontConfig);
        }
        catch (SimpleIMEException exce)
        {
            logv(err, "Thread ImeWnd failed. {}", exce.what());
            delete gImeWnd;
            return;
        }
        catch (...)
        {
            logv(err, "Fatal error.");
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

    void ImeApp::D3DPresent(std::uint32_t ptr)
    {
        D3DPresentHook(ptr);
        if (!gState->Initialized.load())
        {
            return;
        }

        g_keyboard->Acquire(key_state_buffer, sizeof(key_state_buffer));
        if (key_state_buffer[DIK_F5] & 0x80)
        {
            logv(debug, "foucs popup window");
            gImeWnd->Focus();
        }
    }

    void ImeApp::DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events)
    {
        ProcessEvent(a_events);
        //auto events = gImeWnd->FilterInputEvent(a_events);
        DispatchInputEventHook(a_dispatcher, a_events);
    }

    inline bool inRange(int v, int min, int max)
    {
        return v >= min && v <= max;
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
                case RE::INPUT_DEVICE::kKeyboard: {
                    if (gImeWnd->IsImeEnabled())
                    {
                        auto code      = buttonEvent->idCode;
                        bool alphaCode = inRange(code, DIK_Q, DIK_P);
                        alphaCode |= inRange(code, DIK_A, DIK_L);
                        alphaCode |= inRange(code, DIK_Z, DIK_M);
                        if (alphaCode && buttonEvent->IsPressed())
                        {
                            logv(debug, "Focus to ImeWnd");
                            gImeWnd->Focus();
                        }
                    }
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
            case WM_INPUTLANGCHANGE:
                gImeWnd->SendMessage(msg, wParam, lParam);
                return S_OK;
            case WM_IME_NOTIFY: {
                switch (wParam)
                {
                    case IMN_SETOPENSTATUS:
                    case IMN_SETCONVERSIONMODE:
                    case IMN_SETSENTENCEMODE:
                        gImeWnd->SendMessage(msg, wParam, lParam);
                        return S_OK;
                }
            }
            case WM_CHAR:
                logv(debug, "MainWndProc {:#x}, {:#x}, {:#x}", msg, wParam, lParam);
                break;
            default:
                break;
        }

        return RealWndProc(hWnd, msg, wParam, lParam);
    }
}