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
#include "common/WCharUtils.h"
#include "common/toml++/toml.hpp"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "spdlog/spdlog.h"

#include <array>
#include <atlcomcli.h>
#include <d3d11.h>
#include <dinput.h>
#include <dwrite.h>
#include <ranges>
#include <regex>
#include <string>
#include <windowsx.h>

#pragma comment(lib, "dwrite.lib")
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

struct SystemFontFamily
{
    std::string                familyName;
    CComPtr<IDWriteFontFamily> pFontFamily = nullptr;

    struct FontProperty
    {
        DWRITE_FONT_WEIGHT      weight;
        DWRITE_FONT_STYLE       style;
        DWRITE_FONT_STRETCH     stretch;
        DWRITE_FONT_SIMULATIONS simulations;
        uint32_t                faceIndex = -1;

        FontProperty(
            const DWRITE_FONT_WEIGHT weight, const DWRITE_FONT_STYLE style, const DWRITE_FONT_STRETCH stretch,
            const DWRITE_FONT_SIMULATIONS simulations, uint32_t faceIndex
        )
            : weight(weight), style(style), stretch(stretch), simulations(simulations), faceIndex(faceIndex)
        {
        }
    };

    std::vector<FontProperty> fontProperties;
    uint32_t                  numFaces = 1;

    SystemFontFamily(const std::string &familyName, const CComPtr<IDWriteFontFamily> &pFontFamily)
        : familyName(familyName), pFontFamily(pFontFamily)
    {
    }
};

// Forward declarations of helper functions
bool           CreateDeviceD3D(HWND hWnd);
void           CleanupDeviceD3D();
void           CreateRenderTarget();
void           CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void           OnStatusbarSize(HWND hWnd, UINT state, int cx, int cy);
void           RenderTileBarWindow();
bool           InitDirectInput() noexcept;
bool           GetState() noexcept;

struct FontManager
{
    std::vector<SystemFontFamily> fontFamilies;
    std::string                   defaultFontPath;
    ImFont                       *emojiFont          = nullptr;
    ImFont                       *previewFont        = nullptr;
    bool                          open               = false;
    bool                          requestReplaceFont = false;

    void Init();
    void Show();
    void ApplyFont();

private:
    static auto GetFontFilePath(IDWriteFont *font) -> std::string;
    static auto GetLocalizedString(IDWriteLocalizedStrings *pStrings) -> std::string;
};

std::array<char, 256> keyboardState{0};
}

static HINSTANCE winst;
static HWND      parentWnd;

ImFont *ApplyFont(const std::string &fontName);

// Main code
int main(int, char **)
{
    // AddressLibTool::DumpSpecialVerAddress(5, 97);
    // return 0;

    // Create application window
    // ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = {
        sizeof(wc),
        CS_CLASSDC,
        WndProc,
        0L,
        0L,
        GetModuleHandle(nullptr),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        L"ImGui Example",
        nullptr
    };
    ::RegisterClassExW(&wc);
    //    ::RegisterClassExW(&wchild);
    winst     = wc.hInstance;
    HWND hwnd = ::CreateWindowW(
        wc.lpszClassName,
        L"Dear ImGui DirectX11 Example",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        1280,
        800,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );
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

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImGui::StyleColorsDark();

    ImGui::GetMainViewport()->PlatformHandleRaw = (void *)hwnd;

    // Our state
    bool show_demo_window    = true;
    bool show_another_window = false;
    auto clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    FontManager fontManager;
    fontManager.Init();

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

        if (fontManager.requestReplaceFont)
        {
            fontManager.requestReplaceFont = false;
            io.Fonts->RemoveFont(fontManager.emojiFont);
            io.FontDefault          = std::move(fontManager.previewFont);
            fontManager.emojiFont   = io.FontDefault;
            fontManager.previewFont = nullptr;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        {
            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            fontManager.Show();

            if (show_another_window)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
                if (ImGui::Begin(
                        "##Data",
                        &show_another_window,
                        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration |
                            ImGuiWindowFlags_UnsavedDocument
                    ))
                {

                    static int  selectedIndex  = -1;
                    static bool ShowTableChild = true;
                    ImGuiID     popupId        = 0;
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 10.0f});
                    ImGuiChildFlags TableChildFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX;
                    if (ImGui::BeginChild("Sidebar", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX))
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                        ImGui::Button("🥼");
                        ImGui::Dummy(ImVec2{1, ImGui::GetFontSize() * 0.5f});
                        if (ImGui::Button("📁"))
                        {
                            ShowTableChild = !ShowTableChild;
                        }
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndChild();

                    ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0);
                    ImGui::SameLine();
                    ImGui::PopStyleVar(2);

                    // ImGui::PushStyleVarX(ImGuiStyleVar_WindowMinSize, 0);
                    if (ShowTableChild)
                    {
                        if (ImGui::BeginChild("TableChild", {300, 500}, TableChildFlags))
                        {
                            static auto flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable |
                                                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable |
                                                ImGuiTableFlags_Borders;
                            ImGui::CheckboxFlags("NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
                            ImGui::CheckboxFlags("ScrollY", &flags, ImGuiTableFlags_ScrollY);
                            ImGui::CheckboxFlags(
                                "NoPadding", &flags, ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX
                            );
                            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 15)); // 水平4px，垂直2px
                            if (ImGui::BeginTable("Armor Name", 3, flags))
                            {
                                ImGui::TableSetupScrollFreeze(1, 1);
                                ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                                ImGui::TableSetColumnIndex(0);

                                ImGui::PushFontSize(20);
                                ImGui::Text("Col1");
                                ImGui::SameLine();
                                ImGui::TableHeader("Col0");

                                ImGui::TableSetColumnIndex(1);
                                ImGui::SetItemTooltip("%s", "$SosGui_CreateOutfit");
                                ImGui::TableHeader("Col1");

                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("Col1");
                                ImGui::SameLine();
                                ImGui::TableHeader("Col2");
                                ImGui::PopFontSize();
                                ImGuiListClipper clipper;
                                clipper.Begin(10);
                                while (clipper.Step())
                                {
                                    for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index)
                                    {
                                        ImGui::TableNextRow(0, ImGui::GetFontSize() + 10);
                                        ImGui::TableNextColumn();
                                        ImGui::Value("#", index);

                                        ImGui::TableNextColumn();
                                        bool isSelected = selectedIndex == index;
                                        // ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
                                        if (ImGui::Selectable(
                                                std::format("SetCursorPosX {}", index).c_str(),
                                                isSelected,
                                                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap
                                            ))
                                        {
                                            selectedIndex = isSelected ? -1 : index;
                                            if (!isSelected)
                                            {
                                                ImGui::OpenPopup(popupId);
                                            }
                                        }

                                        ImGui::TableNextColumn();
                                        // ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
                                        ImGui::Value("ModName", index);
                                    }
                                }
                                ImGui::EndTable();
                            }
                            ImGui::PopStyleVar();
                        }
                        ImGui::EndChild();
                    }
                    // ImGui::PopStyleVar();

                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    {
                    }
                    ImGui::EndGroup();
                }
                ImGui::End();
                ImGui::PopStyleVar();
            }

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
        const float clear_color_with_alpha[4] = {
            clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w
        };
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
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
        &g_pd3dDeviceContext
    );
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(
            nullptr,
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
            &g_pd3dDeviceContext
        );
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
    ImVec2 size = ImGui::CalcItemSize(
        size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f
    );

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
                       ? ImGui::GetColorU32(ImGuiCol_TabSelected)
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
        const ImVec2 close_button_offset(
            bb.Max.x - close_button_size.x - style.FramePadding.x - close_button_padding,
            bb.Min.y + (bb.GetHeight() - close_button_size.y) * 0.5f
        );
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
        window->DrawList->AddLine(
            center + ImVec2(-close_button_size_base * 0.3f, -close_button_size_base * 0.3f),
            center + ImVec2(close_button_size_base * 0.3f, close_button_size_base * 0.3f),
            col,
            thickness
        );
        window->DrawList->AddLine(
            center + ImVec2(close_button_size_base * 0.3f, -close_button_size_base * 0.3f),
            center + ImVec2(-close_button_size_base * 0.3f, close_button_size_base * 0.3f),
            col,
            thickness
        );
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

    static std::array<std::string_view, 5> fakeLangProfiles = {
        "ENG", "SouGou", "Microsoft Pinyin", "Google Pinyin", "Baidu"
    };

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

bool InitDirectInput() noexcept
{
    if (FAILED(DirectInput8Create(
            GetModuleHandle(nullptr),
            DIRECTINPUT_VERSION,
            IID_IDirectInput8,
            reinterpret_cast<void **>(&g_pDirectInput),
            nullptr
        )))
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
    themeColorSetup(
        colors_node["TabDimmedSelectedOverline"].value_or(""), style.Colors[ImGuiCol_TabDimmedSelectedOverline]
    );
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

void FontManager::Init()
{
    ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

    ImWchar ascii_ranges[] = {0x0020, 0x00FF, 0};
    cfg.GlyphExcludeRanges = ascii_ranges;

    auto &io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    if (fontFamilies.size() > 0) return;
    CComPtr<IDWriteFactory> pDWriteFactory = nullptr;
    HRESULT                 hr             = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&pDWriteFactory)
    );

    CComPtr<IDWriteFontCollection> pFontCollection = nullptr;

    // Get the system font collection.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory->GetSystemFontCollection(&pFontCollection);
    }

    NONCLIENTMETRICS ncm = {};
    ncm.cbSize           = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    CComPtr<IDWriteGdiInterop> pGdiInterop = nullptr;
    if (SUCCEEDED(pDWriteFactory->GetGdiInterop(&pGdiInterop)))
    {
        CComPtr<IDWriteFont> pFont = nullptr;
        pGdiInterop->CreateFontFromLOGFONT(&ncm.lfMessageFont, &pFont);
        std::string filePath = GetFontFilePath(pFont);

        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 16.0, &cfg);
        ImFontConfig cfg1;
        cfg1.MergeMode  = true;
        defaultFontPath = std::move(filePath);
        emojiFont       = io.Fonts->AddFontFromFileTTF(defaultFontPath.c_str(), 16.0, &cfg1);
        io.FontDefault  = emojiFont;
    }

    UINT32 familyCount = 0;

    // Get the number of font families in the collection.
    if (SUCCEEDED(hr))
    {
        familyCount = pFontCollection->GetFontFamilyCount();
    }

    fontFamilies.reserve(familyCount);
    for (UINT32 i = 0; i < familyCount; ++i)
    {
        CComPtr<IDWriteFontFamily> pFontFamily = nullptr;
        if (hr = pFontCollection->GetFontFamily(i, &pFontFamily); SUCCEEDED(hr))
        {
            CComPtr<IDWriteLocalizedStrings> pFamilyNames = nullptr;

            // Get a list of localized strings for the family name.
            if (hr = pFontFamily->GetFamilyNames(&pFamilyNames); FAILED(hr))
            {
                continue;
            }
            auto name = GetLocalizedString(pFamilyNames);
            if (name.empty()) continue;

            SystemFontFamily systemFontFamily{name, pFontFamily};
            for (uint32_t fontIndex = 0; fontIndex < pFontFamily->GetFontCount(); ++fontIndex)
            {
                CComPtr<IDWriteFont> pFont = nullptr;
                if (SUCCEEDED(pFontFamily->GetFont(fontIndex, &pFont)))
                {
                    uint32_t                 faceIndex = -1;
                    CComPtr<IDWriteFontFace> pFontFace = nullptr;
                    if (SUCCEEDED(pFont->CreateFontFace(&pFontFace)))
                    {
                        faceIndex = pFontFace->GetIndex();
                        BOOL                  isSupportedFontType;
                        DWRITE_FONT_FILE_TYPE fontFileType;
                        DWRITE_FONT_FACE_TYPE fontFaceType;
                        UINT32                numberOfFace;

                        UINT32 numFiles = 0;
                        pFontFace->GetFiles(&numFiles, nullptr);
                        std::vector<IDWriteFontFile *> fontFiles(numFiles);
                        pFontFace->GetFiles(&numFiles, fontFiles.data());
                        IDWriteFontFile *fontFile = fontFiles[0];

                        if (SUCCEEDED(
                                fontFile->Analyze(&isSupportedFontType, &fontFileType, &fontFaceType, &numberOfFace)
                            ))
                        {
                            systemFontFamily.numFaces = numberOfFace;
                        }
                    }
                    systemFontFamily.fontProperties.emplace_back(
                        pFont->GetWeight(), pFont->GetStyle(), pFont->GetStretch(), pFont->GetSimulations(), faceIndex
                    );
                }
            }
            fontFamilies.push_back(systemFontFamily);
        }
    }
}

void FontManager::Show()
{
    auto &io = ImGui::GetIO();
    ImGui::Checkbox("Show Font Selector", &open);

    ImGui::DragFloat("##Font_Size_Scale", &io.FontGlobalScale, 0.05, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_NoInput);

    auto *viewport = ImGui::GetMainViewport();
    bool  is_open  = ImGui::BeginViewportSideBar(
        "##MainMenuBar",
        viewport,
        ImGuiDir_Left,
        30,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar
    );
    if (is_open)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Views");
                ImGui::MenuItem("Settings");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();

    if (!open) return;
    ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 20);
    int pushCount = 1;
    if (ImGui::Begin("FontSelector", &open, ImGuiWindowFlags_MenuBar))
    {
        ImGui::PopStyleVar(pushCount--);

        bool               rebuildFont       = false;
        static int         selectedProperty  = 0;
        static int         selectedFontIndex = -1;
        static std::string prevFontPath      = R"(C:\WINDOWS\Fonts\msyh.ttc)";

        auto preview = "";
        if (selectedFontIndex >= 0)
        {
            preview = fontFamilies[selectedFontIndex].familyName.c_str();
        }
        ImGui::BeginGroup();
        if (ImGui::BeginCombo("Font list", preview))
        {
            int index = 0;
            for (const auto &fontFamily : fontFamilies)
            {
                bool selected = index == selectedFontIndex;
                if (ImGui::Selectable(
                        std::format("{} face {}", fontFamily.familyName, fontFamily.numFaces).c_str(), selected
                    ) &&
                    !selected)
                {
                    selectedFontIndex = index;
                    selectedProperty  = 0;
                    rebuildFont       = true;
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
                index++;
            }
            ImGui::EndCombo();
        }
        if (selectedFontIndex >= 0)
        {
            auto &fontFamily = fontFamilies[selectedFontIndex];

            constexpr auto weightTOString = [](DWRITE_FONT_WEIGHT a_weight) {
                switch (a_weight)
                {
                    case DWRITE_FONT_WEIGHT_THIN:
                        return "Thin";
                    case DWRITE_FONT_WEIGHT_EXTRA_LIGHT: // DWRITE_FONT_WEIGHT_ULTRA_LIGHT
                        return "Extra light";
                    case DWRITE_FONT_WEIGHT_LIGHT:
                        return "Light";
                    case DWRITE_FONT_WEIGHT_SEMI_LIGHT:
                        return "Semi-light";
                    case DWRITE_FONT_WEIGHT_NORMAL: // DWRITE_FONT_WEIGHT_REGULAR
                        return "Normal";
                    case DWRITE_FONT_WEIGHT_MEDIUM:
                        return "Medium";
                    case DWRITE_FONT_WEIGHT_DEMI_BOLD: // DWRITE_FONT_WEIGHT_SEMI_BOLD
                        return "DemiBold";
                    case DWRITE_FONT_WEIGHT_BOLD:
                        return "Bold";
                    case DWRITE_FONT_WEIGHT_EXTRA_BOLD: // DWRITE_FONT_WEIGHT_ULTRA_BOLD
                        return "Extra bold";
                    case DWRITE_FONT_WEIGHT_BLACK: // DWRITE_FONT_WEIGHT_HEAVY
                        return "Black";
                    case DWRITE_FONT_WEIGHT_EXTRA_BLACK: // DWRITE_FONT_WEIGHT_ULTRA_BLACK
                        return "Extra black";
                };
                return "Unknown";
            };
            constexpr auto styleToString = [](DWRITE_FONT_STYLE a_style) {
                switch (a_style)
                {
                    case DWRITE_FONT_STYLE_NORMAL:
                        return "Normal";
                    case DWRITE_FONT_STYLE_OBLIQUE:
                        return "Oblique";
                    case DWRITE_FONT_STYLE_ITALIC:
                        return "Italic";
                }
                return "Unknown";
            };
            constexpr auto simulationsToString = [](DWRITE_FONT_SIMULATIONS aSimulations) {
                switch (aSimulations)
                {
                    case DWRITE_FONT_SIMULATIONS_NONE:
                        return "None";
                    case DWRITE_FONT_SIMULATIONS_BOLD:
                        return "Bold";
                    case DWRITE_FONT_SIMULATIONS_OBLIQUE:
                        return "Oblique";
                }
                return "Unknown";
            };
            constexpr auto stretchToString = [](DWRITE_FONT_STRETCH aStretch) {
                switch (aStretch)
                {
                    case DWRITE_FONT_STRETCH_UNDEFINED:
                        return "Undefined";
                    case DWRITE_FONT_STRETCH_ULTRA_CONDENSED:
                        return "Ultra-condensed";
                    case DWRITE_FONT_STRETCH_EXTRA_CONDENSED:
                        return "Extra-condensed";
                    case DWRITE_FONT_STRETCH_CONDENSED:
                        return "Condensed";
                    case DWRITE_FONT_STRETCH_SEMI_CONDENSED:
                        return "Semi-condensed";
                    case DWRITE_FONT_STRETCH_NORMAL:
                        return "Normal";
                    case DWRITE_FONT_STRETCH_SEMI_EXPANDED:
                        return "Semi-expanded";
                    case DWRITE_FONT_STRETCH_EXPANDED:
                        return "Expanded";
                    case DWRITE_FONT_STRETCH_EXTRA_EXPANDED:
                        return "Extra-expanded";
                    case DWRITE_FONT_STRETCH_ULTRA_EXPANDED:
                        return "Ultra-expanded";
                }
                return "Unknown";
            };

            int   index = 0;
            auto &prop  = fontFamily.fontProperties[selectedProperty];
            ;
            if (ImGui::BeginCombo(
                    "Font Property",
                    std::format(
                        "weight {}, style: {}, stretch {}, simulation {}",
                        weightTOString(prop.weight),
                        styleToString(prop.style),
                        stretchToString(prop.stretch),
                        simulationsToString(prop.simulations)
                    )
                        .c_str()
                ))
            {
                for (const auto &property : fontFamily.fontProperties)
                {
                    ImGui::PushID(index);
                    if (ImGui::Selectable(std::format(
                                              "weight {}, style: {}, stretch {}, simulations {}, face {}",
                                              weightTOString(property.weight),
                                              styleToString(property.style),
                                              stretchToString(property.stretch),
                                              simulationsToString(property.simulations),
                                              property.faceIndex
                        )
                                              .c_str()))
                    {
                        selectedProperty = index;
                        rebuildFont      = true;
                    }
                    ImGui::PopID();
                    index++;
                }
                ImGui::EndCombo();
            }

            if (selectedProperty >= 0)
            {
                auto                &property = fontFamily.fontProperties[selectedProperty];
                CComPtr<IDWriteFont> pFont    = nullptr;
                HRESULT              hr       = fontFamily.pFontFamily->GetFont(selectedProperty, &pFont);
                // HRESULT              hr       = fontFamily.pFontFamily->GetFirstMatchingFont(
                //     property.weight, property.stretch, property.style, &pFont
                // );

                std::string utf8Path;
                if (SUCCEEDED(hr))
                {
                    utf8Path = GetFontFilePath(pFont);
                    ImGui::Text("Font Path: %s", utf8Path.c_str());
                }

                {
                    CComPtr<IDWriteLocalizedStrings> pFullName = nullptr;
                    BOOL                             exists    = FALSE;
                    hr = pFont->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_FULL_NAME, &pFullName, &exists);
                    if (SUCCEEDED(hr) && exists)
                    {
                        auto fullName = GetLocalizedString(pFullName);
                        ImGui::Text("Full name %s", fullName.c_str());
                    }
                }

                ImFontConfig cfg;
                cfg.OversampleH = cfg.OversampleV = 1;
                cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

                ImWchar ascii_ranges[] = {0x0020, 0x00FF, 0};
                cfg.GlyphExcludeRanges = ascii_ranges;

                ImFontConfig cfg1;
                cfg1.MergeMode = true;
                if (rebuildFont /*&& prevFontPath != utf8Path*/)
                {
                    if (previewFont)
                    {
                        io.Fonts->RemoveFont(previewFont);
                    }

                    cfg1.FontNo = property.faceIndex;
                    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 16.0, &cfg);
                    if (static_cast<uint32_t>(property.weight) >= static_cast<uint32_t>(DWRITE_FONT_WEIGHT_BOLD))
                    {
                        cfg1.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Bold;
                    }
                    if (property.style == DWRITE_FONT_STYLE_OBLIQUE)
                    {
                        cfg1.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Oblique;
                    }
                    io.Fonts->AddFontFromFileTTF(utf8Path.c_str(), 16.0, &cfg1);
                    previewFont = io.Fonts->AddFontFromFileTTF(defaultFontPath.c_str(), 16.0, &cfg1);
                }
                prevFontPath = utf8Path;
            }
        }
        ImGui::EndGroup();

        if (ImGui::Button("Apply"))
        {
            requestReplaceFont = true;
        }

        if (previewFont) ImGui::PushFont(previewFont);
        ImGui::Text("%s", R"(abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKLMNOPQRSTUVWXYZ
0123456789 (){}[]
+ - * / = .,;:!? #&$%@|^)");
        ImGui::Text("The quick brown fox jumps over the lazy dog");

        ImGui::Text("中文: 快速的棕色狐狸跳过了懒惰的狗");
        ImGui::Text("日本語: 速い茶色のキツネが怠惰な犬を飛び越えます");
        ImGui::Text("한국어: 빠른 갈색 여우가 게으른 개를 뛰어넘습니다");
        if (previewFont) ImGui::PopFont();
    }

    ImGui::PopStyleVar(pushCount);
    ImGui::End();
}

void FontManager::ApplyFont()
{
    if (!previewFont) return;

    auto &io           = ImGui::GetIO();
    requestReplaceFont = false;
    io.Fonts->RemoveFont(emojiFont);
    io.FontDefault = std::move(previewFont);
    emojiFont      = io.FontDefault;
    previewFont    = nullptr;
}

auto FontManager::GetFontFilePath(IDWriteFont *pFont) -> std::string
{
    CComPtr<IDWriteFontFace> pFontFace = nullptr;
    pFont->CreateFontFace(&pFontFace);

    UINT32 numFiles = 0;
    pFontFace->GetFiles(&numFiles, nullptr);
    std::vector<IDWriteFontFile *> fontFiles(numFiles);
    pFontFace->GetFiles(&numFiles, fontFiles.data());

    // 2. 取第一个文件并获取引用键
    IDWriteFontFile *fontFile         = fontFiles[0];
    const void      *referenceKey     = nullptr;
    UINT32           referenceKeySize = 0;
    fontFile->GetReferenceKey(&referenceKey, &referenceKeySize);

    // 3. 获取本地加载器实例
    CComPtr<IDWriteFontFileLoader> baseLoader;
    fontFile->GetLoader(&baseLoader);
    CComPtr<IDWriteLocalFontFileLoader> localLoader;
    localLoader = baseLoader;

    // 4. 查询路径长度并分配缓冲区
    UINT32 pathLength = 0;
    localLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &pathLength);
    std::vector<WCHAR> pathBuffer(pathLength + 1);

    // 5. 读取文件路径
    localLoader->GetFilePathFromKey(referenceKey, referenceKeySize, pathBuffer.data(), pathLength + 1);

    std::wstring fontPath(pathBuffer.data());
    std::string  utf8Path = LIBC_NAMESPACE::WCharUtils::ToString(fontPath);

    std::ranges::for_each(fontFiles, [&](IDWriteFontFile *pFontFile) {
        pFontFile->Release();
    });
    return utf8Path;
}

auto FontManager::GetLocalizedString(IDWriteLocalizedStrings *pStrings) -> std::string
{
    HRESULT hr     = S_OK;
    UINT32  index  = 0;
    BOOL    exists = false;

    wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

    // Get the default locale for this user.

    // If the default locale is returned, find that locale name, otherwise use "en-us".
    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH))
    {
        hr = pStrings->FindLocaleName(localeName, &index, &exists);
    }
    if (SUCCEEDED(hr) && !exists) // if the above find did not find a match, retry with US English
    {
        hr = pStrings->FindLocaleName(L"en-us", &index, &exists);
    }

    // If the specified locale doesn't exist, select the first on the list.
    if (!exists) index = 0;

    UINT32 length = 0;

    // Get the string length.
    if (SUCCEEDED(hr))
    {
        hr = pStrings->GetStringLength(index, &length);
    }
    std::wstring stdString;
    stdString.resize(length + 1);
    // Get the family name.
    if (SUCCEEDED(hr))
    {
        hr = pStrings->GetString(index, stdString.data(), length + 1);
    }
    if (SUCCEEDED(hr))
    {
        return LIBC_NAMESPACE::WCharUtils::ToString(stdString.c_str(), static_cast<int>(length));
    }
    return "";
}
}