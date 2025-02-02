#include "ImeApp.h"
#include "Configs.h"
#include "Hooks.hpp"
#include <SimpleIni.h>
#include <array>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    const std::array<char, 256> g_KeyStateBuffer{};

    namespace SimpleIME
    {
        using namespace Hooks;

        static const CallHook<D3DInit::FuncType>            D3DInitHook(D3DInit::Address, ImeApp::D3DInit);
        static const CallHook<D3DPresent::FuncType>         D3DPresentHook(D3DPresent::Address, ImeApp::D3DPresent);
        static const CallHook<DispatchInputEvent::FuncType> DispatchInputEventHook(DispatchInputEvent::Address,
                                                                                   ImeApp::DispatchEvent);

        /**
         * Init ImeApp
         */
        void ImeApp::Init()
        {
            g_pState = std::make_unique<State>();
            g_pState->Initialized.store(false);
            Hooks::InstallCreateWindowHook();
        }

        FontConfig *ImeApp::LoadConfig()
        {
            CSimpleIniA ini;

            ini.SetUnicode();
            SI_Error error = ini.LoadFile(R"(Data\SKSE\Plugins\SimpleIME.ini)");
            if (error < 0)
            {
                SKSE::stl::report_and_fail("Loading config failed.");
            }
            g_pFontConfig.reset(new FontConfig());
            g_pFontConfig->of(ini);
            return g_pFontConfig.get();
        }

        void ImeApp::D3DInit()
        {
            D3DInitHook();
            if (g_pState->Initialized.load())
            {
                return;
            }

            auto *render_manager = RE::BSGraphics::Renderer::GetSingleton();
            if (render_manager == nullptr)
            {
                logv(err, "Cannot find render manager. Initialization failed!");
                return;
            }

            auto render_data = render_manager->data;
            logv(debug, "Getting SwapChain...");
            auto *pSwapChain = render_data.renderWindows->swapChain;
            if (pSwapChain == nullptr)
            {
                logv(err, "Cannot find SwapChain. Initialization failed!");
                return;
            }

            logv(debug, "Getting SwapChain desc...");
            DXGI_SWAP_CHAIN_DESC swapChainDesc{};
            if (pSwapChain->GetDesc(std::addressof(swapChainDesc)) < 0)
            {
                logv(err, "IDXGISwapChain::GetDesc failed.");
                return;
            }

            g_hWnd      = swapChainDesc.OutputWindow;
            g_pKeyboard = std::make_unique<KeyboardDevice>(g_hWnd);

            try
            {
                g_pImeWnd = std::make_unique<ImeWnd>();
                g_pImeWnd->Initialize(g_hWnd);
                g_pImeWnd->Focus();
            }
            catch (SimpleIMEException &e)
            {
                logv(err, "Thread ImeWnd failed. {}", e.what());
                g_pImeWnd.reset();
                g_pImeWnd = nullptr;
                return;
            }

            auto *device  = render_data.forwarder;
            auto *context = render_data.context;
            g_pImeWnd->InitImGui(device, context, g_pFontConfig.get());

            g_pState->Initialized.store(true);

            SKSE::GetTaskInterface()->AddUITask([]() { g_pImeWnd->SetImeOpenStatus(false); });

            logv(debug, "Hooking Skyrim WndProc...");
            RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(swapChainDesc.OutputWindow, GWLP_WNDPROC,
                                                                      reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
            if (RealWndProc == nullptr)
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

            if (g_pKeyboard && g_pKeyboard->GetState((LPVOID)g_KeyStateBuffer.data(), (DWORD)g_KeyStateBuffer.size()))
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

        // if specify recreate param, current g_pKeyboard will be delete
        // This is a insurance if we lost keyboard control
        bool ImeApp::ResetExclusiveMode()
        {
            try
            {
                g_pKeyboard->SetNonExclusive();
                logv(info, "Keyboard device now is non-exclusive.");
                return true;
            }
            catch (SimpleIMEException &exception)
            {
                logv(err, "Change keyboard cooperative level failed: {}", exception.what());
            }
            return false;
        }

        void ImeApp::ProcessEvent(RE::InputEvent **events)
        {
            for (auto *event = *events; event != nullptr; event = event->next)
            {
                auto eventType = event->GetEventType();
                switch (eventType)
                {
                    case RE::INPUT_EVENT_TYPE::kButton: {
                        auto *const buttonEvent = event->AsButtonEvent();
                        if (buttonEvent == nullptr)
                        {
                            continue;
                        }

                        if (event->GetDevice() == RE::INPUT_DEVICE::kKeyboard)
                        {
                            if (buttonEvent->GetIDCode() == g_pFontConfig->GetToolWindowShortcutKey() &&
                                buttonEvent->IsDown())
                            {
                                logv(debug, "show window pressed");
                                g_pImeWnd->ShowToolWindow();
                            }
                        }

                        if (event->GetDevice() != RE::INPUT_DEVICE::kMouse)
                        {
                            continue;
                        }
                        ProcessMouseEvent(buttonEvent);
                        break;
                    }
                    default:
                        break;
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
                    io.AddMouseButtonEvent((int)mouseKey, btnEvent->IsPressed());
                    break;
            }
        }

        LRESULT ImeApp::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
                case WM_ACTIVATE:
                    if (wParam != WA_INACTIVE)
                    {
                        ImeApp::ResetExclusiveMode();
                        logv(debug, "MainWndProc Actived");
                    }
                    else
                    {
                        logv(debug, "MainWndProc Inactived");
                    }
                    break;
                case WM_SYSCOMMAND:
                    switch (wParam & 0xFFF0)
                    {
                        case SC_TASKLIST:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- TASKLIST");
                            break;
                        case SC_MOVE:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- MOVE");
                            break;
                        case SC_NEXTWINDOW:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- NEXTWINDOW");
                            break;
                        case SC_PREVWINDOW:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- PREVWINDOW");
                            break;
                        case SC_SIZE:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- SIZE");
                            break;
                        case SC_CLOSE:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- CLOSE");
                            break;
                        case SC_RESTORE:
                            ImeApp::ResetExclusiveMode();
                            logv(debug, "MainWndProc WM_SYSCOMMAND- RESTORE");
                            break;
                        case SC_CONTEXTHELP:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- CONTEXTHELP");
                            break;
                        case SC_DEFAULT:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- DEFAULT");
                            break;
                        case SC_HOTKEY:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- HOTKEY");
                            break;
                        case SC_HSCROLL:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- HSCROLL");
                            break;
                        case SC_KEYMENU:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- KEYMENU");
                            break;
                        case SC_MAXIMIZE:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- MAXIMIZE");
                            break;
                        case SC_MINIMIZE:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- MINIMIZE");
                            break;
                        case SC_SCREENSAVE:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- SCREENSAVE");
                            break;
                        case SC_MOUSEMENU:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- MOUSEMENU");
                            break;
                        case SC_MONITORPOWER:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- MONITORPOWER");
                            break;
                        case SC_VSCROLL:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- VSCROLL");
                            break;
                        default:
                            logv(debug, "MainWndProc WM_SYSCOMMAND- {:#x}", wParam);
                            break;
                    }
                    break;
                case WM_KILLFOCUS:
                    logv(debug, "MainWndProc WM_KILLFOCUS");
                    break;
                case WM_SETFOCUS:
                    logv(debug, "MainWndProc WM_SETFOCUS");
                    ImeApp::ResetExclusiveMode();
                    g_pImeWnd->Focus();
                    return S_OK;
                default:
                    break;
            }

            return RealWndProc(hWnd, msg, wParam, lParam);
        }
    }
}
