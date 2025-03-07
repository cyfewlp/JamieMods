
#include "ImeApp.h"
#include "ImeWnd.hpp"
#include "common/common.h"
#include "common/hook.h"
#include "common/log.h"
#include "configs/AppConfig.h"
#include "context.h"
#include "gsl/gsl"
#include "hooks/Hooks.hpp"
#include "hooks/ScaleformHook.h"
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
        Ime::ImeApp::GetInstance().Initialize();
        return true;
    }

    void InitializeMessaging()
    {
        SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *a_msg) {
            if (a_msg->type == SKSE::MessagingInterface::kPreLoadGame)
            {
                Ime::Context::GetInstance()->SetIsGameLoading(true);
            }
            else if (a_msg->type == SKSE::MessagingInterface::kPostLoadGame)
            {
                Ime::Context::GetInstance()->SetIsGameLoading(false);
            }
        });
    }

    namespace Ime
    {

        ImeApp ImeApp::instance{};

        auto ImeApp::GetInstance() -> ImeApp &
        {
            return instance;
        }

        /**
         * Init ImeApp
         */
        void ImeApp::Initialize()
        {
            m_state.Initialized.store(false);
            Hooks::InstallRegisterClassHook();
            Hooks::InstallDirectInputHook();

            D3DInitHook = Hooks::D3DInitHookData(ImeApp::D3DInit);
        }

        class InitErrorMessageShow final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
        public:
            RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent *a_event,
                                                  RE::BSTEventSource<RE::MenuOpenCloseEvent> *) override
            {
                if (a_event->menuName == RE::MainMenu::MENU_NAME && a_event->opening)
                {
                    auto &messages = Context::GetInstance()->Messages();
                    while (!messages.empty())
                    {
                        RE::DebugMessageBox(messages.front().c_str());
                        messages.pop();
                    }
                }
                return RE::BSEventNotifyControl::kContinue;
            }
        };

        std::unique_ptr<InitErrorMessageShow> g_pInitErrorMessageShow(nullptr);

        void ImeApp::Start(RE::BSGraphics::RendererData &renderData)
        {
            std::promise<bool> ensureInitialized;
            std::future<bool>  initialized = ensureInitialized.get_future();
            // run ImeWnd in a standalone thread
            std::thread childWndThread([&ensureInitialized, this]() {
                try
                {
                    m_imeWnd.Initialize();
                    ensureInitialized.set_value(true);
                    m_imeWnd.Start(m_hWnd);
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

            initialized.get();
            childWndThread.detach();
            auto *device  = renderData.forwarder;
            auto *context = renderData.context;
            m_imeWnd.InitImGui(m_hWnd, device, context);
        }

        void ImeApp::D3DInit()
        {
            if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
            {
                g_pInitErrorMessageShow.reset(new InitErrorMessageShow());
                ui->AddEventSink(g_pInitErrorMessageShow.get());
            }
            try
            {
                DoD3DInit();
                return;
            }
            catch (std::exception &error)
            {
                const auto message = std::format("SimpleIME initialize fail: \n {}", error.what());
                log_error(message.c_str());
                Context::GetInstance()->PushMessage(message);
            }
            catch (...)
            {
                const auto message = std::string("SimpleIME initialize fail: \n occur unexpected error.");
                log_error(message.c_str());
                Context::GetInstance()->PushMessage(message);
            }
            LogStacktrace();
            log_info("Force close ImeWnd.");

            if (GetInstance().m_imeWnd.SendMessage_(WM_QUIT, -1, 0) != S_OK)
            {
                log_error("Send WM_QUIT to ImeWnd failed.");
            }
        }

        void ImeApp::DoD3DInit()
        {
            auto &app = GetInstance();
            if (!app.D3DInitHook.has_value())
            {
                return;
            }
            app.D3DInitHook->Original();
            if (app.m_state.Initialized.load())
            {
                return;
            }

            auto *render_manager = RE::BSGraphics::Renderer::GetSingleton();
            if (render_manager == nullptr)
            {
                throw SimpleIMEException("Cannot find render manager. Initialization failed!");
            }

            auto &render_data = render_manager->data;
            log_debug("Getting SwapChain...");
            auto *pSwapChain = render_data.renderWindows->swapChain;
            if (pSwapChain == nullptr)
            {
                throw SimpleIMEException("Cannot find SwapChain. Initialization failed!");
            }

            log_debug("Getting SwapChain desc...");
            auto swapChainDesc = gsl::not_null(new DXGI_SWAP_CHAIN_DESC());
            if (pSwapChain->GetDesc(swapChainDesc) < 0)
            {
                throw SimpleIMEException("IDXGISwapChain::GetDesc failed.");
            }

            app.m_hWnd = swapChainDesc->OutputWindow;
            app.Start(render_data);
            app.m_state.Initialized.store(true);

            log_debug("Hooking Skyrim WndProc...");
            RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(swapChainDesc->OutputWindow, GWLP_WNDPROC,
                                                                      reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
            if (RealWndProc == nullptr)
            {
                throw SimpleIMEException("Hook WndProc failed!");
            }
            InstallHooks();
        }

        void ImeApp::InstallHooks()
        {
            auto &app                  = GetInstance();
            app.D3DPresentHook         = Hooks::D3DPresentHookData(D3DPresent);
            app.DispatchInputEventHook = Hooks::DispatchInputEventHookData(DispatchEvent);

            if (AppConfig::GetConfig().AlwaysActiveIme())
            {
                log_info("Ime won't enable dynamic activate because 'Always_Active_Ime' is true");
            }
            else
            {
                Hooks::ScaleformHooks::InstallHooks();
            }
        }

        void ImeApp::D3DPresent(std::uint32_t ptr)
        {
            auto &app = GetInstance();
            if (!app.D3DPresentHook.has_value())
            {
                return;
            }
            app.D3DPresentHook->Original(ptr);
            if (!app.m_state.Initialized.load())
            {
                return;
            }
            app.m_imeWnd.RenderIme();
        }

        // we need set our keyboard to non-exclusive after game default.
        void ImeApp::DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events)
        {
            static RE::InputEvent *dummy[] = {nullptr};
            auto                  &app     = GetInstance();
            app.ProcessEvent(a_events);
            if (app.m_imeWnd.IsDiscardGameInputEvents(a_events)) // Disable Game Input
            {
                app.DispatchInputEventHook->Original(a_dispatcher, dummy);
            }
            else
            {
                app.DispatchInputEventHook->Original(a_dispatcher, a_events);
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
                        case RE::INPUT_DEVICE::kKeyboard:
                            ProcessKeyboardEvent(pButtonEvent);
                            break;
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

        void ImeApp::ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent)
        {
            auto keyCode = btnEvent->GetIDCode();
            if (keyCode == AppConfig::GetConfig().GetToolWindowShortcutKey() && btnEvent->IsDown())
            {
                m_imeWnd.ShowToolWindow();
            }
            using Key = RE::BSKeyboardDevice::Keys::Key;
            if (keyCode == Key::kTilde)
            {
                m_imeWnd.AbortIme();
            }
        }

        void ImeApp::ProcessMouseEvent(const RE::ButtonEvent *btnEvent)
        {
            using RE::BSWin32MouseDevice;
            auto &imGuiIo = ImGui::GetIO();
            if (!imGuiIo.WantCaptureMouse)
            {
                if (btnEvent->GetIDCode() < BSWin32MouseDevice::Keys::kWheelUp)
                {
                    m_imeWnd.AbortIme();
                }
            }
            else
            {
                auto value = btnEvent->Value();
                switch (auto mouseKey = btnEvent->GetIDCode())
                {
                    case BSWin32MouseDevice::Keys::kWheelUp:
                        imGuiIo.AddMouseWheelEvent(0, value);
                        break;
                    case BSWin32MouseDevice::Keys::kWheelDown:
                        imGuiIo.AddMouseWheelEvent(0, value * -1);
                        break;
                    default:
                        imGuiIo.AddMouseButtonEvent(static_cast<int>(mouseKey), btnEvent->IsPressed());
                        break;
                }
            }
        }

        auto ImeApp::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            auto &app = GetInstance();
            switch (uMsg)
            {
                case WM_ACTIVATE:
                    if (LOWORD(wParam) != WA_INACTIVE)
                    {
                        app.m_imeWnd.Focus();
                    }
                    break;
                case WM_SYSCOMMAND: {
                    if ((wParam & 0xFFF0) == SC_RESTORE)
                    {
                        app.m_imeWnd.Focus();
                    }
                    break;
                }
                case WM_SETFOCUS:
                    app.m_imeWnd.Focus();
                    return S_OK;
                case WM_IME_SETCONTEXT:
                    return ::DefWindowProc(hWnd, uMsg, wParam, 0);
                default:
                    break;
            }
            return RealWndProc(hWnd, uMsg, wParam, lParam);
        }
    } // namespace Ime
} // namespace LIBC_NAMESPACE_DECL
