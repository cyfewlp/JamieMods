#include "App.h"

#include "PapyrusFunctions.h"
#include "SosGuiMenu.h"
#include "common.h"
#include "gui/UiSettings.h"
#include "imgui/ImThemeLoader.h"
#include "log.h"
#include "util/UiSettingsLoader.h"

#include <exception>
#include <memory>
#include <string>

namespace SksePlugin
{
auto Initialize() -> bool
{
    using Message = SKSE::MessagingInterface::Message;
    SKSE::GetMessagingInterface()->RegisterListener([](Message *message) {
        if (message->type == SKSE::MessagingInterface::kPostLoadGame)
        {
            logger::debug("Send Event with Init");
        }
    });
    SKSE::GetPapyrusInterface()->Register(SosGui::PapyrusFunctions::Register);

    const auto themeFilePath = SosGui::util::GetInterfaceFile(ImTheme::THEME_FILE_NAME);
    ImTheme::Loader::GetInstance().LoadThemes(themeFilePath);
    SosGui::App::GetInstance().Init();
    return true;
}
} // namespace SksePlugin

namespace SosGui
{

auto App::Init() -> void
{
    logger::info("Initializing....");

    logger::info("Install D3DInitHook....");
    D3DInitHook = std::make_unique<Hooks::D3DInitHookData>(D3DInit);
}

auto App::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (msg)
    {
        case WM_DESTROY: {
            SosGuiMenu::ShutDown();
            break;
        }
        default:
            break;
    }
    return RealWndProc(hWnd, msg, wParam, lParam);
}

void App::D3DInit()
{
    try
    {
        GetInstance().DoD3DInit();
        return;
    }
    catch (std::exception &error)
    {
        const auto message = std::format("SimpleIME initialize fail: \n {}", error.what());
        logger::error(message.c_str());
    }
    catch (...)
    {
        const auto message = std::string("SimpleIME initialize fail: \n occur unexpected error.");
        logger::error(message.c_str());
    }
    logger::LogStacktrace();
}

void App::DoD3DInit()
{
    D3DInitHook->Original();

    const auto *renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (renderer == nullptr)
    {
        throw InitFail("Cannot find render manager.");
    }

    const auto &renderData = renderer->GetRuntimeData();
    logger::debug("Getting SwapChain...");
    auto *pSwapChain = renderData.renderWindows[0].swapChain;
    if (pSwapChain == nullptr)
    {
        throw InitFail("Cannot find SwapChain.");
    }
    logger::debug("Getting SwapChain desc...");
    REX::W32::DXGI_SWAP_CHAIN_DESC swapChainDesc;
    if (pSwapChain->GetDesc(&swapChainDesc) < 0)
    {
        throw InitFail("IDXGISwapChain::GetDesc failed.");
    }

    m_hWnd = reinterpret_cast<HWND>(swapChainDesc.outputWindow);
    SosGuiWindow::Init(m_hWnd, renderData);
    RealWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MainWndProc)));
    if (RealWndProc == nullptr)
    {
        throw InitFail("Hook WndProc failed!");
    }
    m_fInitialized.store(true);
    SosGuiMenu::RegisterMenu();
}
} // namespace SosGui
