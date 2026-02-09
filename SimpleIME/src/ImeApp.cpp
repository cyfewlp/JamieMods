#include "ImeApp.h"

#include "ImeWnd.hpp"
#include "common.h"
#include "configs/ConfigSerializer.h"
#include "configs/CustomMessage.h"
#include "core/EventHandler.h"
#include "core/State.h"
#include "hook.h"
#include "hooks/ScaleformHook.h"
#include "hooks/WinHooks.h"
#include "ime/ImeController.h"
#include "imguiex/ErrorNotifier.h"
#include "imguiex/M3ThemeBuilder.h"
#include "imguiex/Material3.h"
#include "log.h"
#include "menu/ImeMenu.h"
#include "menu/ToolWindowMenu.h"
#include "path_utils.h"
#include "ui/Settings.h"
#include "ui/imgui_system.h"

#include <basetsd.h>
#include <future>
#include <memory>
#include <queue>
#include <thread>

namespace
{
constexpr auto                         CONFIG_FILE_NAME     = "SimpleIME.toml";
constexpr auto                         INIT_TIMEOUT_SECONDS = 5s;
std::unique_ptr<Ime::ImeApp>           g_instance           = nullptr;
std::unique_ptr<ImGuiEx::M3::M3Styles> g_M3Styles           = nullptr;

auto ConfigFilePath() -> std::filesystem::path
{
    return utils::GetInterfacePath() / SIMPLE_IME / CONFIG_FILE_NAME;
}
} // namespace

namespace SksePlugin
{
static auto Initialize() -> bool
{
    const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
    const auto  version = plugin->GetVersion();

    static Ime::Settings g_settings;

    Ime::ConfigSerializer::Deserialize(ConfigFilePath(), g_settings);
    InitializeLogging(SpdLogSettings(g_settings.logging.level, g_settings.logging.flushLevel));
    g_instance = std::make_unique<Ime::ImeApp>(g_settings);

    logger::info("{} {} is loading...", plugin->GetName(), version.string());

    g_instance->Initialize();

    logger::info("{} has finished loading.", plugin->GetName());
    InitializeMessaging();
    return true;
}

static void InitializeMessaging()
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
} // namespace SksePlugin

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

void ImeApp::OnInputLoaded()
{
    if (m_state.IsInitialized())
    {
        Events::InstallEventSinks(&m_imeWnd, m_settings.shortcutKey);
    }
}

void ImeApp::Uninitialize()
{
    UI::Shutdown();
    Events::UnInstallEventSinks(); // should safe
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
    SaveSettings();
    m_state.SetState(State::StateKey::DORMANCY);
}

class InitErrorMessageShow final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    std::queue<std::string> m_message;

public:
    auto ProcessEvent(const RE::MenuOpenCloseEvent *a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent> *)
        -> RE::BSEventNotifyControl override
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

static std::unique_ptr<InitErrorMessageShow> g_pInitErrorMessageShow(nullptr);

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
        logger::error(message.c_str());
        g_pInitErrorMessageShow->PushMessage(std::move(message));
    }
    catch (...)
    {
        app.m_state.SetState(State::StateKey::INITIALIZE_FAILED);
        auto message = std::string("SimpleIME: Unknown fatal error during D3DInit.");
        logger::error(message.c_str());
        g_pInitErrorMessageShow->PushMessage(std::move(message));
    }

    app.Shutdown();
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
    logger::debug("Getting SwapChain...");
    auto *pSwapChain = renderData.renderWindows->swapChain;
    if (pSwapChain == nullptr)
    {
        throw SimpleIMEException("Cannot find SwapChain. Initialization failed!");
    }

    logger::debug("Getting SwapChain desc...");
    REX::W32::DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    if (pSwapChain->GetDesc(&swapChainDesc) < 0)
    {
        throw SimpleIMEException("IDXGISwapChain::GetDesc failed.");
    }

    m_hWnd        = reinterpret_cast<HWND>(swapChainDesc.outputWindow);
    m_hIMCDefault = ImmAssociateContext(m_hWnd, nullptr);

    Start(renderData);

    logger::debug("Hooking Skyrim WndProc...");
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
    const auto         initialized = ensureInitialized.get_future();
    // run ImeWnd in a standalone thread
    auto *device  = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto *context = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);

    UI::Initialize(m_hWnd, device, context);
    auto      *primaryFont  = UI::AddPrimaryFont(m_settings.resources.fontPathList);
    const auto iconFontPath = utils::GetInterfacePath() / SIMPLE_IME / Settings::ICON_FILE;
    // FIXME:: should move to ImeUI after refactor ImeUI
    auto *iconFont = UI::AddFont(iconFontPath.generic_string());
    if (iconFont == nullptr)
    {
        logger::error("Cannot find icon font. Initialization failed!");
        iconFont = primaryFont;
    }

    const auto &appearance = m_settings.appearance;
    auto        colors     = ImGuiEx::M3::ThemeBuilder::Build(
        {appearance.themeContrastLevel, appearance.themeSourceColor, appearance.themeDarkMode}
    );
    g_M3Styles = std::make_unique<ImGuiEx::M3::M3Styles>(colors, iconFont);

    std::thread childWndThread([&ensureInitialized, this] -> void {
        try
        {
            m_imeWnd.Initialize(m_settings.enableTsf);
            ensureInitialized.set_value(true);
            // we can't call ensureInitialized after create child window, will cause deadlock.
            m_imeWnd.CreateHost(m_hWnd, m_settings);
            m_imeWnd.ApplyUiSettings(m_settings, *g_M3Styles);
            m_imeWnd.Run();
        }
        catch (...)
        {
            try
            {
                ensureInitialized.set_exception(std::current_exception());
            }
            catch (...)
            {
                // set_exception() may throw too
            }
        }
    });

    if (initialized.wait_for(INIT_TIMEOUT_SECONDS) == std::future_status::timeout)
    {
        logger::error("IME Window initialization timed out!");

        if (childWndThread.joinable())
        {
            if (!m_imeWnd.SendNotifyMessageToIme(WM_CLOSE, 0, 0))
            {
                logger::warn("IME thread did not respond to WM_CLOSE, detaching...");
            }
            std::thread([t = std::move(childWndThread)]() mutable -> void {
                if (t.joinable())
                {
                    t.join();
                    logger::info("IME child thread successfully joined after timeout.");
                }
            }).detach();
        }
        throw SimpleIMEException("IME Thread initialization timeout.");
    }
    childWndThread.detach();
}

// FIXME: is safe?
void ImeApp::Shutdown()
{
    logger::LogStacktrace();
    m_state.SetState(State::StateKey::SHUTDOWN);
    logger::info("Force close ImeWnd...");
    if (!m_imeWnd.SendNotifyMessageToIme(WM_QUIT, 0, 0))
    {
        logger::error("Can't close ImeWnd! May IME uninitialized?");
    }
    Uninitialize();
}

void ImeApp::SaveSettings() const
{
    ImeController::GetInstance()->SaveSettings(m_settings);
    ConfigSerializer::Serialize(ConfigFilePath(), m_settings);
}

void ImeApp::InstallHooks()
{
    Hooks::Scaleform::Install();
}

void ImeApp::UninstallHooks()
{
    Hooks::Scaleform::Uninstall();
}

void ImeApp::LogAlreadyInitialized() const
{
    logger::warn("Already Initialized! Current state: {}", m_state.GetStateKetText());
}

void ImeApp::Draw()
{
    if (!m_state.IsInitialized() || !g_M3Styles)
    {
        return;
    }
    UI::NewFrame();

    m_imeWnd.DrawIme(m_settings, *g_M3Styles);

    UI::EndFrame();
    UI::Render();
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
            if (wParam == TRUE)
            {
                ImeController::GetInstance()->TryFocusIme();
            }
            break;
        case WM_SETFOCUS:
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
