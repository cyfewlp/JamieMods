#include "App.h"
#include "PapyrusFunctions.h"
#include "SosGuiMenu.h"
#include "common/common.h"
#include "common/log.h"
#include "util/ImThemeLoader.h"

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

        ImThemeLoader::Loader::loadThemes();
        App::GetInstance().Init();
        return true;
    }

    auto App::Init() -> void
    {
        log_info("Initializing....");

        log_info("Install D3DInitHook....");
        D3DInitHook = std::make_unique<Hooks::D3DInitHookData>(D3DInit);
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
        m_fInitialized.store(true);
        SosGuiMenu::RegisterMenu();
    }
}