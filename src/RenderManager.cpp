// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "RenderManager.h"

#include <d3d11.h>
#include <tchar.h>

#include "Hooks.h"
#include "Transmogrify.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

LRESULT WINAPI
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT
Hooks::WndProcHook::thunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return func(hWnd, uMsg, wParam, lParam);
}

void ConfigImGui(HWND);
void
Hooks::D3DInitHook::thunk()
{
    func();

    LOG(info, "D3DInit Hooked!");
    auto render_manager = RE::BSGraphics::Renderer::GetSingleton();
    if (!render_manager) {
        LOG(err, "Cannot find render manager. Initialization failed!");
        return;
    }

    auto render_data = render_manager->data;

    LOG(info, "Getting SwapChain...");
    auto g_pSwapChain = render_data.renderWindows->swapChain;
    if (!g_pSwapChain) {
        LOG(err, "Cannot find SwapChain. Initialization failed!");
        return;
    }

    LOG(info, "Getting SwapChain desc...");
    DXGI_SWAP_CHAIN_DESC sd{};
    if (g_pSwapChain->GetDesc(std::addressof(sd)) < 0) {
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

    WndProcHook::func = reinterpret_cast<WNDPROC>(
      SetWindowLongPtrA(sd.OutputWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook::thunk)));
    if (!WndProcHook::func)
        LOG(err, "SetWindowLongPtrA failed!");
}

void
ConfigImGui(HWND hwnd)
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
render(bool* show_demo_window, bool* show_another_window);
void
Hooks::D3DPresentHook::thunk(std::uint32_t a_p1)
{
    func(a_p1);
    if (!Hooks::D3DInitHook::initialized.load() || !RenderManager::showWindow) {
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
render(bool* show_demo_window, bool* show_another_window)
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
    // to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                    // Create a window called "Hello, world!" and append into it.
        ImGui::Text("This is some useful text.");         // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", show_another_window);

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button(
              "Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        if (ImGui::Button("Access player inventory")) {
            Transmogrify::AccessPlayerInventory();
        }
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

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
RenderManager::cleanup()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}