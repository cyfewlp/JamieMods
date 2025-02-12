
#include "ImeApp.h"
#include "AppConfig.h"
#include "Hooks.hpp"
#include "ImeWnd.hpp"
#include "gsl/gsl"
#include "imgui.h"
#include <basetsd.h>
#include <cstdint>
#include <future>
#include <memory>
#include <thread>

namespace LIBC_NAMESPACE_DECL
{

    bool PluginInit()
    {
        InitializeMessaging();
        SimpleIME::ImeApp::Init();
        return true;
    }

    void InitializeMessaging()
    {
        // Delay call focus to avoid other crash when not create ImGui context not yet.
        SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *a_msg) {
            if (a_msg->type == SKSE::MessagingInterface::kPostPostLoad)
            {
                SimpleIME::ImeApp::GetImeWnd()->Focus();
            }
        });
    }

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
            g_pState = std::make_unique<State>();
            g_pState->Initialized.store(false);
            Hooks::InstallRegisterClassHook();
            Hooks::InstallDirectInPutHook();
        }

        static void FatalError(const char *msg)
        {
            log_error("Can't Initialize ImeWnd because {}", msg);
            SKSE::stl::report_and_error("SimpleIME initialize failed!");
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
                FatalError("Cannot find render manager. Initialization failed!");
            }

            auto render_data = render_manager->data;
            log_debug("Getting SwapChain...");
            auto *pSwapChain = render_data.renderWindows->swapChain;
            if (pSwapChain == nullptr)
            {
                FatalError("Cannot find SwapChain. Initialization failed!");
            }

            log_debug("Getting SwapChain desc...");
            auto swapChainDesc = gsl::not_null(new DXGI_SWAP_CHAIN_DESC());
            if (pSwapChain->GetDesc(swapChainDesc) < 0)
            {
                FatalError("IDXGISwapChain::GetDesc failed.");
            }
            g_hWnd = swapChainDesc->OutputWindow;

            std::promise<bool> ensureInitialized;
            std::future<bool>  initialized = ensureInitialized.get_future();
            // run ImeWnd in a standalone thread
            g_pImeWnd = std::make_unique<ImeWnd>();
            std::thread childWndThread([&ensureInitialized]() {
                try
                {
                    g_pImeWnd->Initialize();
                    ensureInitialized.set_value(true);
                    g_pImeWnd->Start(g_hWnd);
                }
                catch (...)
                {
                    try
                    {
                        ensureInitialized.set_exception(std::current_exception());
                    }
                    catch (...)
                    { // set_exception() may throw too
                    }
                }
            });
            try
            {
                initialized.get();
                childWndThread.detach();
                auto *device  = render_data.forwarder;
                auto *context = render_data.context;
                g_pImeWnd->InitImGui(g_hWnd, device, context);
            }
            catch (const std::exception &e)
            {
                g_pImeWnd.reset();
                g_pImeWnd = nullptr;
                FatalError(e.what());
            }

            g_pState->Initialized.store(true);

            SKSE::GetTaskInterface()->AddUITask([]() { g_pImeWnd->SetImeOpenStatus(false); });

            log_debug("Hooking Skyrim WndProc...");
            RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(swapChainDesc->OutputWindow, GWLP_WNDPROC,
                                                                      reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
            if (RealWndProc == nullptr)
            {
                FatalError("Hook WndProc failed!");
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
            if (g_pImeWnd->IsDiscardGameInputEvents(a_events)) // Disable Game Input
            {
                DispatchInputEventHook(a_dispatcher, dummy);
            }
            else
            {
                DispatchInputEventHook(a_dispatcher, a_events);
            }
        }

        auto ImeApp::GetImeWnd() -> ImeWnd *
        {
            return g_pImeWnd.get();
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
                            if (pButtonEvent->GetIDCode() == AppConfig::Load()->GetToolWindowShortcutKey() &&
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
