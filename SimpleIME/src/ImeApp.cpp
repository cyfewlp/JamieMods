
#include "ImeApp.h"
#include "Configs.h"
#include "Hooks.hpp"
#include "ImeWnd.hpp"
#include "gsl/gsl"
#include "imgui.h"
#include <RE/B/BSWin32MouseDevice.h>
#include <RE/I/InputDevices.h>
#include <SimpleIni.h>
#include <basetsd.h>
#include <cstdint>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        static const auto D3DInitHook            = Hooks::D3DInitHookData(ImeApp::D3DInit);
        static const auto D3DPresentHook         = Hooks::D3DPresentHookData(ImeApp::D3DPresent);
        static const auto DispatchInputEventHook = Hooks::DispatchInputEventHookData(ImeApp::DispatchEvent);

        /**
         * Init ImeApp
         */
        void ImeApp::Init()
        {
            g_pState            = std::make_unique<State>();
            g_pState->Initialized.store(false);
            Hooks::InstallRegisterClassHook();
            Hooks::InstallDirectInPutHook();
        }

        auto ImeApp::LoadConfig() -> FontConfig *
        {
            CSimpleIniA ini;

            ini.SetUnicode();
            SI_Error const error = ini.LoadFile(R"(Data\SKSE\Plugins\SimpleIME.ini)");
            if (error < 0)
            {
                SKSE::stl::report_and_fail("Loading config failed.");
            }
            g_pFontConfig = std::make_unique<FontConfig>();
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
                log_error("Cannot find render manager. Initialization failed!");
                return;
            }

            auto render_data = render_manager->data;
            log_debug("Getting SwapChain...");
            auto *pSwapChain = render_data.renderWindows->swapChain;
            if (pSwapChain == nullptr)
            {
                log_error("Cannot find SwapChain. Initialization failed!");
                return;
            }

            log_debug("Getting SwapChain desc...");
            auto swapChainDesc = gsl::not_null(new DXGI_SWAP_CHAIN_DESC());
            if (pSwapChain->GetDesc(swapChainDesc) < 0)
            {
                log_error("IDXGISwapChain::GetDesc failed.");
                return;
            }

            g_hWnd = swapChainDesc->OutputWindow;

            try
            {
                g_pImeWnd = std::make_unique<ImeWnd>();
                g_pImeWnd->Initialize(g_hWnd);
                g_pImeWnd->Focus();
            }
            catch (SimpleIMEException &e)
            {
                log_error("Thread ImeWnd failed. {}", e.what());
                g_pImeWnd.reset();
                g_pImeWnd = nullptr;
                return;
            }

            auto *device  = render_data.forwarder;
            auto *context = render_data.context;
            g_pImeWnd->InitImGui(device, context, g_pFontConfig.get());

            g_pState->Initialized.store(true);

            SKSE::GetTaskInterface()->AddUITask([]() { g_pImeWnd->SetImeOpenStatus(false); });

            log_debug("Hooking Skyrim WndProc...");
            RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(swapChainDesc->OutputWindow, GWLP_WNDPROC,
                                                                      reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
            if (RealWndProc == nullptr)
            {
                log_error("Hook WndProc failed!");
            }
        }

        void ImeApp::D3DPresent(std::uint32_t ptr)
        {
            D3DPresentHook(ptr);
            if (!g_pState->Initialized.load())
            {
                return;
            }

            g_pImeWnd->RenderIme();
        }

        // we need set our keyboard to non-exclusive after game default.
        void ImeApp::DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events)
        {
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

        void ImeApp::ProcessEvent(RE::InputEvent **events)
        {
            if (events == nullptr)
            {
                return;
            }
            auto *head = *events;
            if (head == nullptr)
            {
                return;
            }

            switch (head->GetEventType())
            {
                case RE::INPUT_EVENT_TYPE::kMouseMove: {
                    auto *cursor = RE::MenuCursor::GetSingleton();
                    ImGui::GetIO().AddMousePosEvent(cursor->cursorPosX, cursor->cursorPosY);
                    break;
                }
                case RE::INPUT_EVENT_TYPE::kButton: {
                    auto *const pButtonEvent = head->AsButtonEvent();
                    if (pButtonEvent == nullptr)
                    {
                        return;
                    }

                    switch (head->GetDevice())
                    {
                        case RE::INPUT_DEVICE::kKeyboard: {
                            if (pButtonEvent->GetIDCode() == g_pFontConfig->GetToolWindowShortcutKey() &&
                                pButtonEvent->IsDown())
                            {
                                g_pImeWnd->ShowToolWindow();
                            }
                            break;
                        }
                        case RE::INPUT_DEVICE ::kMouse:
                            ProcessMouseEvent(pButtonEvent);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void ImeApp::ProcessMouseEvent(RE::ButtonEvent *btnEvent)
        {
            auto &imGuiIo = ImGui::GetIO();
            auto  value   = btnEvent->Value();
            switch (auto mouseKey = btnEvent->GetIDCode())
            {
                case RE::BSWin32MouseDevice::Key::kWheelUp:
                    imGuiIo.AddMouseWheelEvent(0, value);
                    break;
                case RE::BSWin32MouseDevice::Key::kWheelDown:
                    imGuiIo.AddMouseWheelEvent(0, value * -1);
                    break;
                default:
                    imGuiIo.AddMouseButtonEvent(static_cast<int>(mouseKey), btnEvent->IsPressed());
                    break;
            }
        }

        auto ImeApp::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
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
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
