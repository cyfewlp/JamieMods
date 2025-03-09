
#include "ImeApp.h"
#include "ImeWnd.hpp"
#include "common/common.h"
#include "common/hook.h"
#include "common/log.h"
#include "configs/AppConfig.h"
#include "context.h"
#include "core/EventHandler.h"
#include "gsl/gsl"
#include "hooks/Hooks.hpp"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"
#include "hooks/WinHooks.h"
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
        using State = Ime::Core::State;
        SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message *a_msg) {
            if (a_msg->type == SKSE::MessagingInterface::kPreLoadGame)
            {
                State::GetInstance()->Set(State::GAME_LOADING);
            }
            else if (a_msg->type == SKSE::MessagingInterface::kPostLoadGame)
            {
                State::GetInstance()->Clear(State::GAME_LOADING);
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
            // Hooks::InstallRegisterClassHook();
            Hooks::WinHooks::InstallHooks();

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

            if (GetInstance().m_imeWnd.SendMessage(WM_QUIT, -1, 0) != S_OK)
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

            app.OnD3DInit();
        }

        void ImeApp::OnD3DInit()
        {
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
            REX::W32::DXGI_SWAP_CHAIN_DESC swapChainDesc;
            if (pSwapChain->GetDesc(&swapChainDesc) < 0)
            {
                throw SimpleIMEException("IDXGISwapChain::GetDesc failed.");
            }

            m_hWnd = reinterpret_cast<HWND>(swapChainDesc.outputWindow);
            Start(render_data);
            m_state.Initialized.store(true);

            log_debug("Hooking Skyrim WndProc...");
            RealWndProc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
            if (RealWndProc == nullptr)
            {
                throw SimpleIMEException("Hook WndProc failed!");
            }
            InstallHooks();
        }

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
            auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
            auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);
            m_imeWnd.InitImGui(m_hWnd, device, context);
        }

        void ImeApp::InstallHooks()
        {
            auto &app                  = GetInstance();
            app.D3DPresentHook         = Hooks::D3DPresentHookData(D3DPresent);
            app.DispatchInputEventHook = Hooks::DispatchInputEventHookData(DispatchEvent);

            Hooks::ScaleformHooks::InstallHooks();
            Hooks::UiHooks::InstallHooks();
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

            auto &app                 = GetInstance();
            bool  discardCurrentEvent = false;
            app.ProcessEvent(a_events, discardCurrentEvent);
            if (discardCurrentEvent)
            {
                app.DispatchInputEventHook->Original(a_dispatcher, dummy);
            }
            else
            {
                app.DispatchInputEventHook->Original(a_dispatcher, a_events);
            }
            app.DispatchInputEventHook->Original(a_dispatcher, a_events);
            Core::EventHandler::PostHandleKeyboardEvent();
        }

        void ImeApp::ProcessEvent(RE::InputEvent **events, bool &discard)
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
                            ProcessKeyboardEvent(pButtonEvent, discard);
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

        void ImeApp::ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent, bool &discard)
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
            Core::EventHandler::HandleKeyboardEvent(btnEvent);
            discard = Core::EventHandler::IsDiscardKeyboardEvent(btnEvent);
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
