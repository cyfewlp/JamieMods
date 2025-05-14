#include "App.h"

#include "PapyrusFunctions.h"
#include "SosGuiMenu.h"
#include "common/common.h"
#include "common/imgui/ImThemeLoader.h"
#include "common/log.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include <exception>
#include <memory>
#include <string>

namespace LIBC_NAMESPACE_DECL
{
auto PluginInit() -> bool
{
    using Message = SKSE::MessagingInterface::Message;
    SKSE::GetMessagingInterface()->RegisterListener([](Message *message) {
        if (message->type == SKSE::MessagingInterface::kPostLoadGame)
        {
            log_debug("Send Event with Init");
        }
    });
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions::Register);

    auto themeFilePath = util::GetInterfaceFile(ImTheme::THEME_FILE_NAME);
    ImTheme::Loader::GetInstance().LoadThemes(themeFilePath);
    App::GetInstance().Init();
    return true;
}

auto App::Init() -> void
{
    log_info("Initializing....");

    log_info("Install D3DInitHook....");
    D3DInitHook = std::make_unique<Hooks::D3DInitHookData>(D3DInit);
}

auto App::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    switch (msg)
    {
        case WM_DESTROY: {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
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
        log_error(message.c_str());
    }
    catch (...)
    {
        const auto message = std::string("SimpleIME initialize fail: \n occur unexpected error.");
        log_error(message.c_str());
    }
    LogStacktrace();
}

void App::DoD3DInit()
{
    D3DInitHook->Original();

    const auto *renderer = RE::BSGraphics::Renderer::GetSingleton();
    if (renderer == nullptr)
    {
        throw InitFail("Cannot find render manager.");
    }

    const auto &renderData = renderer->data;
    log_debug("Getting SwapChain...");
    auto *pSwapChain = renderData.renderWindows[0].swapChain;
    if (pSwapChain == nullptr)
    {
        throw InitFail("Cannot find SwapChain.");
    }
    log_debug("Getting SwapChain desc...");
    REX::W32::DXGI_SWAP_CHAIN_DESC swapChainDesc;
    if (pSwapChain->GetDesc(&swapChainDesc) < 0)
    {
        throw InitFail("IDXGISwapChain::GetDesc failed.");
    }

    m_hWnd = reinterpret_cast<HWND>(swapChainDesc.outputWindow);
    if (!SosGui::Init(renderData, m_hWnd))
    {
        throw InitFail("Can't initialize SosGui.");
    }
    RealWndProc =
        reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MainWndProc)));
    if (RealWndProc == nullptr)
    {
        throw InitFail("Hook WndProc failed!");
    }
    m_fInitialized.store(true);
    SosGuiMenu::RegisterMenu();
}
}