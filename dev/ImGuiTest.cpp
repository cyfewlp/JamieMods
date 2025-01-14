// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <d3d11.h>
#include <format>
#include <iostream>
#include <vector>

#define LOG(a_type, ...)                                                                                               \
    spdlog::log(spdlog::source_loc(__FILE__, __LINE__, __FUNCTION__), spdlog::level::a_type, __VA_ARGS__)

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static bool candChosen = false;

// Forward declarations of helper functions
bool
CreateDeviceD3D(HWND hWnd);
void
CleanupDeviceD3D();
void
CreateRenderTarget();
void
CleanupRenderTarget();
LRESULT WINAPI
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void
Render();
void
UpdateKeyboardCodePage();

// Main code
int
main(int, char**)
{
    // Create application window
    // ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXA wc = { sizeof(wc), CS_CLASSDC, WndProc,         0L,     0L, GetModuleHandle(nullptr), nullptr, nullptr,
                       nullptr,    nullptr,    "ImGui Example", nullptr };
    ::RegisterClassExA(&wc);
    HWND hwnd = ::CreateWindowA(wc.lpszClassName,
                                "Dear ImGui DirectX11 Example",
                                WS_OVERLAPPEDWINDOW,
                                100,
                                100,
                                1280,
                                800,
                                nullptr,
                                nullptr,
                                wc.hInstance,
                                nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your
    // application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr,
    // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 20.0f);
    io.Fonts->AddFontFromFileTTF(
      "C:\\Windows\\Fonts\\simsun.ttc", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar icons_ranges[] = { 0x1, 0x1FFFF, 0 }; // Will not be copied
    // io.Fonts->AddFontFromFileTTF(config->emojiFontFile.c_str(), config->fontSize, &imFontConfig, icons_ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 16.0f, &cfg, icons_ranges);
    io.Fonts->Build();
    ImGui::GetMainViewport()->PlatformHandleRaw = (void*)hwnd;
    UpdateKeyboardCodePage();

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
    LOG(info,
        "msg: {}, wParam: {}, lParam: {}",
        fmt::format("{:#x}", msg.message).c_str(),
        fmt::format("{:#x}", msg.wParam).c_str(),
        fmt::format("{:#x}", msg.lParam).c_str());
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code
        // to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            ImGui::Text("\xf0\x9f\x8d\x89 \xf0\x9f\x8d\x8a \xf0\x9f\x8d\x8b");
            ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button(
                  "Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
        Render();

        // 3. Show another simple window.
        if (show_another_window) {
            ImGui::Begin("Another Window",
                         &show_another_window); // Pass a pointer to our bool variable (the window will have a closing
                                                // button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = {
            clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w
        };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
        // HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassA(wc.lpszClassName, wc.hInstance);

    return 0;
}

static void
Render()
{
    static char buf[64] = "";
    static int selected = 0;
    // ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() *
    // 16));
    if (ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf))) {
        ImGui::SetKeyboardFocusHere(-1);
    }
    for (int i = 0; i < 5; i++) {
        bool b = i == selected;
        if (ImGui::Selectable(fmt::format("{}. select", i).c_str(), i == selected)) {
            LOG(info, "selected {}", i);
            selected = i;
            // ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, true);
        }
    }
}

// Helper functions

bool
CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr,
                                                D3D_DRIVER_TYPE_HARDWARE,
                                                nullptr,
                                                createDeviceFlags,
                                                featureLevelArray,
                                                2,
                                                D3D11_SDK_VERSION,
                                                &sd,
                                                &g_pSwapChain,
                                                &g_pd3dDevice,
                                                &featureLevel,
                                                &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr,
                                            D3D_DRIVER_TYPE_WARP,
                                            nullptr,
                                            createDeviceFlags,
                                            featureLevelArray,
                                            2,
                                            D3D11_SDK_VERSION,
                                            &sd,
                                            &g_pSwapChain,
                                            &g_pd3dDevice,
                                            &featureLevel,
                                            &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void
CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void
CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void
CleanupRenderTarget()
{
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void
GetCandidateList(HIMC hIMC)
{
    // Get the size of the candidate list buffer
    DWORD bufferSize = ImmGetCandidateList(hIMC, 0, NULL, 0);
    if (bufferSize == 0) {
        std::cerr << "Failed to get the size of the candidate list buffer." << std::endl;
        return;
    }

    // Allocate buffer for the candidate list
    std::vector<BYTE> buffer(bufferSize);
    // LPCANDIDATELIST candList = reinterpret_cast<LPCANDIDATELIST>(buffer.data());
    LPCANDIDATELIST candList = (LPCANDIDATELIST)GlobalAlloc(GPTR, bufferSize);

    // Retrieve the candidate list
    DWORD copiedSize = ImmGetCandidateList(hIMC, 0, candList, bufferSize);
    if (copiedSize == 0) {
        std::cerr << "Failed to retrieve the candidate list." << std::endl;
        return;
    }

    // Output the candidate strings
    std::wcout << L"Candidate List:" << std::endl;
    for (DWORD i = 0; i < candList->dwCount; ++i) {
        LPWSTR candidateStr = reinterpret_cast<LPWSTR>(reinterpret_cast<BYTE*>(candList) + candList->dwOffset[i]);
        std::wcout << L"Candidate " << i + 1 << L": " << candidateStr << std::endl;
    }
    GlobalFree(candList);
}

static int keyboardPage = CP_ACP;

static void
UpdateKeyboardCodePage()
{
    // Retrieve keyboard code page, required for handling of non-Unicode Windows.
    HKL keyboard_layout = ::GetKeyboardLayout(0);
    LCID keyboard_lcid = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
    if (::GetLocaleInfoA(keyboard_lcid,
                         (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE),
                         (LPSTR)&keyboardPage,
                         sizeof(keyboardPage)) == 0)
        keyboardPage = CP_ACP; // Fallback to default ANSI code page when fails.
    std::cout << "Keybard page: " << keyboardPage << std::endl;
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite
// your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
// clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide them
// from your application based on those two flags.
LRESULT WINAPI
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // LOG(info,
    //     "msg: {}, wParam: {}, lParam: {}",
    //     fmt::format("{:#x}", msg).c_str(),
    //     fmt::format("{:#x}", wParam).c_str(),
    //     fmt::format("{:#x}", lParam).c_str());
    switch (msg) {
        case WM_INPUTLANGCHANGE: {
            UpdateKeyboardCodePage();
            return S_OK;
        }
        case WM_CHAR: {
            LOG(info, "MSG [WM_CHAR] {}", wParam);
            break;
        }
        case WM_IME_CHAR: {
            LOG(info, "MSG [WM_IME_CHAR] {}", wParam);
            char mbstr[3]{ '\0' };
            BYTE hiByte = HIBYTE(wParam);
            BYTE loByte = LOBYTE(wParam);
            if (hiByte == 0) {
                mbstr[0] = loByte;
            } else {
                mbstr[0] = hiByte;
                mbstr[1] = loByte;
            }
            wchar_t wstr[2];
            MultiByteToWideChar(keyboardPage, MB_PRECOMPOSED, mbstr, -1, wstr, _countof(wstr));
            if (ImGui::GetCurrentContext()) {
                ImGuiIO& io = ImGui::GetIO();
                io.AddInputCharacter(wstr[0]);
                io.AddInputCharacter(wstr[1]);
            }
            Transmogrify::utils::NormalizeTest(wstr, keyboardPage);
            LOG(info, "ime char: {}", Transmogrify::utils::WideStrToStrCodePage(wstr, keyboardPage));
            return S_OK;
            // break;
        }
        case WM_IME_NOTIFY: {
            switch (wParam) {
                case IMN_OPENCANDIDATE:
                case IMN_SETCANDIDATEPOS:
                case IMN_CHANGECANDIDATE: {
                    HIMC hIMC = ImmGetContext(hWnd);
                    if (!hIMC)
                        break;
                    DWORD dwBufLen = ImmGetCandidateList(hIMC, 0, nullptr, 0); // Get size first
                    if (dwBufLen == 0) {
                        ImmReleaseContext(hWnd, hIMC);
                        break;
                    }
                    LOG(debug, "Candidate list length {}", dwBufLen);
                    candChosen = true;
                    ImmReleaseContext(hWnd, hIMC);
                    break;
                }
                case IMN_CLOSECANDIDATE:
                    candChosen = false;
                    break;
            }
            break;
        }
        case WM_IME_SETCONTEXT:
            return ::DefWindowProcW(hWnd, msg, wParam, lParam);
        case WM_SIZE: {
            if (wParam == SIZE_MINIMIZED)
                return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        }
        case WM_SYSCOMMAND: {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        }
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        default:
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                return true;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}