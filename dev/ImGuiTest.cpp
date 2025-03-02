// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "AddressLibTool.hpp"
#include "Status.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "spdlog/spdlog.h"
#include "utils.hpp"
#include <array>
#include <d3d11.h>
#include <dinput.h>
#include <iostream>
#include <msctf.h>
#include <vector>
#include <windowsx.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

// Data
static ID3D11Device           *g_pd3dDevice        = nullptr;
static ID3D11DeviceContext    *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain         *g_pSwapChain        = nullptr;
static bool                    g_SwapChainOccluded = false;
static UINT                    g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static bool                    candChosen             = false;
// compsition varibles
static ImVec2               inputPos       = {};
static ImVec2               compWindowSize = {};
static std::wstring         compStr;
static bool                 showCompWindow    = false;
static LPDIRECTINPUT8       g_pDirectInput    = nullptr;
static LPDIRECTINPUTDEVICE8 g_pKeyboardDevice = nullptr;

// Forward declarations of helper functions
bool           CreateDeviceD3D(HWND hWnd);
void           CleanupDeviceD3D();
void           CreateRenderTarget();
void           CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI ChildWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void           Render(HWND main, HWND child);
void           UpdateKeyboardCodePage();
static void    MyPlatform_SetImeDataFn_DefaultImpl(ImGuiContext *, ImGuiViewport *viewport, ImGuiPlatformImeData *data);
static void    RenderImeState();
void           OnStatusbarSize(HWND hWnd, UINT state, int cx, int cy);
void           ConfigStyle();

std::array<char, 256> keyboardState{0};

auto GetState() noexcept -> bool;

static HINSTANCE winst;
static HWND      parentWnd;

// Main code
int main(int, char **)
{
    AddressLibTool::DumpSpecialVerAddress(6, 1170);

    // Create application window
    // ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc,          0L,     0L, GetModuleHandle(nullptr), nullptr, nullptr,
                      nullptr,    nullptr,    L"ImGui Example", nullptr};
    WNDCLASSEXW wchild = {sizeof(wc),
                          CS_CLASSDC,
                          ChildWndProc,
                          0L,
                          0L,
                          GetModuleHandle(nullptr),
                          nullptr,
                          nullptr,
                          (HBRUSH)GetStockObject(GRAY_BRUSH),
                          NULL,
                          L"Child Window",
                          nullptr};
    ::RegisterClassExW(&wc);
    //    ::RegisterClassExW(&wchild);
    winst     = wc.hInstance;
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280,
                                800, nullptr, nullptr, wc.hInstance, nullptr);
    parentWnd = hwnd;

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ConfigStyle();
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
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 16.0f, nullptr,
                                 io.Fonts->GetGlyphRangesChineseFull());

    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode                     = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
    // io.Fonts->AddFontFromFileTTF(config->emojiFontFile.c_str(), config->fontSize, &imFontConfig, icons_ranges);
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 16.0f, &cfg, icons_ranges);
    io.Fonts->Build();
    ImGui::GetMainViewport()->PlatformHandleRaw = (void *)hwnd;
    UpdateKeyboardCodePage();
    //    HWND child = ::CreateWindowW(wchild.lpszClassName, L"SKSE Transmogrify Window", WS_POPUP, 0, 0, 0, 0, hwnd,
    //    NULL,
    //                                 winst, nullptr);

    // Our state
    bool   show_demo_window                      = true;
    bool   show_another_window                   = false;
    ImVec4 clear_color                           = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGui::GetPlatformIO().Platform_SetImeDataFn = MyPlatform_SetImeDataFn_DefaultImpl;

    // Main loop
    // DisableIME();
    bool                           done = false;
    std::unordered_map<BYTE, bool> keyStateMap;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
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
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves.We use a Begin / End pair to create a named window.
        {
            static float f       = 0.0f;
            static int   counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            // ImGui::SameLine();
            ImGui::Text("\xf0\x9f\x8d\x89 \xe2\x9c\x94\xef\xb8\x8f");
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::EndGroup();

            static char buf[64] = "";
            ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));
            if (true)
            {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32_WHITE);
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
                ImGui::SetNextWindowPos(inputPos);

                ImVec2 childSize = {(compStr.size() >> 1) * ImGui::GetFontSize(), ImGui::GetTextLineHeight()};
                ImGui::BeginChild("A child", childSize, 0, 0);
                ImGui::GetForegroundDrawList()->AddLine({inputPos.x, inputPos.y + childSize.y},
                                                        {inputPos.x + childSize.x, inputPos.y + childSize.y},
                                                        IM_COL32(125, 125, 125, 255));
                auto str = Transmogrify::utils::WideStrToStrUTF8(compStr);
                ImGui::Text(str.c_str());
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
            }
            ImGui::End();
        }
        //        Render(hwnd, child);
        RenderImeState();

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin(
                "Another Window", &show_another_window,
                ImGuiWindowFlags_NoDecoration |
                    ImGuiWindowFlags_AlwaysAutoResize); // Pass a pointer to our bool variable (the window will have a

            static bool showChild = false;
            static bool collapse = false;

            if (showChild)
            {
                if (ImGui::CollapsingHeader("Settings", &showChild))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0.0f, 10.0f});
                    if (ImGui::BeginTable("Test Table", 3, ImGuiTableFlags_RowBg))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Hello 0, 0");
                        ImGui::SameLine();
                        ImGui::Text("Hello 0, 0 1");
                        ImGui::SameLine();
                        ImGui::Text("Hello 0, 0 2");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Hello 0, 1");
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Hello 0, 2");

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Hello 1, 0");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Hello 1, 1");
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Hello 1, 2");

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Hello 2, 0");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Hello 2, 1");
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Hello 2, 2");

                        ImGui::EndTable();
                    }
                    ImGui::PopStyleVar();
                }
            }
            ImGui::Checkbox("Show Child", &showChild);
            ImGui::SameLine();
            if (ImGui::Button("Close Me")) show_another_window = false;

            ImGui::End();
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                                                 clear_color.z * clear_color.w, clear_color.w};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
        // HRESULT hr  = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

        UpdateWindow(hwnd);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

static void MyPlatform_SetImeDataFn_DefaultImpl(ImGuiContext *, ImGuiViewport *viewport, ImGuiPlatformImeData *data)
{
    inputPos.x = data->InputPos.x + 5;
    inputPos.y = data->InputPos.y;

    // Notify OS Input Method Editor of text input position
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;
    if (hwnd == 0) return;

    //::ImmAssociateContextEx(hwnd, NULL, data->WantVisible ? IACE_DEFAULT : 0);
    if (HIMC himc = ::ImmGetContext(hwnd))
    {
        COMPOSITIONFORM composition_form = {};
        composition_form.ptCurrentPos.x  = (LONG)(data->InputPos.x - viewport->Pos.x);
        composition_form.ptCurrentPos.y  = (LONG)(data->InputPos.y - viewport->Pos.y);
        composition_form.dwStyle         = CFS_FORCE_POSITION;
        ::ImmSetCompositionWindow(himc, &composition_form);
        CANDIDATEFORM candidate_form  = {};
        candidate_form.dwStyle        = CFS_CANDIDATEPOS;
        candidate_form.ptCurrentPos.x = (LONG)(data->InputPos.x - viewport->Pos.x);
        candidate_form.ptCurrentPos.y = (LONG)(data->InputPos.y - viewport->Pos.y);
        ::ImmSetCandidateWindow(himc, &candidate_form);
        ::ImmReleaseContext(hwnd, himc);
    }
}

static void Render(HWND main, HWND child)
{
}

static void RenderImeState()
{
    HIMC himc;
    if (!(himc = ImmGetContext(parentWnd))) return;
    DWORD conversion, sentence;
    if (ImmGetConversionStatus(himc, &conversion, &sentence))
    {

        switch (conversion)
        {
            case IME_CMODE_ALPHANUMERIC:
                ImGui::Text("IME Conversion: ALPHANUMERIC ");
                break;
            case IME_CMODE_NATIVE:
                ImGui::Text("IME Conversion: NATIVE");
                break;
            case IME_CMODE_KATAKANA:
                ImGui::Text("IME Conversion: KATAKANA");
                break;
            case IME_CMODE_LANGUAGE:
                ImGui::Text("IME Conversion: LANGUAGE");
                break;
            case IME_CMODE_FULLSHAPE:
                ImGui::Text("IME Conversion: FULLSHAPE");
                break;
            case IME_CMODE_ROMAN:
                ImGui::Text("IME Conversion: ROMAN");
                break;
            case IME_CMODE_CHARCODE:
                ImGui::Text("IME Conversion: CHARCODE");
                break;
            case IME_CMODE_HANJACONVERT:
                ImGui::Text("IME Conversion: HANJACONVERT");
                break;
            case IME_CMODE_NATIVESYMBOL:
                ImGui::Text("IME Conversion: NATIVESYMBOL");
                break;
        }
        switch (sentence)
        {
            case IME_SMODE_NONE:
                ImGui::Text("IME Sentence: NONE");
                break;
            case IME_SMODE_PLAURALCLAUSE:
                ImGui::Text("IME Sentence: PLAURALCLAUSE");
                break;
            case IME_SMODE_SINGLECONVERT:
                ImGui::Text("IME Sentence: SINGLECONVERT");
                break;
            case IME_SMODE_AUTOMATIC:
                ImGui::Text("IME Sentence: AUTOMATIC");
                break;
            case IME_SMODE_PHRASEPREDICT:
                ImGui::Text("IME Sentence: PHRASEPREDICT");
                break;
            case IME_SMODE_CONVERSATION:
                ImGui::Text("IME Sentence: CONVERSATION");
                break;
            case IME_SMODE_RESERVED:
                ImGui::Text("IME Sentence: RESERVED");
                break;
        }
    }

    ImmReleaseContext(parentWnd, himc);
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hWnd;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags                = 0;
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL       featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
                                                featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
                                                &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
                                            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice,
                                            &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain)
    {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext)
    {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void CreateRenderTarget()
{
    ID3D11Texture2D *pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void GetCandidateList(HIMC hIMC)
{
    // Get the size of the candidate list buffer
    DWORD bufferSize = ImmGetCandidateList(hIMC, 0, NULL, 0);
    if (bufferSize == 0)
    {
        std::cerr << "Failed to get the size of the candidate list buffer." << std::endl;
        return;
    }

    // Allocate buffer for the candidate list
    std::vector<BYTE> buffer(bufferSize);
    // LPCANDIDATELIST candList = reinterpret_cast<LPCANDIDATELIST>(buffer.data());
    LPCANDIDATELIST candList = (LPCANDIDATELIST)GlobalAlloc(GPTR, bufferSize);

    // Retrieve the candidate list
    DWORD copiedSize = ImmGetCandidateList(hIMC, 0, candList, bufferSize);
    if (copiedSize == 0)
    {
        std::cerr << "Failed to retrieve the candidate list." << std::endl;
        return;
    }

    // Output the candidate strings
    std::wcout << L"Candidate List:" << std::endl;
    for (DWORD i = 0; i < candList->dwCount; ++i)
    {
        LPWSTR candidateStr = reinterpret_cast<LPWSTR>(reinterpret_cast<BYTE *>(candList) + candList->dwOffset[i]);
        std::wcout << L"Candidate " << i + 1 << L": " << candidateStr << std::endl;
    }
    GlobalFree(candList);
}

static int keyboardPage = CP_ACP;

static void UpdateKeyboardCodePage()
{
    // Retrieve keyboard code page, required for handling of non-Unicode Windows.
    HKL  keyboard_layout = ::GetKeyboardLayout(0);
    LCID keyboard_lcid   = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
    if (::GetLocaleInfoA(keyboard_lcid, (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE), (LPSTR)&keyboardPage,
                         sizeof(keyboardPage)) == 0)
        keyboardPage = CP_ACP; // Fallback to default ANSI code page when fails.
    std::cout << "Keybard page: " << keyboardPage << std::endl;
}

static bool ImeNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HIMC hIMC;
    switch (wParam)
    {
        case IMN_SETOPENSTATUS: {
            SetStatusItems(hWnd);
            break;
        }
        case IMN_OPENCANDIDATE: {
            showCompWindow = true;
            break;
        }
        case IMN_SETCANDIDATEPOS:
        case IMN_CHANGECANDIDATE: {
            hIMC = ImmGetContext(hWnd);
            if (!hIMC) break;
            DWORD dwBufLen = ImmGetCandidateList(hIMC, 0, nullptr, 0); // Get size first
            if (dwBufLen == 0)
            {
                ImmReleaseContext(hWnd, hIMC);
                break;
            }
            LOG(info, "Candidate list length {}", dwBufLen);
            candChosen = true;
            ImmReleaseContext(hWnd, hIMC);
            break;
        }
        case IMN_CLOSECANDIDATE:
            candChosen     = false;
            showCompWindow = false;
            break;
    }
    return true;
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite
// your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
// clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide them
// from your application based on those two flags.

void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC         hdc = BeginPaint(hwnd, &ps);
    TextOut(hdc, 0, 0, L"Hello, Windows!", 15);
    EndPaint(hwnd, &ps);
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) != 0)
    {
        return true;
    }
    switch (msg)
    {
        case WM_CREATE:
            if (CreateStatus(hWnd, winst) != 0)
            {
                SetStatusItems(hWnd);
                return S_OK;
            }
            return S_FALSE;
        case WM_INPUTLANGCHANGE: {
            UpdateKeyboardCodePage();
            return S_OK;
        }
        case WM_IME_CHAR: {
            LOG(info, "Parebt msg: {}, wParam: {}, lParam: {}", fmt::format("{:#x}", msg).c_str(),
                fmt::format("{:#x}", wParam).c_str(), fmt::format("{:#x}", lParam).c_str());

            // try discard unable character
            if (wParam == 63)
            {
                LOG(err, "unknown char");
            }

            char       mbstr[3]{'\0'};
            BYTE const hiByte = HIBYTE(wParam);
            BYTE const loByte = LOBYTE(wParam);
            if (hiByte == 0)
            {
                mbstr[0] = loByte;
            }
            else
            {
                mbstr[0] = hiByte;
                mbstr[1] = loByte;
            }
            wchar_t wstr[2];
            MultiByteToWideChar(keyboardPage, MB_PRECOMPOSED, mbstr, -1, wstr, _countof(wstr));
            //            if (ImGui::GetCurrentContext())
            //            {
            //                ImGuiIO &io = ImGui::GetIO();
            //                io.AddInputCharacter(wstr[0]);
            //                io.AddInputCharacter(wstr[1]);
            //            }
            LOG(info, "ime char: {}", Transmogrify::utils::WideStrToStrCodePage(wstr, keyboardPage));
            return S_OK;
            break;
        }
        case WM_CHAR: {
            LOG(info, "MSG [WM_CHAR] {}", wParam);
            break;
        }
        case WM_IME_NOTIFY: {
            //            ImeNotify(hWnd, wParam, lParam);
            return S_OK;
        }

            HANDLE_MSG(hWnd, WM_SIZE, OnStatusbarSize);
            HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
        case WM_SYSCOMMAND: {
            if ((wParam & 0xfff0) == SC_KEYMENU)
            { // Disable ALT application menu
                return 0;
            }
            break;
        }
        case WM_IME_SETCONTEXT:
            // lParam &= ~ISC_SHOWUICANDIDATEWINDOW;
            return DefWindowProcW(hWnd, WM_IME_SETCONTEXT, wParam, 0);
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void OnStatusbarSize(HWND hWnd, UINT state, int cx, int cy)
{
    if (state == SIZE_MINIMIZED) return;
    g_ResizeWidth  = cx; // Queue resize
    g_ResizeHeight = cy;

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    int *pPartsWidth = (int *)LocalAlloc(LPTR, sizeof(int) * NUM_PARTS);
    if (pPartsWidth == nullptr) return;
    int nWidth    = rcClient.right / NUM_PARTS;
    int rightEdge = nWidth;
    for (int i = 0; i < NUM_PARTS; i++)
    {
        rightEdge += nPartsWidthTbl[i];
        pPartsWidth[i] = nWidth;
    }

    SendMessage(hStatusWnd, SB_SETPARTS, (WPARAM)NUM_PARTS, (LPARAM)pPartsWidth);

    LocalFree((HANDLE)pPartsWidth);
    // Resize statusbar so it's always same width as parent's client area
    SendMessage(hStatusWnd, WM_SIZE, 0, 0);
    InvalidateRect(hWnd, &rcClient, TRUE);
    UpdateWindow(hWnd);
}

LRESULT WINAPI ChildWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_SYSCOMMAND:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_IME_SETCONTEXT:
        case WM_INPUTLANGCHANGE:
        case WM_IME_STARTCOMPOSITION:
        // case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_NOTIFY:
            ::SendMessage(parentWnd, msg, wParam, lParam);
            return S_OK;
        case WM_CHAR: {
            ImGui::GetIO().AddInputCharacterUTF16((WCHAR)wParam);
            return S_OK;
        }
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

auto GetState() noexcept -> bool
{
    HRESULT hresult = TRUE;
    g_pKeyboardDevice->Poll();
    hresult = g_pKeyboardDevice->GetDeviceState(sizeof(keyboardState), reinterpret_cast<LPVOID>(keyboardState.data()));
    if (hresult == DIERR_INPUTLOST || hresult == DIERR_NOTACQUIRED)
    {
        g_pKeyboardDevice->Acquire();
        hresult =
            g_pKeyboardDevice->GetDeviceState(sizeof(keyboardState), reinterpret_cast<LPVOID>(keyboardState.data()));
    }
    return SUCCEEDED(hresult);
}

#define RGB(hex) ImColor((hex & 0xff0000) >> 16, (hex & 0xff00) >> 8, (hex & 0xff))

void ConfigStyle()
{
    auto   &style                   = ImGui::GetStyle();
    ImVec4 *colors                  = style.Colors;

    colors[ImGuiCol_WindowBg]       = RGB(0x2E3440);
    colors[ImGuiCol_Border]         = RGB(0x434C5E);
    colors[ImGuiCol_Text]           = RGB(0xE2E8F0);
    colors[ImGuiCol_Button]         = RGB(0x4C566A);
    colors[ImGuiCol_ButtonHovered]  = RGB(0x5E81AC);
    colors[ImGuiCol_ButtonActive]   = RGB(0x88C0D0);
    colors[ImGuiCol_Header]         = colors[ImGuiCol_Button];
    colors[ImGuiCol_HeaderHovered]  = colors[ImGuiCol_ButtonHovered];
    colors[ImGuiCol_HeaderActive]   = colors[ImGuiCol_ButtonActive];
    colors[ImGuiCol_FrameBg]        = colors[ImGuiCol_Button];
    colors[ImGuiCol_FrameBgHovered] = colors[ImGuiCol_ButtonHovered];
    colors[ImGuiCol_FrameBgActive]  = colors[ImGuiCol_ButtonActive];
}