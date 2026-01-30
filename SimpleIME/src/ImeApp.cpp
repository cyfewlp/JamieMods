
#include "ImeApp.h"

#include "ImeWnd.hpp"
#include "common/common.h"
#include "common/hook.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/imgui/Material3.h"
#include "common/log.h"
#include "common/utils.h"
#include "configs/ConfigSerializer.h"
#include "configs/CustomMessage.h"
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
#include <queue>
#include <thread>

namespace LIBC_NAMESPACE_DECL
{

static constexpr auto               CONFIG_FILE_NAME     = "SimpleIME.toml";
static constexpr auto               INIT_TIMEOUT_SECONDS = 5s;
static std::unique_ptr<Ime::ImeApp> g_instance;
static ImGuiEx::M3::M3Styles        g_M3Styles;

bool PluginInit()
{
    const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
    const auto  version = plugin->GetVersion();

    static Ime::Settings g_settings;

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
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
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
    if (!m_state.IsUnInitialized())
    {
        LogAlreadyInitialized();
        return;
    }
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
    if (m_state.IsInitialized())
    {
        ImmAssociateContext(m_hWnd, m_hIMCDefault);
        m_hIMCDefault = nullptr;
        Hooks::WinHooks::Uninstall();
        D3DInitHook.reset();
        D3DInitHook = nullptr;
        UninstallHooks();
        if (RealWndProc)
        {
            SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(RealWndProc));
            RealWndProc = nullptr;
        }
    }
    const auto filePath = CommonUtils::GetInterfaceFile(CONFIG_FILE_NAME);
    ConfigSerializer::Serialize(filePath, m_settings);
    m_state.SetState(State::StateKey::DORMANCY);
    spdlog::shutdown();
}

void ImeApp::OnInputLoaded()
{
    Core::EventHandler::InstallEventSink(&m_imeWnd, m_settings.shortcutKey);
}

class InitErrorMessageShow final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    std::queue<std::string> m_message;

public:
    RE::BSEventNotifyControl ProcessEvent(
        const RE::MenuOpenCloseEvent *a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent> *
    ) override
    {
        if (a_event->menuName == RE::MainMenu::MENU_NAME && a_event->opening)
        {
            while (!m_message.empty())
            {
                RE::DebugMessageBox(m_message.front().c_str());
                m_message.pop();
            }
            RE::UI::GetSingleton()->RemoveEventSink(this);
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    void PushMessage(std::string &&message)
    {
        m_message.emplace(std::move(message));
    }
};

std::unique_ptr<InitErrorMessageShow> g_pInitErrorMessageShow(nullptr);

void ImeApp::D3DInit()
{
    auto &app = GetInstance();
    if (!app.m_state.IsUnInitialized())
    {
        app.LogAlreadyInitialized();
        return;
    }
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        g_pInitErrorMessageShow = std::make_unique<InitErrorMessageShow>();
        ui->AddEventSink(g_pInitErrorMessageShow.get());
    }
    try
    {
        app.DoD3DInit();
        RE::UI::GetSingleton()->RemoveEventSink(g_pInitErrorMessageShow.get());
        g_pInitErrorMessageShow.reset();
        return;
    }
    catch (std::exception &error)
    {
        app.m_state.SetState(State::StateKey::INITIALIZE_FAILED);
        auto message = std::format("SimpleIME initialize fail: \n {}", error.what());
        log_error(message.c_str());
        g_pInitErrorMessageShow->PushMessage(std::move(message));
    }
    catch (...)
    {
        app.m_state.SetState(State::StateKey::INITIALIZE_FAILED);
        auto message = std::string("SimpleIME: Unknown fatal error during D3DInit.");
        log_error(message.c_str());
        g_pInitErrorMessageShow->PushMessage(std::move(message));
    }
    LogStacktrace();
    log_info("Force close ImeWnd...");

    if (app.m_imeWnd.SendNotifyMessageToIme(WM_QUIT, -1, 0) == FALSE)
    {
        log_error("Send WM_QUIT to ImeWnd failed.");
    }
    app.Uninitialize();
}

void ImeApp::DoD3DInit()
{
    m_state.SetState(State::StateKey::INITIALIZING);
    D3DInitHook->Original();
    OnD3DInit();
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
    m_state.SetState(State::StateKey::INITIALIZED);
}

void ImeApp::Start(const RE::BSGraphics::RendererData &renderData)
{
    std::promise<bool> ensureInitialized;
    std::future<bool>  initialized = ensureInitialized.get_future();
    // run ImeWnd in a standalone thread
    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);

    ImGuiManager::Initialize(m_hWnd, device, context, m_settings);
    ImGuiManager::AddPrimaryFont(m_settings.resources.fontPathList);
    const auto iconFile = CommonUtils::GetInterfaceFile(Settings::ICON_FILE);
    g_M3Styles.iconFont = ImGuiManager::AddIconFont(iconFile);

    std::thread childWndThread([&ensureInitialized, this] {
        try
        {
            m_imeWnd.Initialize(g_M3Styles);
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

    if (initialized.wait_for(INIT_TIMEOUT_SECONDS) == std::future_status::timeout)
    {
        log_error("IME Window initialization timed out!");

        if (childWndThread.joinable())
        {
            if (m_imeWnd.SendNotifyMessageToIme(WM_CLOSE, 0, 0))
            {
                const auto future = std::async(std::launch::deferred, [&childWndThread] {
                    childWndThread.join();
                });
                if (future.wait_for(1s) == std::future_status::timeout)
                {
                    log_warn("IME thread did not respond to WM_CLOSE, detaching...");
                }
            }
        }
        throw SimpleIMEException("IME Thread initialization timeout.");
    }
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

void ImeApp::LogAlreadyInitialized() const
{
    log_warn("Already Initialized! Current state: {}", m_state.GetStateKetText());
}

void ImeApp::Draw() const
{
    if (!m_state.IsInitialized())
    {
        return;
    }
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
        case CM_EXECUTE_TASK: {
            TaskQueue::GetInstance().ExecuteMainThreadTasks();
            return 0;
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
            return 0;
        case WM_IME_SETCONTEXT:
            return ::DefWindowProc(hWnd, uMsg, wParam, 0);
        case WM_NCDESTROY: {
            app.Uninitialize();
            return 0;
        }
        default:
            break;
    }
    return RealWndProc(hWnd, uMsg, wParam, lParam);
}
} // namespace Ime
} // namespace LIBC_NAMESPACE_DECL
