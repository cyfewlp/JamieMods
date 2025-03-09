// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
// #include "AddressLibTool.hpp"
#include "SimpleIni.h"
#include "Status.h"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "spdlog/spdlog.h"

#include <array>
#include <commdlg.h>
#include <d3d11.h>
#include <dinput.h>
#include <format>
#include <msctf.h>
#include <ranges>
#include <regex>
#include <span>
#include <string>
#include <windowsx.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

using namespace std::literals;

// Data
static ID3D11Device           *g_pd3dDevice        = nullptr;
static ID3D11DeviceContext    *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain         *g_pSwapChain        = nullptr;
static bool                    g_SwapChainOccluded = false;
static UINT                    g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static bool                    candChosen             = false;
// compsition varibles
static ImVec2               inputPos          = {};
static LPDIRECTINPUT8       g_pDirectInput    = nullptr;
static LPDIRECTINPUTDEVICE8 g_pKeyboardDevice = nullptr;
static bool                 g_fShowToolWindow = false;
static bool                 g_fFollowCursor   = false;
static bool                 m_fPinToolWindow  = false;
static bool                 g_fShowSettings   = false;
static int                  g_toolWindowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration;
static ImGuiKey             g_keyDown         = ImGuiKey_None;

extern ImGuiKey ImGui_ImplWin32_KeyEventToImGuiKey(WPARAM wParam, LPARAM lParam);

namespace
{

#define HEX_TO_RGB(hex)  ImColor((hex & 0xff0000) >> 16, (hex & 0xff00) >> 8, (hex & 0xff))
#define HEX_TO_RGBA(hex) ImColor((hex & 0xff0000) >> 16, (hex & 0xff00) >> 8, (hex & 0xff), (hex & 0xff000000) >> 24)

    enum KeyModifier : uint8_t
    {
        MODIFIER_SHIFT = 1 << 0,
        MODIFIER_CTRL  = 1 << 1,
        MODIFIER_ALT   = 1 << 2,
    };

    struct Shortcut
    {
        uint8_t     modifiers = 0;
        uint32_t    key       = 0;
        std::string desc{};
    };

    uint8_t g_keyModifiers = ImGuiKey_None;

    enum ImeUiColorIdx : int
    {
        Text_Color = 0,
        Highlight_Text_Color,
        Border_Color,
        Window_Background_Color,
        Button_Color,
        Button_Hovered_Color,
        Button_Active_Color,
        Header_Color,
        Header_Hovered_Color,
        Header_Active_Color,
        FrameBg_Color,
        FrameBg_Hovered_Color,
        FrameBg_Active_Color,
        UiColorCount,
    };

    struct ImeTheme
    {
        std::array<ImVec4, UiColorCount> uiColors{};
        std::string                      name;
        bool                             isMutable = true;

        ImeTheme(const std::string &name, bool isMutable) : name(std::move(name)), isMutable(isMutable)
        {
            uiColors.fill({0.0f, 0.0f, 0.0f, 1.0f});
        }
    };

    struct ImeConfig
    {
        Shortcut toolWindowShortcutKey;
        uint32_t Log_Level   = spdlog::level::info;
        uint32_t Flush_Level = spdlog::level::trace;

        bool                  Enable_Tsf        = false;
        bool                  Always_Active_Ime = false;
        bool                  Use_Classic_Theme = false;
        std::vector<ImeTheme> themes;

        ImeConfig()
        {
            ImeTheme defaultTheme                             = {"Modern", false};
            defaultTheme.uiColors.at(Text_Color)              = ImColor(0xFFFFFFFF);
            defaultTheme.uiColors.at(Highlight_Text_Color)    = ImColor(0xFF00D7FF);
            defaultTheme.uiColors.at(Border_Color)            = ImColor(0x6D6D7F80);
            defaultTheme.uiColors.at(Window_Background_Color) = ImColor(0x0A0A0AF0);
            defaultTheme.uiColors.at(Button_Color)            = ImColor(0x0A0A0AF0);
            defaultTheme.uiColors.at(Button_Hovered_Color)    = ImColor(0x191919FF);
            defaultTheme.uiColors.at(Button_Active_Color)     = ImColor(0xFF3B3BFF);
            defaultTheme.uiColors.at(Header_Color)            = ImColor(0x00000074);
            defaultTheme.uiColors.at(Header_Hovered_Color)    = ImColor(0x2E2E2ECC);
            defaultTheme.uiColors.at(Header_Active_Color)     = ImColor(0x2E2E2ECC);
            defaultTheme.uiColors.at(FrameBg_Color)           = ImColor(0x0000006B);
            defaultTheme.uiColors.at(FrameBg_Hovered_Color)   = ImColor(0x24242466);
            defaultTheme.uiColors.at(FrameBg_Active_Color)    = ImColor(0x3B3B3BDC);

            ImeTheme lightTheme                             = {"Light", false};
            lightTheme.uiColors.at(Text_Color)              = ImColor(0xFF333333);
            lightTheme.uiColors.at(Highlight_Text_Color)    = ImColor(0xFFCC6600);
            lightTheme.uiColors.at(Border_Color)            = ImColor(0xFFD6D6D6);
            lightTheme.uiColors.at(Window_Background_Color) = ImColor(0x80F7F7F7);
            lightTheme.uiColors.at(Button_Color)            = ImColor(0x9AE0E0E0);
            lightTheme.uiColors.at(Button_Hovered_Color)    = ImColor(0x66E0E0E0);
            lightTheme.uiColors.at(Button_Active_Color)     = ImColor(0xAAE0E0E0);

            ImeTheme defaultCyperpunkTheme                             = {"Cyperpunk", false};
            defaultCyperpunkTheme.uiColors.at(Text_Color)              = ImColor(0xFFE0E0E0);
            defaultCyperpunkTheme.uiColors.at(Highlight_Text_Color)    = ImColor(0xFFFF00FF);
            defaultCyperpunkTheme.uiColors.at(Border_Color)            = ImColor(0xFFFFFF00);
            defaultCyperpunkTheme.uiColors.at(Window_Background_Color) = ImColor(0x80000000);
            defaultCyperpunkTheme.uiColors.at(Button_Color)            = ImColor(0x9A222222);
            defaultCyperpunkTheme.uiColors.at(Button_Hovered_Color)    = ImColor(0x66222222);
            defaultCyperpunkTheme.uiColors.at(Button_Active_Color)     = ImColor(0xAA222222);

            themes.push_back(defaultTheme);
            themes.push_back(lightTheme);
            themes.push_back(defaultCyperpunkTheme);
        }
    };

    struct GuiView
    {
        bool        showToolWindowShortcutKeyConfig;
        bool        commitRenameTheme;
        uint32_t    selectedThemeIdx;
        std::string ensureDeletePopupName = "EnsureDelete?";
    };

    // Forward declarations of helper functions
    bool           CreateDeviceD3D(HWND hWnd);
    void           CleanupDeviceD3D();
    void           CreateRenderTarget();
    void           CleanupRenderTarget();
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void           OnStatusbarSize(HWND hWnd, UINT state, int cx, int cy);
    void           RenderSettings();
    void           ShowToolWindow();
    void           RenderToolWindow();
    bool           InitDirectInput() noexcept;
    bool           GetState() noexcept;
    void           RenderLogLevelConfig(const char *label, uint32_t &level);
    void           RenderThemeUiConfig(ImeConfig &config, GuiView &guiView);
    bool           LoadStyleFromFile(const char *path);
    bool           LoadColors(CSimpleIniA &ini, ImGuiStyle &style);
    void           LoadColor(CSimpleIniA &ini, const char *key, ImVec4 &color);

    std::array<char, 256> keyboardState{0};

}

static HINSTANCE winst;
static HWND      parentWnd;

// Main code
int main(int, char **)
{
    // AddressLibTool::DumpSpecialVerAddress(5, 97);
    // return 0;

    // Create application window
    // ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = {sizeof(wc), CS_CLASSDC, WndProc,          0L,     0L, GetModuleHandle(nullptr), nullptr, nullptr,
                      nullptr,    nullptr,    L"ImGui Example", nullptr};
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
    InitDirectInput();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking

    ImGui::StyleColorsDark();
    // ConfigStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImFont *font1 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 13.0f, nullptr,
                                                 io.Fonts->GetGlyphRangesChineseFull());
    ImFont *font2 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 16.0f, nullptr,
                                                 io.Fonts->GetGlyphRangesChineseFull());

    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode                     = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 13.0f, &cfg, icons_ranges);
    io.Fonts->Build();

    ImGui::GetMainViewport()->PlatformHandleRaw = (void *)hwnd;

    // Our state
    bool show_demo_window    = true;
    bool show_another_window = false;
    auto clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ShowToolWindow();

    bool                           done = false;
    std::unordered_map<BYTE, bool> keyStateMap;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;
        GetState();

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(font1);
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        {
            static float f       = 0.0f;
            static int   counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            ImGui::Text("\xf0\x9f\x8d\x89 \xe2\x9c\x94\xef\xb8\x8f");
            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::EndGroup();

            static char buf[64] = "";
            ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));
            ImGui::End();
        }

        ImGui::Begin("Candidate");
        auto &style = ImGui::GetStyle();
        auto color = style.Colors[ImGuiCol_FrameBgActive];

        ImDrawList * drawList = ImGui::GetWindowDrawList();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.0f, 10.0f});
        ImGui::Text("1. ni");
        ImGui::SameLine();

        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        cursorPos.x -=  style.ItemSpacing.x / 2;
        cursorPos.y -=  style.ItemSpacing.y / 2;
        auto width = ImGui::CalcTextSize("2. wo").x;
        drawList->AddRectFilled(cursorPos,
            cursorPos + ImVec2(width + style.ItemSpacing.x, ImGui::GetTextLineHeight() + style.ItemSpacing.y),
            ImColor(color));
        ImGui::Text("2. wo");
        ImGui::SameLine();

        ImGui::Text("3. ta");
        ImGui::SameLine();
        ImGui::PopStyleVar();
        ImGui::End();

        RenderToolWindow();

        ImGui::PopFont();

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

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
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

        UINT createDeviceFlags = 0;
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
                                                featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
                                                &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
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

    // Win32 message handler
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
    // inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite
    // your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
    // clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide
    // them from your application based on those two flags.

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
                HANDLE_MSG(hWnd, WM_SIZE, OnStatusbarSize);
                HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
            case WM_SYSCOMMAND: {
                if ((wParam & 0xfff0) == SC_KEYMENU)
                { // Disable ALT application menu
                    return 0;
                }
                break;
            }
            case WM_KEYDOWN:
                if (!(keyboardState[DIK_LSHIFT] & 0x80) && !(keyboardState[DIK_RSHIFT] & 0x80) //
                    && !(keyboardState[DIK_LALT] & 0x80) && !(keyboardState[DIK_RALT] & 0x80)  //
                    && !(keyboardState[DIK_LCONTROL] & 0x80) && !(keyboardState[DIK_RCONTROL] & 0x80))
                {
                    g_keyModifiers = 0;
                }
                if (wParam == VK_SHIFT)
                {
                    g_keyModifiers |= MODIFIER_SHIFT;
                }
                else if (wParam == VK_CONTROL)
                {
                    g_keyModifiers |= MODIFIER_CTRL;
                }
                else if (wParam == VK_MENU)
                {
                    g_keyModifiers |= MODIFIER_ALT;
                }
                else
                {
                    g_keyDown = ImGui_ImplWin32_KeyEventToImGuiKey(wParam, lParam);
                }

                if (wParam == VK_F2)
                {
                    ShowToolWindow();
                }
                return 0;
            case WM_DESTROY:
                ::PostQuitMessage(0);
                return 0;
            default:
                break;
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

        auto pPartsWidth = (int *)LocalAlloc(LPTR, sizeof(int) * NUM_PARTS);
        if (pPartsWidth == nullptr) return;
        int nWidth    = rcClient.right / NUM_PARTS;
        int rightEdge = nWidth;
        for (int i = 0; i < NUM_PARTS; i++)
        {
            rightEdge += nPartsWidthTbl[i];
            pPartsWidth[i] = nWidth;
        }

        SendMessage(hStatusWnd, SB_SETPARTS, (WPARAM)NUM_PARTS, (LPARAM)pPartsWidth);

        LocalFree(pPartsWidth);
        // Resize statusbar so it's always same width as parent's client area
        SendMessage(hStatusWnd, WM_SIZE, 0, 0);
        InvalidateRect(hWnd, &rcClient, TRUE);
        UpdateWindow(hWnd);
    }

    bool GetState() noexcept
    {
        g_pKeyboardDevice->Poll();
        HRESULT hresult = g_pKeyboardDevice->GetDeviceState(sizeof(keyboardState), keyboardState.data());
        if (hresult == DIERR_INPUTLOST || hresult == DIERR_NOTACQUIRED)
        {
            g_pKeyboardDevice->Acquire();
            hresult = g_pKeyboardDevice->GetDeviceState(sizeof(keyboardState), keyboardState.data());
        }
        return SUCCEEDED(hresult);
    }

    void RenderToolWindow()
    {
        if (!g_fShowToolWindow)
        {
            return;
        }

        ImGui::Begin("DevImeToolWindowTest", &g_fShowToolWindow, g_toolWindowFlags);

        RenderSettings();

        ImGui::Text("Drag");
        ImGui::SameLine();

        if (ImGui::Button("\xf0\x9f\x93\x8c"))
        {
            g_toolWindowFlags |= ImGuiWindowFlags_NoInputs;
            m_fPinToolWindow = true;
        }
        ImGui::SameLine();

        ImGui::Checkbox("Settings", &g_fShowSettings);
        ImGui::SameLine();

        static std::array<std::string_view, 5> fakeLangProfiles = {"ENG", "SouGou", "Microsoft Pinyin", "Google Pinyin",
                                                                   "Baidu"};

        static uint8_t selectedProfileIdx = 0;
        const char    *previewImeName     = fakeLangProfiles[selectedProfileIdx].data();
        ImGui::SameLine();
        if (ImGui::BeginCombo("###InstalledIME", previewImeName))
        {
            uint8_t idx = 0;
            for (const std::string_view langProfile : fakeLangProfiles)
            {
                if (ImGui::Selectable(langProfile.data()))
                {
                    selectedProfileIdx = idx;
                }
                if (selectedProfileIdx == idx)
                {
                    ImGui::SetItemDefaultFocus();
                }
                idx++;
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();

        // if (!m_fPinToolWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        // {
        //     ShowToolWindow();
        // }
        ImGui::End();
    }

    void RenderSettings()
    {
        static bool EnableMod       = true;
        static bool CollapseVisible = false;
        CollapseVisible             = g_fShowSettings;

        if (!g_fShowSettings)
        {
            return;
        }

        ImGui::Begin("Settings", &CollapseVisible, ImGuiWindowFlags_AlwaysAutoResize);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 4));
            if (ImGui::BeginTable("SettingsTable", 3))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                static bool EnableModFail = false;
                ImGui::Checkbox("Enable Mod", &EnableMod);
                ImGui::SetItemTooltip("Uncheck will disable all mod feature(Disable keyboard).");
                if (EnableModFail)
                {
                    ImGui::TableNextColumn();
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, .0f, .0f, 1.0f), "%s", "Failed to enable mod");
                    if (ImGui::Button("X"))
                    {
                        EnableModFail = false;
                    }
                }

                ImGui::TableNextColumn();

                ImGui::Text("Ime Enabled %s", "\xe2\x9d\x8c"); // red  ❌

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Ime Focus: %s", "\xe2\x9c\x85");
                ImGui::SetItemTooltip("Mod must has keyboard focus to work.");

                ImGui::TableNextColumn();
                ImGui::Button("Force Focus Ime");

                ImGui::TableNextColumn();
                ImGui::Checkbox("Ime follow cursor", &g_fFollowCursor);
                ImGui::SetItemTooltip("Ime window appear in cursor position.");

                static ImeConfig config;
                static GuiView   guiView;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Checkbox("Tool_Window_Shortcut_Key", &guiView.showToolWindowShortcutKeyConfig) &&
                    guiView.showToolWindowShortcutKeyConfig)
                {
                    g_keyModifiers = config.toolWindowShortcutKey.modifiers;
                    g_keyDown      = static_cast<ImGuiKey>(config.toolWindowShortcutKey.key);
                }
                std::string shortcutDesc;
                auto        toShortcutDesc = [](uint8_t modifiers, ImGuiKey key) {
                    std::string shortcutDesc;
                    if ((modifiers & MODIFIER_CTRL) == MODIFIER_CTRL)
                    {
                        shortcutDesc += "Ctrl + ";
                    }
                    if ((modifiers & MODIFIER_SHIFT) == MODIFIER_SHIFT)
                    {
                        shortcutDesc += "Shift + ";
                    }
                    if ((modifiers & MODIFIER_ALT) == MODIFIER_ALT)
                    {
                        shortcutDesc += "Alt + ";
                    }
                    shortcutDesc += ImGui::GetKeyName(key);
                    return shortcutDesc;
                };
                if (guiView.showToolWindowShortcutKeyConfig)
                {
                    ImGui::OpenPopup("ShortcutConfig");
                }
                if (ImGui::BeginPopupModal("ShortcutConfig", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    shortcutDesc = toShortcutDesc(g_keyModifiers, g_keyDown);
                    ImGui::Text("%s", shortcutDesc.c_str());
                    if (ImGui::Button("OK", ImVec2(120, 0)))
                    {
                        guiView.showToolWindowShortcutKeyConfig = false;
                        config.toolWindowShortcutKey.modifiers  = g_keyModifiers;
                        config.toolWindowShortcutKey.key        = g_keyDown;
                        config.toolWindowShortcutKey.desc.assign(shortcutDesc);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        guiView.showToolWindowShortcutKeyConfig = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::TableNextColumn();
                RenderLogLevelConfig("Log level", config.Log_Level);

                ImGui::TableNextColumn();
                RenderLogLevelConfig("Flush level", config.Flush_Level);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Checkbox("Enable Tsf", &config.Enable_Tsf);

                ImGui::TableNextColumn();
                ImGui::Checkbox("Always Active Ime", &config.Always_Active_Ime);

                // UI configs
                // ImGui::Checkbox("East_Asia_Font_File", &config.East_Asia_Font_File);
                // ImGui::Checkbox("Emoji_Font_File", &config.Emoji_Font_File);
                ImGui::TableNextColumn();
                ImGui::Checkbox("Use_Classic_Theme", &config.Use_Classic_Theme);
                ImGui::EndTable();

                RenderThemeUiConfig(config, guiView);
            }
            ImGui::PopStyleVar();
        }
        ImGui::End();
    }

    void ShowToolWindow()
    {
        g_toolWindowFlags &= ~ImGuiWindowFlags_NoInputs;
        if (m_fPinToolWindow)
        {
            m_fPinToolWindow = false;
            ImGui::SetWindowFocus("DevImeToolWindowTest");
        }
        else
        {
            g_fShowToolWindow = !g_fShowToolWindow;
            if (g_fShowToolWindow)
            {
                ImGui::SetWindowFocus("DevImeToolWindowTest");
            }
        }
    }

    bool InitDirectInput() noexcept
    {
        if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
                                      reinterpret_cast<void **>(&g_pDirectInput), nullptr)))
        {
            return false;
        }

        if (FAILED(g_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pKeyboardDevice, nullptr)))
        {
            return false;
        }

        if (FAILED(g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
        {
            return false;
        }

        if (FAILED(g_pKeyboardDevice->SetCooperativeLevel(parentWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
        {
            return false;
        }
        g_pKeyboardDevice->Acquire();
        return true;
    }

    void RenderLogLevelConfig(const char *label, uint32_t &level)
    {
        static std::array<std::string_view, 5> logLevels = {"Trace", "Debug", "Info", "Warn", "Error"};

        if (ImGui::BeginCombo(label, logLevels[level].data(),
                              ImGuiComboFlags_HeightRegular | ImGuiComboFlags_PopupAlignLeft))
        {
            uint32_t idx = 0;
            for (const std::string_view &logLevel : logLevels)
            {
                if (ImGui::Selectable(logLevel.data()))
                {
                    level = idx;
                }
                if (level == idx)
                {
                    ImGui::SetItemDefaultFocus();
                }
                idx++;
            }
            ImGui::EndCombo();
        }
    }

    void RenderThemeUiConfig(ImeConfig &config, GuiView &guiView)
    {
        if (ImGui::CollapsingHeader("Theme Config", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Load"))
            {
                LoadStyleFromFile(R"(D:\repo\JamieMods\dev\themes\theme_modern.ini)");
            }
        }
    }

    static void GetFloatValue(const char *value, float *target)
    {
        if (value != nullptr)
        {
            char       *pEnd{};
            const float result = std::strtof(value, &pEnd);
            if (*pEnd == 0)
            {
                *target = result;
            }
        }
    }

    static void GetFloatValue(const CSimpleIniA &ini, const char *key, float *target)
    {
        if (auto *value = ini.GetValue("style", key); value != nullptr)
        {
            GetFloatValue(value, target);
        }
    }

    static void GetVec2Value(const CSimpleIniA &ini, const char *key, ImVec2 &target)
    {
        if (auto *value = ini.GetValue("style", key); value != nullptr)
        {
            if (std::string strValue(value); strValue.starts_with("[") && strValue.ends_with("]"))
            {
                strValue.erase(0, 1);
                strValue.erase(strValue.end() - 1);
                std::ranges::split_view view = std::views::split(strValue, std::string_view(", "));

                std::vector<std::string> elements;
                for (const auto &part : view)
                {
                    elements.emplace_back(part.begin(), part.end());
                }
                if (elements.size() == 2)
                {
                    GetFloatValue(elements[0].c_str(), &target.x);
                    GetFloatValue(elements[1].c_str(), &target.y);
                }
            }
        }
    }

    static void GetEnumDirValue(CSimpleIniA &ini, const char *key, ImGuiDir &dir)
    {
        /*ImGuiDir_Left    = 0,
        ImGuiDir_Right   = 1,
        ImGuiDir_Up      = 2,
        ImGuiDir_Down    = 3,*/
        if (auto *value = ini.GetValue("style", key); value != nullptr)
        {
            if (strcmp("Left", value) == 0)
            {
                dir = ImGuiDir_Left;
            }
            else if (strcmp("Right", value) == 0)
            {
                dir = ImGuiDir_Right;
            }
            else if (strcmp("Up", value) == 0)
            {
                dir = ImGuiDir_Up;
            }
            else if (strcmp("Down", value) == 0)
            {
                dir = ImGuiDir_Down;
            }
        }
    }

    bool LoadStyleFromFile(const char *path)
    {
        auto       &style = ImGui::GetStyle();
        CSimpleIniA ini;
        ini.LoadFile(path);

        GetFloatValue(ini, "style", &style.Alpha);
        GetFloatValue(ini, "disabledAlpha", &style.DisabledAlpha);
        GetVec2Value(ini, "windowPadding", style.WindowPadding);
        GetFloatValue(ini, "windowRounding", &style.WindowRounding);
        GetFloatValue(ini, "windowBorderSize", &style.WindowBorderSize);
        GetVec2Value(ini, "windowMinSize", style.WindowMinSize);
        GetEnumDirValue(ini, "windowMenuButtonPosition", style.WindowMenuButtonPosition);
        GetFloatValue(ini, "childRounding", &style.ChildRounding);
        GetFloatValue(ini, "popupRounding", &style.PopupRounding);
        GetFloatValue(ini, "popupBorderSize", &style.PopupBorderSize);
        GetVec2Value(ini, "framePadding", style.FramePadding);
        GetFloatValue(ini, "frameRounding", &style.FrameRounding);
        GetFloatValue(ini, "frameBorderSize", &style.FrameBorderSize);
        GetVec2Value(ini, "itemSpacing", style.ItemSpacing);
        GetVec2Value(ini, "itemInnerSpacing", style.ItemInnerSpacing);
        GetVec2Value(ini, "cellPadding", style.CellPadding);
        GetFloatValue(ini, "indentSpacing", &style.IndentSpacing);
        GetFloatValue(ini, "columnsMinSpacing", &style.ColumnsMinSpacing);
        GetFloatValue(ini, "scrollbarSize", &style.ScrollbarSize);
        GetFloatValue(ini, "scrollbarRounding", &style.ScrollbarRounding);
        GetFloatValue(ini, "grabMinSize", &style.GrabMinSize);
        GetFloatValue(ini, "grabRounding", &style.GrabRounding);
        GetFloatValue(ini, "tabRounding", &style.TabRounding);
        GetFloatValue(ini, "tabBorderSize", &style.TabBorderSize);
        GetFloatValue(ini, "tabMinWidthForCloseButton", &style.TabCloseButtonMinWidthUnselected);
        GetEnumDirValue(ini, "colorButtonPosition", style.ColorButtonPosition);
        GetVec2Value(ini, "buttonTextAlign", style.ButtonTextAlign);
        GetVec2Value(ini, "selectableTextAlign", style.SelectableTextAlign);

        LoadColors(ini, style);
        return true;
    }

    bool LoadColors(CSimpleIniA &ini, ImGuiStyle &style)
    {
        auto colors = std::span(style.Colors);
        LoadColor(ini, "Text", colors[ImGuiCol_Text]);
        LoadColor(ini, "TextDisabled", colors[ImGuiCol_TextDisabled]);
        LoadColor(ini, "WindowBg", colors[ImGuiCol_WindowBg]);
        LoadColor(ini, "ChildBg", colors[ImGuiCol_ChildBg]);
        LoadColor(ini, "PopupBg", colors[ImGuiCol_PopupBg]);
        LoadColor(ini, "Border", colors[ImGuiCol_Border]);
        LoadColor(ini, "BorderShadow", colors[ImGuiCol_BorderShadow]);
        LoadColor(ini, "FrameBg", colors[ImGuiCol_FrameBg]);
        LoadColor(ini, "FrameBgHovered", colors[ImGuiCol_FrameBgHovered]);
        LoadColor(ini, "FrameBgActive", colors[ImGuiCol_FrameBgActive]);
        LoadColor(ini, "TitleBg", colors[ImGuiCol_TitleBg]);
        LoadColor(ini, "TitleBgActive", colors[ImGuiCol_TitleBgActive]);
        LoadColor(ini, "TitleBgCollapsed", colors[ImGuiCol_TitleBgCollapsed]);
        LoadColor(ini, "MenuBarBg", colors[ImGuiCol_MenuBarBg]);
        LoadColor(ini, "ScrollbarBg", colors[ImGuiCol_ScrollbarBg]);
        LoadColor(ini, "ScrollbarGrab", colors[ImGuiCol_ScrollbarGrab]);
        LoadColor(ini, "ScrollbarGrabHovered", colors[ImGuiCol_ScrollbarGrabHovered]);
        LoadColor(ini, "ScrollbarGrabActive", colors[ImGuiCol_ScrollbarGrabActive]);
        LoadColor(ini, "CheckMark", colors[ImGuiCol_CheckMark]);
        LoadColor(ini, "SliderGrab", colors[ImGuiCol_SliderGrab]);
        LoadColor(ini, "SliderGrabActive", colors[ImGuiCol_SliderGrabActive]);
        LoadColor(ini, "Button", colors[ImGuiCol_Button]);
        LoadColor(ini, "ButtonHovered", colors[ImGuiCol_ButtonHovered]);
        LoadColor(ini, "ButtonActive", colors[ImGuiCol_ButtonActive]);
        LoadColor(ini, "Header", colors[ImGuiCol_Header]);
        LoadColor(ini, "HeaderHovered", colors[ImGuiCol_HeaderHovered]);
        LoadColor(ini, "HeaderActive", colors[ImGuiCol_HeaderActive]);
        LoadColor(ini, "Separator", colors[ImGuiCol_Separator]);
        LoadColor(ini, "SeparatorHovered", colors[ImGuiCol_SeparatorHovered]);
        LoadColor(ini, "SeparatorActive", colors[ImGuiCol_SeparatorActive]);
        LoadColor(ini, "ResizeGrip", colors[ImGuiCol_ResizeGrip]);
        LoadColor(ini, "ResizeGripHovered", colors[ImGuiCol_ResizeGripHovered]);
        LoadColor(ini, "ResizeGripActive", colors[ImGuiCol_ResizeGripActive]);
        LoadColor(ini, "Tab", colors[ImGuiCol_Tab]);
        LoadColor(ini, "TabHovered", colors[ImGuiCol_TabHovered]);
        LoadColor(ini, "TabActive", colors[ImGuiCol_TabActive]);
        LoadColor(ini, "TabUnfocused", colors[ImGuiCol_TabUnfocused]);
        LoadColor(ini, "TabUnfocusedActive", colors[ImGuiCol_TabUnfocusedActive]);
        LoadColor(ini, "PlotLines", colors[ImGuiCol_PlotLines]);
        LoadColor(ini, "PlotLinesHovered", colors[ImGuiCol_PlotLinesHovered]);
        LoadColor(ini, "PlotHistogram", colors[ImGuiCol_PlotHistogram]);
        LoadColor(ini, "PlotHistogramHovered", colors[ImGuiCol_PlotHistogramHovered]);
        LoadColor(ini, "TableHeaderBg", colors[ImGuiCol_TableHeaderBg]);
        LoadColor(ini, "TableBorderStrong", colors[ImGuiCol_TableBorderStrong]);
        LoadColor(ini, "TableBorderLight", colors[ImGuiCol_TableBorderLight]);
        LoadColor(ini, "TableRowBg", colors[ImGuiCol_TableRowBg]);
        LoadColor(ini, "TableRowBgAlt", colors[ImGuiCol_TableRowBgAlt]);
        LoadColor(ini, "TextSelectedBg", colors[ImGuiCol_TextSelectedBg]);
        LoadColor(ini, "DragDropTarget", colors[ImGuiCol_DragDropTarget]);
        LoadColor(ini, "NavHighlight", colors[ImGuiCol_NavHighlight]);
        LoadColor(ini, "NavWindowingHighlight", colors[ImGuiCol_NavWindowingHighlight]);
        LoadColor(ini, "NavWindowingDimBg", colors[ImGuiCol_NavWindowingDimBg]);
        LoadColor(ini, "ModalWindowDimBg", colors[ImGuiCol_ModalWindowDimBg]);
        return true;
    }

    static void GetIntValue(const char *value, uint8_t *target)
    {
        if (value != nullptr)
        {
            std::string_view strView(value);
            char            *pEnd{};
            uint32_t         result = std::strtoul(value, &pEnd, 10);

            if (*pEnd != 0 || result > 255)
            {
                SPDLOG_WARN("Can't get unsigned int value or value to large()");
            }
            else
            {
                *target = static_cast<uint8_t>(result);
            }
        }
    }

    void LoadColor(CSimpleIniA &ini, const char *key, ImVec4 &color)
    {
        if (const auto *value = ini.GetValue("colors", key); value != nullptr)
        {
            const std::regex base_regex("\"rgba\\((\\d+),\\s?(\\d+),\\s?(\\d+),\\s?([0,1]\\.\\d+)\\)\"");
            std::smatch      base_match;
            std::string      strValue(value);
            if (std::regex_match(strValue, base_match, base_regex))
            {
                if (base_match.size() == 5)
                {
                    uint8_t r = 0, g = 0, b = 0;
                    GetIntValue(base_match[1].str().data(), &r);
                    GetIntValue(base_match[2].str().data(), &g);
                    GetIntValue(base_match[3].str().data(), &b);
                    color.x = r / 255.0F;
                    color.y = b / 255.0F;
                    color.z = b / 255.0F;
                    GetFloatValue(base_match[4].str().data(), &color.w);
                }
            }
        }
    }
}
