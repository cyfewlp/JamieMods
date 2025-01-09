// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "RenderManager.h"

#include <tchar.h>

#include "Hooks.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <unordered_set>

LRESULT
RenderManager::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return RealWndProc(hWnd, uMsg, wParam, lParam);
}

void
RenderManager::Init()
{
    RealD3dInitFunc = Hooks::D3DInitHook::Install(&D3DInit);
    RealD3dPresentFunc = Hooks::D3DPresentHook::Install(&D3DPresent);

    SKSE::GetTaskInterface()->AddTask([] {
        Transmogrify::GetAllArmors(&uiState.modToArmors);
        for (auto& pair : uiState.modToArmors) {
            uiState.mods.push_back(pair.first);
        }
    });
}

void
RenderManager::D3DInit()
{
    RealD3dInitFunc();

    LOG(info, "D3DInit Hooked!");
    auto render_manager = RE::BSGraphics::Renderer::GetSingleton();
    if (!render_manager) {
        LOG(err, "Cannot find render manager. Initialization failed!");
        return;
    }

    auto render_data = render_manager->data;

    LOG(info, "Getting SwapChain...");
    auto pSwapChain = render_data.renderWindows->swapChain;
    if (!pSwapChain) {
        LOG(err, "Cannot find SwapChain. Initialization failed!");
        return;
    }

    LOG(info, "Getting SwapChain desc...");
    DXGI_SWAP_CHAIN_DESC sd{};
    if (pSwapChain->GetDesc(std::addressof(sd)) < 0) {
        LOG(err, "IDXGISwapChain::GetDesc failed.");
        return;
    }

    auto device = render_data.forwarder;
    auto context = render_data.context;

    LOG(info, "Initializing ImGui...");
    ImGui::CreateContext();
    if (!ImGui_ImplWin32_Init(sd.OutputWindow)) {
        LOG(err, "ImGui initialization failed (Win32)");
        return;
    }
    if (!ImGui_ImplDX11_Init(device, context)) {
        LOG(err, "ImGui initialization failed (DX11)");
        return;
    }

    ConfigImGui(sd.OutputWindow);
    initialized.store(true);

    RealWndProc = reinterpret_cast<WNDPROC>(
      SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(RenderManager::WndProc)));
    if (!RealWndProc) {
        LOG(err, "SetWindowLongPtrA failed!");
    }
}

void
RenderManager::ConfigImGui(HWND hwnd)
{
    RECT rect = { 0, 0, 0, 0 };
    LOG(info, "rect right: {}, left: {}, top: {}, bottom: {}", rect.right, rect.left, rect.top, rect.bottom);
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    GetClientRect(hwnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
    // ImFontConfig config;
    // config.SizePixels = 20.0f;
    // io.Fonts->AddFontDefault(&config);
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 20.0f);

    io.MouseDrawCursor = true;
    io.ConfigNavMoveSetMousePos = true;

    LOG(info, "ImGui initialized!");
}

void
RenderManager::D3DPresent(std::uint32_t a_p1)
{
    RealD3dPresentFunc(a_p1);
    if (!initialized.load() || !RenderManager::showWindow) {
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    bool show_demo_window = true;
    bool show_another_window = false;
    render(&show_demo_window, &show_another_window);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void
RenderManager::render(bool* show_demo_window, bool* show_another_window)
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
    // to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;

        ImGui::Begin("Hello, world!");                    // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text.");         // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", show_another_window);
        ImGui::Checkbox("Show Mods", &RenderManager::uiState.showArmors);

        if (RenderManager::uiState.showArmors) {
            RenderArmors();
        }

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Access player inventory")) {
            Transmogrify::AccessPlayerInventory();
        }
        ImGui::SameLine();

        auto& io = ImGui::GetIO();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (*show_another_window) {
        ImGui::Begin("Another Window",
                     show_another_window); // Pass a pointer to our bool variable (the window will have a closing
                                           // button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            *show_another_window = false;
        ImGui::End();
    }
}

void
RenderManager::RenderArmors()
{
    auto& uiState = RenderManager::uiState;
    if (ImGui::BeginCombo("Mods", uiState.mods[uiState.selectedMod].c_str())) {
        for (int n = 0; n < uiState.mods.size(); n++) {
            const bool is_selected = (uiState.selectedMod == n);
            if (ImGui::Selectable(uiState.mods[n].c_str(), is_selected))
                uiState.selectedMod = n;

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void
RenderManager::SwitchShowWindow()
{
    showWindow = !showWindow;
}

void
RenderManager::cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}