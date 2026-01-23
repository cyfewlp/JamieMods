
#include "ImeApp.h"

#include "ImeWnd.hpp"
#include "common/common.h"
#include "common/hook.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/imgui/Material3.h"
#include "common/log.h"
#include "configs/ConfigSerializer.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "core/EventHandler.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"
#include "hooks/WinHooks.h"
#include "ime/ImeController.h"
#include "menu/ImeMenu.h"
#include "menu/ToolWindowMenu.h"
#include "ui/ImGuiManager.h"

#include <basetsd.h>
#include <future>
#include <memory>
#include <thread>

namespace LIBC_NAMESPACE_DECL
{

static constexpr auto               CONFIG_FILE_NAME = "SimpleIME.toml";
static std::unique_ptr<Ime::ImeApp> g_instance;
static ImGuiEx::M3::M3Styles        g_darkStyles;

bool PluginInit()
{
    const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
    const auto  version = plugin->GetVersion();

    static Ime::Settings g_settings;

    //
    {
        // FIXME: should read styles from file(.css/.json)
        g_darkStyles.colors = ImGuiEx::M3::Colors{
            .primary                    = ImColor(211, 188, 253),
            .surface_tint               = ImColor(211, 188, 253),
            .on_primary                 = ImColor(56, 38, 92),
            .primary_container          = ImColor(79, 61, 116),
            .on_primary_container       = ImColor(235, 221, 255),
            .secondary                  = ImColor(205, 194, 219),
            .on_secondary               = ImColor(52, 45, 64),
            .secondary_container        = ImColor(75, 67, 88),
            .on_secondary_container     = ImColor(233, 222, 248),
            .tertiary                   = ImColor(240, 183, 197),
            .on_tertiary                = ImColor(74, 37, 48),
            .tertiary_container         = ImColor(100, 59, 70),
            .on_tertiary_container      = ImColor(255, 217, 225),
            .error                      = ImColor(255, 180, 171),
            .on_error                   = ImColor(105, 0, 5),
            .error_container            = ImColor(147, 0, 10),
            .on_error_container         = ImColor(255, 218, 214),
            .background                 = ImColor(21, 18, 24),
            .on_background              = ImColor(231, 224, 232),
            .surface                    = ImColor(21, 18, 24),
            .on_surface                 = ImColor(231, 224, 232),
            .surface_variant            = ImColor(73, 69, 78),
            .on_surface_variant         = ImColor(203, 196, 207),
            .outline                    = ImColor(148, 143, 153),
            .outline_variant            = ImColor(73, 69, 78),
            .shadow                     = ImColor(0, 0, 0),
            .scrim                      = ImColor(0, 0, 0),
            .inverse_surface            = ImColor(231, 224, 232),
            .inverse_on_surface         = ImColor(50, 47, 53),
            .inverse_primary            = ImColor(104, 84, 142),
            .primary_fixed              = ImColor(235, 221, 255),
            .on_primary_fixed           = ImColor(35, 15, 70),
            .primary_fixed_dim          = ImColor(211, 188, 253),
            .on_primary_fixed_variant   = ImColor(79, 61, 116),
            .secondary_fixed            = ImColor(233, 222, 248),
            .on_secondary_fixed         = ImColor(31, 24, 43),
            .secondary_fixed_dim        = ImColor(205, 194, 219),
            .on_secondary_fixed_variant = ImColor(75, 67, 88),
            .tertiary_fixed             = ImColor(255, 217, 225),
            .on_tertiary_fixed          = ImColor(49, 16, 27),
            .tertiary_fixed_dim         = ImColor(240, 183, 197),
            .on_tertiary_fixed_variant  = ImColor(100, 59, 70),
            .surface_dim                = ImColor(21, 18, 24),
            .surface_bright             = ImColor(59, 56, 62),
            .surface_container_lowest   = ImColor(15, 13, 19),
            .surface_container_low      = ImColor(29, 27, 32),
            .surface_container          = ImColor(33, 31, 36),
            .surface_container_high     = ImColor(44, 41, 47),
            .surface_container_highest  = ImColor(54, 52, 58),
            // .primary_container_hovered  = ImColor(54, 52, 58),
            // .primary_container_pressed  = ImColor(54, 52, 58),
        };
        g_darkStyles.colors.calculateExtensionFields();
    }
    //

    const auto filePath = CommonUtils::GetInterfaceFile(CONFIG_FILE_NAME);
    Ime::ConfigSerializer::Deserialize(filePath, g_settings);
    InitializeLogging(g_settings.logging.level, g_settings.logging.flushLevel);
    g_instance = std::make_unique<Ime::ImeApp>(g_settings);

    log_info("{} {} is loading...", plugin->GetName(), version.string());

    g_instance->Initialize();

    log_info("{} has finished loading.", plugin->GetName());
    InitializeMessaging();
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
auto ImeApp::GetInstance() -> ImeApp &
{
    return *g_instance;
}

/**
 * Init ImeApp
 */
void ImeApp::Initialize()
{
    m_fInitialized.store(false);
    Hooks::WinHooks::Install();

    D3DInitHook         = std::make_unique<Hooks::D3DInitHookData>(D3DInit);
    auto &errorNotifier = ErrorNotifier::GetInstance();
    errorNotifier.SetMessageDuration(m_settings.appearance.errorDisplayDuration);
#ifdef SIMPLE_IME_DEBUG
    errorNotifier.SetMessageLevel(ErrorMsg::Level::debug);
#endif
}

void ImeApp::Uninitialize()
{
    ImGuiManager::Shutdown();
    if (m_fInitialized)
    {
        Hooks::WinHooks::Uninstall();
        D3DInitHook.reset();
        D3DInitHook = nullptr;
        UninstallHooks();
    }
    const auto filePath = CommonUtils::GetInterfaceFile(CONFIG_FILE_NAME);
    ConfigSerializer::Serialize(filePath, m_settings);
    m_fInitialized.store(false);
}

void ImeApp::OnInputLoaded()
{
    Core::EventHandler::InstallEventSink(&m_imeWnd, m_settings.shortcutKey);
}

class InitErrorMessageShow final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
    RE::BSEventNotifyControl
    ProcessEvent(const RE::MenuOpenCloseEvent *a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent> *) override
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
    auto *renderManager = RE::BSGraphics::Renderer::GetSingleton();
    if (renderManager == nullptr)
    {
        throw SimpleIMEException("Cannot find render manager. Initialization failed!");
    }

    const auto &renderData = renderManager->GetRuntimeData();
    log_debug("Getting SwapChain...");
    auto *pSwapChain = renderData.renderWindows->swapChain;
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

    m_hWnd        = reinterpret_cast<HWND>(swapChainDesc.outputWindow);
    m_hIMCDefault = ImmAssociateContext(m_hWnd, nullptr);

    Start(renderData);
    m_fInitialized.store(true);

    log_debug("Hooking Skyrim WndProc...");
    RealWndProc =
        reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MainWndProc)));
    if (RealWndProc == nullptr)
    {
        throw SimpleIMEException("Hook WndProc failed!");
    }
    InstallHooks();

    ImeMenu::RegisterMenu();
    ToolWindowMenu::RegisterMenu();
}

void ImeApp::Start(const RE::BSGraphics::RendererData &renderData)
{
    std::promise<bool> ensureInitialized;
    std::future<bool>  initialized = ensureInitialized.get_future();
    // run ImeWnd in a standalone thread
    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);

    ImGuiManager::Initialize(m_hWnd, device, context, m_settings);

    std::thread childWndThread([&ensureInitialized, this] {
        try
        {
            m_imeWnd.Initialize(g_darkStyles);
            ensureInitialized.set_value(true);
            m_imeWnd.Start(m_hWnd, &m_settings);
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
}

void ImeApp::InstallHooks()
{
    // D3DPresentHook         = std::make_unique<Hooks::D3DPresentHookData>(D3DPresent);
    // DispatchInputEventHook = std::make_unique<Hooks::DispatchInputEventHookData>(DispatchEvent);

    Hooks::ScaleformHooks::Install();
}

void ImeApp::UninstallHooks()
{
    // D3DPresentHook         = nullptr;
    // DispatchInputEventHook = nullptr;

    Hooks::ScaleformHooks::Uninstall();
}

void ImeApp::Draw() const
{
    ImGuiManager::NewFrame();

    m_imeWnd.DrawIme(m_settings);

    ImGuiManager::EndFrame(m_settings);
    ImGuiManager::Render();
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
        case CM_EXECUTE_TASK: {
            TaskQueue::GetInstance().ExecuteMainThreadTasks();
            return S_OK;
        }
        case WM_NCACTIVATE:
            log_debug("WM_NCACTIVATE {:#x}, {:#x}", wParam, lParam);
            if (wParam == TRUE)
            {
                ImeController::GetInstance()->TryFocusIme();
            }
            break;
        case WM_SETFOCUS:
            log_info("WM_SETFOCUS {:#x}, {:#x}", wParam, lParam);
            ImeController::GetInstance()->TryFocusIme();
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
