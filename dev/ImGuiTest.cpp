// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
// #include "AddressLibTool.hpp"
#include "Status.h"
#include "common/toml++/toml.hpp"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "spdlog/spdlog.h"

#include <array>
#include <d3d11.h>
#include <deque>
#include <dinput.h>
#include <ranges>
#include <regex>
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
// compsition varibles
static LPDIRECTINPUT8       g_pDirectInput    = nullptr;
static LPDIRECTINPUTDEVICE8 g_pKeyboardDevice = nullptr;
static bool                 g_fShowToolWindow = false;
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

uint8_t g_keyModifiers = ImGuiKey_None;

// Forward declarations of helper functions
bool           CreateDeviceD3D(HWND hWnd);
void           CleanupDeviceD3D();
void           CreateRenderTarget();
void           CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void           OnStatusbarSize(HWND hWnd, UINT state, int cx, int cy);
void           ShowToolWindow();
void           RenderTileBarWindow();
void           RenderToolWindow();
bool           InitDirectInput() noexcept;
bool           GetState() noexcept;
auto           LoadAllThemes() -> std::vector<std::string>;
void           UseTheme(size_t themeIndex);

std::array<char, 256> keyboardState{0};

void ShowDockSpace();
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
    ImFontConfig cfg1;
    cfg1.MergeMode = true;

    ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    // cfg.MergeMode = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    const ImWchar emojiRange[] = {
        0x1F600, 0x1F64F, // Emoticons
        0x1F300, 0x1F5FF, // Symbols & Pictographs
        0x2600,  0x26FF,  // Miscellaneous Symbols
        0x2700,  0x27BF,  // Dingbats
        0                 // 终止符
    };
    ImWchar ascii_ranges[] = {
        0x0020, 0x007F, // Basic Latin
        0x00A0, 0x00FF, // Latin Supplement（可选，包含部分特殊字符）
        0               // 终止符
    };
    cfg.GlyphExcludeRanges = ascii_ranges;
    ImFont *emojiFont      = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 13.0F, &cfg, emojiRange);
    io.Fonts->AddFontFromFileTTF(
        R"(D:\assets\monaspace-v1.200\monaspace-v1.200\fonts\frozen\MonaspaceXenonFrozen-SemiWideMedium.ttf)", 13.0f,
        &cfg1, io.Fonts->GetGlyphRangesDefault());
    io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\simsun.ttc)", 13.0F, &cfg1);
    io.Fonts->Build();

    ImGui::GetMainViewport()->PlatformHandleRaw = (void *)hwnd;

    // Our state
    bool show_demo_window    = true;
    bool show_another_window = false;
    auto clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ShowToolWindow();

    bool done = false;

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

        ShowDockSpace();

        // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        {
            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            // ImGui::PushFont(emojiFont);
            ImGui::Text("\xf0\x9f\x8d\x89 \xe2\x9c\x94\xef\xb8\x8f \xe2\xad\x90");
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
            {
                ImGui::SetItemTooltip("A unicode string");
            }

            if (ImGui::Begin("Data", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("0x");
                ImGui::SameLine();
                static std::array<char, 8> formIdBuf;
                ImGui::InputText("##FormIdInput", formIdBuf.data(), formIdBuf.size(), ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsHexadecimal);

                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_RootWindow))
                {
                    static int selectedIndex = -1;
                    ImGuiID    popupId       = 0;
                    if (ImGui::BeginChild("Sidebar", {300, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
                    {
                        static std::array<char, 256> armorNameBuf;
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputTextWithHint("##ArmorNameFilter", "Filter Armor Name", armorNameBuf.data(),
                                                 armorNameBuf.size());
                        if (ImGui::BeginTable("Mod Name", 1, ImGuiTableFlags_BordersOuter))
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Selectable("Skyrim", false);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Selectable("CC", false);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Selectable("ArmorPack", false);
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Selectable("Dragonborn", false);
                            ImGui::EndTable();
                        }
                        ImGui::Button("Clear all filter");
                        ImGui::Separator();
                        popupId = ImGui::GetID("Popup Armor info");
                        if (ImGui::BeginPopup("Popup Armor info"))
                        {
                            ImGui::PushFontSize(24);
                            ImGui::Text("Armor");
                            ImGui::PopFontSize();
                            ImGui::Indent();
                            ImGui::Text("Basss & %d", selectedIndex);
                            ImGui::Text("Mod: Skyrim");
                            ImGui::Text("SlotMask: %d", 17782);
                            ImGui::EndPopup();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    {
                        bool        checked   = false;
                        const auto &screenPos = ImGui::GetCursorScreenPos();
                        ImGui::Checkbox("Head", &checked);
                        ImGui::SameLine();
                        ImGui::Checkbox("Body", &checked);
                        ImGui::SameLine();
                        ImGui::Checkbox("hands", &checked);
                        ImGui::PushFontSize(36);
                        ImGui::Text("Armor Name");
                        ImGui::PopFontSize();
                        static auto flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable |
                                            ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders;
                        ImGui::CheckboxFlags("NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
                        ImGui::CheckboxFlags("ScrollY", &flags, ImGuiTableFlags_ScrollY);
                        if (ImGui::BeginTable("Armor Name", 3, flags))
                        {
                            ImGui::TableSetupScrollFreeze(1, 1);
                            ImGui::TableSetupColumn("#Number");
                            ImGui::TableSetupColumn("Armor Name");
                            ImGui::TableSetupColumn("Mod Name");
                            ImGui::TableHeadersRow();
                            ImGuiListClipper clipper;
                            clipper.Begin(100);
                            while (clipper.Step())
                            {
                                for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
                                {
                                    ImGui::TableNextRow();
                                    ImGui::TableNextColumn();
                                    ImGui::Value("#", index);

                                    ImGui::TableNextColumn();
                                    bool isSelected = selectedIndex == index;
                                    if (ImGui::Selectable(std::format("Armor {}", index).c_str(), isSelected,
                                                          ImGuiSelectableFlags_SpanAllColumns |
                                                              ImGuiSelectableFlags_AllowOverlap))
                                    {
                                        selectedIndex = isSelected ? -1 : index;
                                        if (!isSelected)
                                        {
                                            ImGui::OpenPopup(popupId);
                                        }
                                    }

                                    ImGui::TableNextColumn();
                                    ImGui::Value("ModName", index);
                                }
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndGroup();
                }
                else
                {
                    ImGui::Text("Please hover");
                }
            }
            ImGui::End();

            ImGui::SameLine();
            ImGui::BeginGroup();

            ImGui::SameLine();
            static bool toTop = false;
            if (toTop)
            {
                auto y = ImGui::GetScrollY();
                if (y > 100)
                {
                    ImGui::SetScrollY(y - 100);
                }
                else
                {
                    toTop = false;
                    ImGui::SetScrollY(0);
                }
            }
            if (ImGui::Button("Top↑"))
            {
                toTop = true;
            }

            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SameLine();
            auto & imGuiIo = ImGui::GetIO();
            ImGui::Text("Font_Size_Scale");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat("##Font_Size_Scale", &imGuiIo.FontGlobalScale, 0.05,
                                 0.1f, 5.0f,
                                 "%.3f", ImGuiSliderFlags_NoInput);
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 50);
            ImGui::Text(R"(ImGui::Checkbox("Another Window", &show_another_window))");

            static char buf[64] = "";
            ImGui::InputText("UTF-8 input", buf, IM_ARRAYSIZE(buf));
            ImGui::End();
        }

        RenderTileBarWindow();
        // RenderToolWindow();

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
            {
                // Disable ALT application menu
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

bool TagTabButton(const char *label, bool *p_open, bool selected, const ImVec2 &size_arg = ImVec2(0, 0))
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext     &g          = *GImGui;
    const ImGuiStyle &style      = g.Style;
    const ImGuiID     id         = window->GetID(label);
    const ImVec2      label_size = ImGui::CalcTextSize(label, NULL, true);

    // 主按钮区域计算
    ImVec2 pos  = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f,
                                      label_size.y + style.FramePadding.y * 2.0f);

    // 为关闭按钮预留空间
    const float close_button_size_base = g.FontSize * 0.5f;
    const float close_button_padding   = 2.0f;
    size.x += close_button_size_base + close_button_padding + style.ItemSpacing.x;

    // 添加主按钮到布局
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    // 主按钮交互逻辑
    bool main_hovered, main_held;
    bool main_pressed = ImGui::ButtonBehavior(bb, id, &main_hovered, &main_held);

    // 绘制主按钮背景
    ImU32 bg_col = selected
                       ? ImGui::GetColorU32(ImGuiCol_TabActive)
                       : (main_hovered ? ImGui::GetColorU32(ImGuiCol_TabHovered) : ImGui::GetColorU32(ImGuiCol_Tab));
    window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_col, style.FrameRounding);

    // 绘制边框
    if (selected || main_hovered)
    {
        ImU32 border_col = selected ? ImGui::GetColorU32(ImGuiCol_Border) : ImGui::GetColorU32(ImGuiCol_TabHovered);
        window->DrawList->AddRect(bb.Min, bb.Max, border_col, style.FrameRounding, 0, 1.0f);
    }

    // 绘制标签文本
    ImVec2 text_pos = pos + style.FramePadding;
    ImGui::RenderText(text_pos, label);

    // --- 关闭按钮逻辑修复 ---
    bool close_clicked = false;
    if (p_open)
    {
        // 生成唯一 ID
        ImGuiID close_id = window->GetID(label);
        close_id         = ImHashStr("#CLOSE", 0, close_id);

        // 计算关闭按钮区域
        const ImVec2 close_button_size(close_button_size_base, close_button_size_base);
        const ImVec2 close_button_offset(bb.Max.x - close_button_size.x - style.FramePadding.x - close_button_padding,
                                         bb.Min.y + (bb.GetHeight() - close_button_size.y) * 0.5f);
        const ImRect close_bb(close_button_offset, close_button_offset + close_button_size);

        // 添加关闭按钮交互区域
        ImGui::ItemAdd(close_bb, close_id);

        // 处理关闭按钮交互
        bool close_hovered, close_held;
        bool close_pressed = ImGui::ButtonBehavior(close_bb, close_id, &close_hovered, &close_held);

        // 点击逻辑：在鼠标释放时触发关闭
        if (close_pressed && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            *p_open       = false;
            close_clicked = true;
        }

        // 绘制 "×" 图标
        const ImU32 col =
            close_hovered ? ImGui::GetColorU32(ImGuiCol_ButtonHovered) : ImGui::GetColorU32(ImGuiCol_Text);
        const float  thickness = 1.5f;
        const ImVec2 center    = close_bb.GetCenter();
        window->DrawList->AddLine(center + ImVec2(-close_button_size_base * 0.3f, -close_button_size_base * 0.3f),
                                  center + ImVec2(close_button_size_base * 0.3f, close_button_size_base * 0.3f), col,
                                  thickness);
        window->DrawList->AddLine(center + ImVec2(close_button_size_base * 0.3f, -close_button_size_base * 0.3f),
                                  center + ImVec2(-close_button_size_base * 0.3f, close_button_size_base * 0.3f), col,
                                  thickness);
    }

    return main_pressed && !close_clicked;
}

void RenderTileBarWindow()
{
    if (ImGui::BeginCombo("Combo1", "combo1 v1", ImGuiComboFlags_HeightRegular))
    {
        ImGui::Selectable("combo1 v2");
        ImGui::Selectable("combo1 v3");
        ImGui::Selectable("combo1 v2");
        ImGui::Selectable("combo1 v3");
        ImGui::Selectable("combo1 v2");
        ImGui::Selectable("combo1 v3");
        ImGui::EndCombo();
    }
    // 在 ImGui 窗口中调用
    static bool tag1_open    = true;
    static bool tag2_open    = true;
    static int  selected_tag = 0;
    // 标签1
    if (tag1_open)
    {
        if (TagTabButton("Document 1", &tag1_open, selected_tag == 0))
        {
            selected_tag = 0;
        }
        ImGui::SameLine();
    }
    // 标签2
    if (tag2_open)
    {
        if (TagTabButton("Settings", &tag2_open, selected_tag == 1))
        {
            selected_tag = 1;
        }
        ImGui::SameLine();
    }
}

void RenderToolWindow()
{
    if (!g_fShowToolWindow)
    {
        return;
    }

    ImGui::Begin("DevImeToolWindowTest", &g_fShowToolWindow, g_toolWindowFlags);

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

    ImGui::SameLine();
    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders;

    static ImGuiSelectionBasicStorage storge;
    auto *msIo = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_BoxSelect1d, storge.Size, fakeLangProfiles.size());
    storge.ApplyRequests(msIo);
    if (ImGui::BeginTable("###InstalledIME", 3, flags))
    {
        ImGui::TableSetupColumn("Order");
        ImGui::TableSetupColumn("IME");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();

        uint8_t idx = 0;
        for (const std::string_view langProfile : fakeLangProfiles)
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("1st column");

            ImGui::TableNextColumn();
            bool isSelected = storge.Contains(idx);
            ImGui::SetNextItemSelectionUserData(idx);
            ImGui::Selectable(langProfile.data(), isSelected, ImGuiSelectableFlags_SpanAllColumns);

            ImGui::TableNextColumn();
            ImGui::Text("3rd column");
            idx++;
        }
        ImGui::EndTable();
    }
    msIo = ImGui::EndMultiSelect();
    storge.ApplyRequests(msIo);

    ImGui::Value("Select count", storge.Size);
    for (uint8_t i = 0; i < fakeLangProfiles.size(); ++i)
    {
        if (storge.Contains(i))
        {
            ImGui::Text("Selected: %s", fakeLangProfiles.at(i).data());
        }
    }

    // if (!m_fPinToolWindow && !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    // {
    //     ShowToolWindow();
    // }
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

static void GetFloatValue(const char *value, float *target)
{
    if (std::string strValue(value); !strValue.empty())
    {
        char       *pEnd{};
        const float result = std::strtof(value, &pEnd);
        if (*pEnd == 0)
        {
            *target = result;
        }
    }
}

static void GetVec2Value(const char *value, ImVec2 &target)
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

static void GetEnumDirValue(const char *value, ImGuiDir &dir)
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

void GetIntValue(const char *value, uint8_t *target)
{
    if (value != nullptr)
    {
        char    *pEnd{};
        uint32_t result = std::strtoul(value, &pEnd, 10);

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

void themeColorSetup(const char *colorString, ImVec4 &color)
{
    std::string strValue(colorString);
    if (strValue.empty())
    {
        return;
    }
    const std::regex base_regex("rgba\\((\\d+),\\s?(\\d+),\\s?(\\d+),\\s?([0,1]\\.\\d+)\\)");
    std::smatch      base_match;
    if (std::regex_match(strValue, base_match, base_regex))
    {
        if (base_match.size() == 5)
        {
            uint8_t r = 0, g = 0, b = 0;
            GetIntValue(base_match[1].str().data(), &r);
            GetIntValue(base_match[2].str().data(), &g);
            GetIntValue(base_match[3].str().data(), &b);
            color.x = r / 255.0F;
            color.y = g / 255.0F;
            color.z = b / 255.0F;
            GetFloatValue(base_match[4].str().data(), &color.w);
            spdlog::info("Color {} ==> {}, {}, {}, {}", strValue, color.x, color.y, color.z, color.w);
        }
    }
    else
    {
    }
}

std::vector<std::string> LoadAllThemes()
{
    auto config = toml::parse_file(R"(D:\repo\JamieMods\common\themes.toml.txt)");

    // get key-value pairs
    const auto              &themesArray = config["themes"].as_array();
    std::vector<std::string> themes;
    for (const toml::node &themeNode : *themesArray)
    {
        auto        theme = *themeNode.as_table();
        std::string name  = theme["name"].value_or("");
        if (!name.empty())
        {
            themes.emplace_back(std::move(name));
        }
    }
    return themes;
}

void UseTheme(size_t index)
{
    auto        config      = toml::parse_file(R"(D:\repo\JamieMods\common\themes.toml.txt)");
    const auto &themesArray = config["themes"].as_array();

    toml::node &theme_node  = themesArray->at(index);
    toml::table theme_table = *theme_node.as_table();
    spdlog::info("Select theme is {}", theme_table["name"].value_or(""));

    auto &style      = ImGui::GetStyle();
    auto  style_node = theme_table["style"];
    GetFloatValue(style_node["Alpha"].value_or(""), &style.Alpha);
    GetFloatValue(style_node["DisabledAlpha"].value_or(""), &style.DisabledAlpha);
    GetVec2Value(style_node["WindowPadding"].value_or(""), style.WindowPadding);
    GetFloatValue(style_node["WindowRounding"].value_or(""), &style.WindowRounding);
    GetFloatValue(style_node["WindowBorderSize"].value_or(""), &style.WindowBorderSize);
    GetFloatValue(style_node["WindowBorderHoverPadding"].value_or(""), &style.WindowBorderHoverPadding);
    GetVec2Value(style_node["WindowMinSize"].value_or(""), style.WindowMinSize);
    GetVec2Value(style_node["WindowTitleAlign"].value_or(""), style.WindowTitleAlign);
    GetFloatValue(style_node["ChildRounding"].value_or(""), &style.ChildRounding);
    GetFloatValue(style_node["ChildBorderSize"].value_or(""), &style.ChildBorderSize);
    GetFloatValue(style_node["PopupRounding"].value_or(""), &style.PopupRounding);
    GetFloatValue(style_node["PopupBorderSize"].value_or(""), &style.PopupBorderSize);
    GetVec2Value(style_node["FramePadding"].value_or(""), style.FramePadding);
    GetFloatValue(style_node["FrameRounding"].value_or(""), &style.FrameRounding);
    GetFloatValue(style_node["FrameBorderSize"].value_or(""), &style.FrameBorderSize);
    GetVec2Value(style_node["ItemSpacing"].value_or(""), style.ItemSpacing);
    GetVec2Value(style_node["ItemInnerSpacing"].value_or(""), style.ItemInnerSpacing);
    GetVec2Value(style_node["CellPadding"].value_or(""), style.CellPadding);
    GetVec2Value(style_node["TouchExtraPadding"].value_or(""), style.TouchExtraPadding);
    GetFloatValue(style_node["IndentSpacing"].value_or(""), &style.IndentSpacing);
    GetFloatValue(style_node["ColumnsMinSpacing"].value_or(""), &style.ColumnsMinSpacing);
    GetFloatValue(style_node["ScrollbarSize"].value_or(""), &style.ScrollbarSize);
    GetFloatValue(style_node["ScrollbarRounding"].value_or(""), &style.ScrollbarRounding);
    GetFloatValue(style_node["GrabMinSize"].value_or(""), &style.GrabMinSize);
    GetFloatValue(style_node["GrabRounding"].value_or(""), &style.GrabRounding);
    GetFloatValue(style_node["LogSliderDeadzone"].value_or(""), &style.LogSliderDeadzone);
    GetFloatValue(style_node["ImageBorderSize"].value_or(""), &style.ImageBorderSize);
    GetFloatValue(style_node["TabRounding"].value_or(""), &style.TabRounding);
    GetFloatValue(style_node["TabBorderSize"].value_or(""), &style.TabBorderSize);
    GetFloatValue(style_node["TabCloseButtonMinWidthSelected"].value_or(""), &style.TabCloseButtonMinWidthSelected);
    GetFloatValue(style_node["TabCloseButtonMinWidthUnselected"].value_or(""), &style.TabCloseButtonMinWidthUnselected);
    GetFloatValue(style_node["TabBarBorderSize"].value_or(""), &style.TabBarBorderSize);
    GetFloatValue(style_node["TabBarOverlineSize"].value_or(""), &style.TabBarOverlineSize);
    GetFloatValue(style_node["TableAngledHeadersAngle"].value_or(""), &style.TableAngledHeadersAngle);
    GetEnumDirValue(style_node["WindowMenuButtonPosition"].value_or(""), style.WindowMenuButtonPosition);
    GetVec2Value(style_node["TableAngledHeadersTextAlign"].value_or(""), style.TableAngledHeadersTextAlign);
    GetEnumDirValue(style_node["ColorButtonPosition"].value_or(""), style.ColorButtonPosition);
    GetVec2Value(style_node["ButtonTextAlign"].value_or(""), style.ButtonTextAlign);
    GetVec2Value(style_node["SelectableTextAlign"].value_or(""), style.SelectableTextAlign);
    GetFloatValue(style_node["SeparatorTextBorderSize"].value_or(""), &style.SeparatorTextBorderSize);
    GetVec2Value(style_node["SeparatorTextAlign"].value_or(""), style.SeparatorTextAlign);
    GetVec2Value(style_node["SeparatorTextPadding"].value_or(""), style.SeparatorTextPadding);
    GetVec2Value(style_node["DisplayWindowPadding"].value_or(""), style.DisplayWindowPadding);
    GetVec2Value(style_node["DisplaySafeAreaPadding"].value_or(""), style.DisplaySafeAreaPadding);
    GetFloatValue(style_node["DockingSeparatorSize"].value_or(""), &style.DockingSeparatorSize);
    GetFloatValue(style_node["MouseCursorScale"].value_or(""), &style.MouseCursorScale);

    // bool        AntiAliasedLines;           // Enable anti-aliased lines/borders. Disable if you are really tight
    // on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList). bool        AntiAliasedLinesUseTex;
    // // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear
    // filtering (NOT point/nearest filtering). Latched at the beginning of the frame (copied to ImDrawList). bool
    // AntiAliasedFill;            // Enable anti-aliased edges around filled shapes (rounded rectangles, circles,
    // etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to
    // ImDrawList).
    GetFloatValue(style_node["CurveTessellationTol"].value_or(""), &style.CurveTessellationTol);
    GetFloatValue(style_node["CircleTessellationMaxError"].value_or(""), &style.CircleTessellationMaxError);

    auto colors_node = theme_table["style"]["colors"];
    themeColorSetup(colors_node["Text"].value_or(""), style.Colors[ImGuiCol_Text]);
    themeColorSetup(colors_node["TextDisabled"].value_or(""), style.Colors[ImGuiCol_TextDisabled]);
    themeColorSetup(colors_node["WindowBg"].value_or(""), style.Colors[ImGuiCol_WindowBg]);
    themeColorSetup(colors_node["ChildBg"].value_or(""), style.Colors[ImGuiCol_ChildBg]);
    themeColorSetup(colors_node["PopupBg"].value_or(""), style.Colors[ImGuiCol_PopupBg]);
    themeColorSetup(colors_node["Border"].value_or(""), style.Colors[ImGuiCol_Border]);
    themeColorSetup(colors_node["BorderShadow"].value_or(""), style.Colors[ImGuiCol_BorderShadow]);
    themeColorSetup(colors_node["FrameBg"].value_or(""), style.Colors[ImGuiCol_FrameBg]);
    themeColorSetup(colors_node["FrameBgHovered"].value_or(""), style.Colors[ImGuiCol_FrameBgHovered]);
    themeColorSetup(colors_node["FrameBgActive"].value_or(""), style.Colors[ImGuiCol_FrameBgActive]);
    themeColorSetup(colors_node["TitleBg"].value_or(""), style.Colors[ImGuiCol_TitleBg]);
    themeColorSetup(colors_node["TitleBgActive"].value_or(""), style.Colors[ImGuiCol_TitleBgActive]);
    themeColorSetup(colors_node["TitleBgCollapsed"].value_or(""), style.Colors[ImGuiCol_TitleBgCollapsed]);
    themeColorSetup(colors_node["MenuBarBg"].value_or(""), style.Colors[ImGuiCol_MenuBarBg]);
    themeColorSetup(colors_node["ScrollbarBg"].value_or(""), style.Colors[ImGuiCol_ScrollbarBg]);
    themeColorSetup(colors_node["ScrollbarGrab"].value_or(""), style.Colors[ImGuiCol_ScrollbarGrab]);
    themeColorSetup(colors_node["ScrollbarGrabHovered"].value_or(""), style.Colors[ImGuiCol_ScrollbarGrabHovered]);
    themeColorSetup(colors_node["ScrollbarGrabActive"].value_or(""), style.Colors[ImGuiCol_ScrollbarGrabActive]);
    themeColorSetup(colors_node["CheckMark"].value_or(""), style.Colors[ImGuiCol_CheckMark]);
    themeColorSetup(colors_node["SliderGrab"].value_or(""), style.Colors[ImGuiCol_SliderGrab]);
    themeColorSetup(colors_node["SliderGrabActive"].value_or(""), style.Colors[ImGuiCol_SliderGrabActive]);
    themeColorSetup(colors_node["Button"].value_or(""), style.Colors[ImGuiCol_Button]);
    themeColorSetup(colors_node["ButtonHovered"].value_or(""), style.Colors[ImGuiCol_ButtonHovered]);
    themeColorSetup(colors_node["ButtonActive"].value_or(""), style.Colors[ImGuiCol_ButtonActive]);
    themeColorSetup(colors_node["Header"].value_or(""), style.Colors[ImGuiCol_Header]);
    themeColorSetup(colors_node["HeaderHovered"].value_or(""), style.Colors[ImGuiCol_HeaderHovered]);
    themeColorSetup(colors_node["HeaderActive"].value_or(""), style.Colors[ImGuiCol_HeaderActive]);
    themeColorSetup(colors_node["Separator"].value_or(""), style.Colors[ImGuiCol_Separator]);
    themeColorSetup(colors_node["SeparatorHovered"].value_or(""), style.Colors[ImGuiCol_SeparatorHovered]);
    themeColorSetup(colors_node["SeparatorActive"].value_or(""), style.Colors[ImGuiCol_SeparatorActive]);
    themeColorSetup(colors_node["ResizeGrip"].value_or(""), style.Colors[ImGuiCol_ResizeGrip]);
    themeColorSetup(colors_node["ResizeGripHovered"].value_or(""), style.Colors[ImGuiCol_ResizeGripHovered]);
    themeColorSetup(colors_node["ResizeGripActive"].value_or(""), style.Colors[ImGuiCol_ResizeGripActive]);
    themeColorSetup(colors_node["InputTextCursor"].value_or(""), style.Colors[ImGuiCol_InputTextCursor]);
    themeColorSetup(colors_node["TabHovered"].value_or(""), style.Colors[ImGuiCol_TabHovered]);
    themeColorSetup(colors_node["Tab"].value_or(""), style.Colors[ImGuiCol_Tab]);
    themeColorSetup(colors_node["TabSelected"].value_or(""), style.Colors[ImGuiCol_TabSelected]);
    themeColorSetup(colors_node["TabSelectedOverline"].value_or(""), style.Colors[ImGuiCol_TabSelectedOverline]);
    themeColorSetup(colors_node["TabDimmed"].value_or(""), style.Colors[ImGuiCol_TabDimmed]);
    themeColorSetup(colors_node["TabDimmedSelected"].value_or(""), style.Colors[ImGuiCol_TabDimmedSelected]);
    themeColorSetup(colors_node["TabDimmedSelectedOverline"].value_or(""),
                    style.Colors[ImGuiCol_TabDimmedSelectedOverline]);
    themeColorSetup(colors_node["DockingPreview"].value_or(""), style.Colors[ImGuiCol_DockingPreview]);
    themeColorSetup(colors_node["DockingEmptyBg"].value_or(""), style.Colors[ImGuiCol_DockingEmptyBg]);
    themeColorSetup(colors_node["PlotLines"].value_or(""), style.Colors[ImGuiCol_PlotLines]);
    themeColorSetup(colors_node["PlotLinesHovered"].value_or(""), style.Colors[ImGuiCol_PlotLinesHovered]);
    themeColorSetup(colors_node["PlotHistogram"].value_or(""), style.Colors[ImGuiCol_PlotHistogram]);
    themeColorSetup(colors_node["PlotHistogramHovered"].value_or(""), style.Colors[ImGuiCol_PlotHistogramHovered]);
    themeColorSetup(colors_node["TableHeaderBg"].value_or(""), style.Colors[ImGuiCol_TableHeaderBg]);
    themeColorSetup(colors_node["TableBorderStrong"].value_or(""), style.Colors[ImGuiCol_TableBorderStrong]);
    themeColorSetup(colors_node["TableBorderLight"].value_or(""), style.Colors[ImGuiCol_TableBorderLight]);
    themeColorSetup(colors_node["TableRowBg"].value_or(""), style.Colors[ImGuiCol_TableRowBg]);
    themeColorSetup(colors_node["TableRowBgAlt"].value_or(""), style.Colors[ImGuiCol_TableRowBgAlt]);
    themeColorSetup(colors_node["TextLink"].value_or(""), style.Colors[ImGuiCol_TextLink]);
    themeColorSetup(colors_node["TextSelectedBg"].value_or(""), style.Colors[ImGuiCol_TextSelectedBg]);
    themeColorSetup(colors_node["DragDropTarget"].value_or(""), style.Colors[ImGuiCol_DragDropTarget]);
    themeColorSetup(colors_node["NavCursor"].value_or(""), style.Colors[ImGuiCol_NavCursor]);
    themeColorSetup(colors_node["NavWindowingHighlight"].value_or(""), style.Colors[ImGuiCol_NavWindowingHighlight]);
    themeColorSetup(colors_node["NavWindowingDimBg"].value_or(""), style.Colors[ImGuiCol_NavWindowingDimBg]);
    themeColorSetup(colors_node["ModalWindowDimBg"].value_or(""), style.Colors[ImGuiCol_ModalWindowDimBg]);
}

void ShowDockSpace()
{
    static bool               opt_fullscreen  = true;
    static bool               opt_padding     = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    static bool showDockSpace = false;
    ImGui::Begin("DockSpace Demo", &showDockSpace, window_flags);
    if (!opt_padding) ImGui::PopStyleVar();

    if (opt_fullscreen) ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    else
    {
        ImGui::Text("ERROR: Docking is not enabled! See Demo > Configuration.");
        ImGui::Text("Set io.ConfigFlags |= ImGuiConfigFlags_DockingEnable in your code, or ");
        ImGui::SameLine(0.0f, 0.0f);
        if (ImGui::SmallButton("click here")) io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::MenuItem("Close"))
        {
            exit(0);
        }
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "",
                                (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode;
            }
            if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit;
            }
            if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking;
            }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
            }
            if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
            }
            if (ImGui::MenuItem("Flag: PassthruCentralNode", "",
                                (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen))
            {
                dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, true)) showDockSpace = false;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}

}