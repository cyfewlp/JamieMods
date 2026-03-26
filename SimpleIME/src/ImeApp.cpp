#include "ImeApp.h"

#include "ImeWnd.hpp"
#include "WCharUtils.h"
#include "common.h"
#include "configs/ConfigSerializer.h"
#include "configs/CustomMessage.h"
#include "configs/configuration.h"
#include "configs/settings_converter.h"
#include "core/EventHandler.h"
#include "core/State.h"
#include "fonts/FontManager.h"
#include "hook.h"
#include "hooks/ScaleformHook.h"
#include "hooks/WinHooks.h"
#include "ime/ImeController.h"
#include "imguiex/ErrorNotifier.h"
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
class InitErrorMessageShow final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    std::queue<std::string> m_message;
    bool                    m_installedSink = false;

public:
    InitErrorMessageShow()
    {
        if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
        {
            ui->AddEventSink(this);
            m_installedSink = true;
        }
    }

    ~InitErrorMessageShow()
    {
        if (auto *ui = RE::UI::GetSingleton(); m_installedSink && ui != nullptr)
        {
            ui->RemoveEventSink(this);
        }
    }

    auto ProcessEvent(const RE::MenuOpenCloseEvent *a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent> *) -> RE::BSEventNotifyControl override
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
        if (m_installedSink)
        {
            m_message.emplace(std::move(message));
        }
    }
};

#ifdef _DEBUG
constexpr auto INIT_TIMEOUT_SECONDS = 500s;
#else
constexpr auto INIT_TIMEOUT_SECONDS = 5s;
#endif
constexpr auto                          CONFIG_FILE_NAME = "SimpleIME.toml";
std::unique_ptr<Ime::ImeApp>            g_instance       = nullptr;
std::unique_ptr<InitErrorMessageShow>   g_pInitErrorMessageShow(nullptr);
std::unique_ptr<Hooks::D3DInitHookData> g_D3DInitHook = nullptr; ///< Only install once, should not be a member of `ImeApp`.

auto ConfigFilePath() -> std::filesystem::path
{
    return utils::GetInterfacePath() / SIMPLE_IME / CONFIG_FILE_NAME;
}
} // namespace

namespace SksePlugin
{
auto Initialize() -> bool
{
    g_instance    = std::make_unique<Ime::ImeApp>(ConfigFilePath());
    g_D3DInitHook = std::make_unique<Hooks::D3DInitHookData>(Ime::D3DInit);
    Hooks::WinHooks::Install();

    auto &errorNotifier = ErrorNotifier::GetInstance();
    errorNotifier.SetMessageDuration(g_instance->GetSettings().appearance.errorDisplayDuration);
#ifdef _DEBUG
    errorNotifier.SetMessageLevel(ErrorMsg::Level::debug);
#endif

    const auto *plugin  = SKSE::PluginDeclaration::GetSingleton();
    const auto  version = plugin->GetVersion();
    logger::info("{}({}) has finished loading.", plugin->GetName(), version.string("."));

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
} // namespace SksePlugin

namespace Ime
{
namespace
{
auto GetDefaultFontFilePathList() -> std::vector<std::string>
{
    std::vector<std::string> fonts{};

    const auto primaryFontFilePath = Fonts::GetDefaultFontFilePath();
    if (primaryFontFilePath.empty())
    {
        ErrorNotifier::GetInstance().Warning("Can't get default font. Fallback to ImGui embedded font.");
        return std::vector<std::string>{};
    }
    fonts.push_back(WCharUtils::ToString(primaryFontFilePath));
    logger::info("Primary font: {}", fonts.back());

    std::wstring emojiFontFilePath = Fonts::GetFirstFontFilePathInFamily(Settings::DEFAULT_EMOJI_FONT_FAMILY);
    if (emojiFontFilePath.empty())
    {
        emojiFontFilePath = Fonts::GetFirstFontFilePathInFamily(Settings::DEFAULT_SYMBOL_FONT_FAMILY);
    }
    if (!emojiFontFilePath.empty())
    {
        fonts.push_back(WCharUtils::ToString(emojiFontFilePath));
        logger::info("Supplementary font: {}", fonts.back());
    }
    return fonts;
}
} // namespace

ImeApp::ImeApp(std::filesystem::path configPath)
{
    const auto configuration = Ime::ConfigSerializer::LoadConfiguration(configPath);
    m_settings               = Ime::ConvertConfigurationToSettings(configuration);

    SksePlugin::InitializeLogging({m_settings.logging.level, m_settings.logging.flushLevel});
}

auto ImeApp::GetInstance() -> ImeApp &
{
    return *g_instance;
}

void ImeApp::OnInputLoaded()
{
    if (m_state.IsInitialized())
    {
        Events::InstallEventSinks();
    }
}

void ImeApp::Uninitialize()
{
    SaveSettings();
    UI::DestroyM3();
    UI::Shutdown();
    Events::UnInstallEventSinks(); // should safety
    Hooks::WinHooks::Uninstall();  // should move to dll_detach, but it's safe.
    g_pInitErrorMessageShow.reset();
    if (m_state.IsInitialized())
    {
        UninstallHooks();
        if (RealWndProc)
        {
            SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(RealWndProc));
            RealWndProc = nullptr;
        }
    }
    m_state.SetState(State::StateKey::DORMANCY);
}

void D3DInit()
{
    auto &app = ImeApp::GetInstance();
    if (!app.m_state.IsUnInitialized())
    {
        logger::warn("Already Initialized! Current state: {}", app.m_state.GetStateKetText());
        return;
    }
    g_pInitErrorMessageShow = std::make_unique<InitErrorMessageShow>();
    try
    {
        app.DoD3DInit();
        g_pInitErrorMessageShow.reset();
        return;
    }
    catch (std::exception &error)
    {
        app.m_state.SetState(ImeApp::State::StateKey::INITIALIZE_FAILED);
        auto message = std::format("SimpleIME initialize fail: \n {}", error.what());
        logger::error(message.c_str());
        g_pInitErrorMessageShow->PushMessage(std::move(message));
    }
    catch (...)
    {
        app.m_state.SetState(ImeApp::State::StateKey::INITIALIZE_FAILED);
        auto message = std::string("SimpleIME: Unknown fatal error during D3DInit.");
        logger::error(message.c_str());
        g_pInitErrorMessageShow->PushMessage(std::move(message));
    }

    app.Shutdown();
}

void ImeApp::DoD3DInit()
{
    m_state.SetState(State::StateKey::INITIALIZING);
    g_D3DInitHook->Original();
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

    m_hWnd = reinterpret_cast<HWND>(swapChainDesc.outputWindow);

    Start(renderData);

    logger::debug("Hooking Skyrim WndProc...");
    RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MainWndProc)));
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
    auto              *device      = reinterpret_cast<ID3D11Device *>(renderData.forwarder);
    auto              *context     = reinterpret_cast<ID3D11DeviceContext *>(renderData.context);

    UI::Initialize(m_hWnd, device, context);
    const auto defaultFontFilePathList = GetDefaultFontFilePathList();
    (void)UI::AddPrimaryFont(m_settings.resources.fontPathList, defaultFontFilePathList);
    UI::InitializeM3(utils::GetInterfacePath() / SIMPLE_IME / Settings::ICON_FILE, m_settings.appearance.schemeConfig);

    std::thread childWndThread([&ensureInitialized, this] -> void {
        SetThreadDescription(GetCurrentThread(), L"SimpleIME Message Thread");
        try
        {
            m_imeWnd.Initialize(m_settings.enableTsf);
            ensureInitialized.set_value(true);
            // we can't call ensureInitialized after create child window, will cause deadlock.
            m_imeWnd.CreateHost(m_hWnd, m_settings);
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

void ImeApp::SaveSettings()
{
    const auto config = Ime::ConvertSettingsToConfiguration(m_settings);
    Ime::ConfigSerializer::SaveConfiguration(ConfigFilePath(), config);
}

void ImeApp::InstallHooks()
{
    Hooks::Scaleform::Install();
}

void ImeApp::UninstallHooks()
{
    Hooks::Scaleform::Uninstall();
}

void ImeApp::Draw()
{
    if (!m_state.IsInitialized())
    {
        return;
    }
    UI::NewFrame();

    m_imeWnd.Draw(m_settings);

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
                ImeController::GetInstance()->SyncImeState();
            }
            break;
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
