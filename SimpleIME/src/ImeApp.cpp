
#include "ImeApp.h"
#include "ImeWnd.hpp"
#include "SimpleImeSupport.h"
#include "common/common.h"
#include "common/hook.h"
#include "common/log.h"
#include "context.h"
#include "core/EventHandler.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"
#include "hooks/WinHooks.h"
#include "ime/ImeManagerComposer.h"
#include "ime/ImeSupportUtils.h"

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
                State::GetInstance().Set(State::GAME_LOADING);
            }
            else if (a_msg->type == SKSE::MessagingInterface::kPostLoadGame)
            {
                State::GetInstance().Clear(State::GAME_LOADING);
            }
            else if (a_msg->type == SKSE::MessagingInterface::kInputLoaded)
            {
                Ime::ImeApp::GetInstance().OnInputLoaded();
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
            m_fInitialized.store(false);
            // TODO: Read type by config?
            Hooks::WinHooks::InstallHooks();

            D3DInitHook = std::make_unique<Hooks::D3DInitHookData>(ImeApp::D3DInit);
        }

        void ImeApp::Uninitialize()
        {
            if (m_fInitialized)
            {
                Hooks::WinHooks::UninstallHooks();
                D3DInitHook.reset();
                D3DInitHook = nullptr;
                UninstallHooks();
            }
            ImeManagerComposer::GetInstance()->PopType();
            m_fInitialized.store(false);
        }

        void ImeApp::OnInputLoaded()
        {
            Core::EventHandler::InstallEventSink(&m_imeWnd);
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

            if (GetInstance().m_imeWnd.SendNotifyMessageToIme(WM_QUIT, -1, 0) != S_OK)
            {
                log_error("Send WM_QUIT to ImeWnd failed.");
            }
        }

        void ImeApp::DoD3DInit()
        {
            auto &app = GetInstance();
            app.D3DInitHook->Original();
            if (app.m_fInitialized.load())
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
            m_fInitialized.store(true);

            log_debug("Hooking Skyrim WndProc...");
            RealWndProc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ImeApp::MainWndProc)));
            if (RealWndProc == nullptr)
            {
                throw SimpleIMEException("Hook WndProc failed!");
            }
            InstallHooks();
            BroadcastImeIntegrationMessage();
        }

        void ImeApp::BroadcastImeIntegrationMessage()
        {
            static SimpleIME::IntegrationData g_IntegrationData //
                = {.RenderIme               = DoD3DPresent,
                   .EnableIme               = ImeSupportUtils::EnableIme,
                   .PushContext             = ImeSupportUtils::PushContext,
                   .PopContext              = ImeSupportUtils::PopContext,
                   .UpdateImeWindowPosition = ImeSupportUtils::UpdateImeWindowPosition,
                   .IsWantCaptureInput      = ImeSupportUtils::IsWantCaptureInput};
            ImeSupportUtils::BroadcastImeIntegrationMessage(&g_IntegrationData);
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
            D3DPresentHook         = std::make_unique<Hooks::D3DPresentHookData>(D3DPresent);
            DispatchInputEventHook = std::make_unique<Hooks::DispatchInputEventHookData>(DispatchEvent);

            Hooks::ScaleformHooks::InstallHooks();
            Hooks::UiHooks::InstallHooks();
        }

        void ImeApp::UninstallHooks()
        {
            D3DPresentHook         = nullptr;
            DispatchInputEventHook = nullptr;

            Hooks::ScaleformHooks::UninstallHooks();
            Hooks::UiHooks::UninstallHooks();
        }

        void ImeApp::D3DPresent(std::uint32_t ptr)
        {
            auto &app = GetInstance();
            app.D3DPresentHook->Original(ptr);
            if (!app.m_fInitialized.load())
            {
                return;
            }
            if (!ImeManagerComposer::GetInstance()->IsSupportOtherMod())
            {
                DoD3DPresent();
            }
        }

        void ImeApp::DoD3DPresent()
        {
            auto &app = GetInstance();
            app.m_imeWnd.RenderIme();
        }

        // we need set our keyboard to non-exclusive after game default.
        void ImeApp::DispatchEvent(RE::BSTEventSource<RE::InputEvent *> *a_dispatcher, RE::InputEvent **a_events)
        {
            auto &app = GetInstance();
            Core::EventHandler::UpdateMessageFilter(a_events);
            app.DispatchInputEventHook->Original(a_dispatcher, a_events);
            Core::EventHandler::PostHandleKeyboardEvent();
        }

        auto ImeApp::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            auto &app = GetInstance();
            switch (uMsg)
            {
                /*case WM_ACTIVATE:
                    log_info("WM_ACTIVATE {:#x}, {:#x}", wParam, lParam);
                    if (LOWORD(wParam) != WA_INACTIVE)
                    {
                        ImeManager::GetInstance()->TryFocusIme();
                    }
                    break;
                case WM_ACTIVATEAPP:
                    log_info("WM_ACTIVATEAPP {:#x}, {:#x}", wParam, lParam);
                    break;*/
                case WM_NCACTIVATE:
                    log_debug("WM_NCACTIVATE {:#x}, {:#x}", wParam, lParam);
                    if (wParam == TRUE)
                    {
                        ImeManagerComposer::GetInstance()->TryFocusIme();
                    }
                    break;
                case CM_IME_ENABLE:
                    return ImeManagerComposer::GetInstance()->EnableIme(wParam != FALSE) ? S_OK : S_FALSE;
                case CM_MOD_ENABLE:
                    return ImeManagerComposer::GetInstance()->EnableMod(wParam != FALSE) ? S_OK : S_FALSE;
                case WM_SETFOCUS:
                    log_info("WM_SETFOCUS {:#x}, {:#x}", wParam, lParam);
                    ImeManagerComposer::GetInstance()->TryFocusIme();
                    return S_OK;
                case WM_IME_SETCONTEXT:
                    return ::DefWindowProc(hWnd, uMsg, wParam, 0);
                case WM_NCDESTROY: {
                    app.Uninitialize();
                    break;
                }
                default:
                    break;
            }
            return RealWndProc(hWnd, uMsg, wParam, lParam);
        }
    } // namespace Ime
} // namespace LIBC_NAMESPACE_DECL
