//
// Created by jamie on 2026/2/13.
//

#include "m3_test_fixture.h"

#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_te_engine.h"
#include "imgui_te_internal.h"
#include "imgui_te_ui.h"
#include "imguiex/M3ThemeBuilder.h"
#include "imguiex/imguiex_m3.h"

#include <atlcomcli.h>

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool g_fHeadless = true;

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    bool use_gui = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--gui") == 0) use_gui = true;
    }
    g_fHeadless = !use_gui;
    return RUN_ALL_TESTS();
}

namespace ImGuiEx::M3::detail
{
namespace
{
[[nodiscard]] auto CreateWindowAndDevice(
    M3TestFixture::DX11RenderState &renderState, WNDPROC lpfnWndProc, LPCWSTR lpszClassName, M3TestFixture *pFixture
) -> HWND;
void InitImGuiTestEngine(ImGuiTestEngine *engine, bool headless);
void ShutdownImGuiTestEngine(ImGuiTestEngine *engine);
bool CreateDeviceD3D(HWND hWnd, M3TestFixture::DX11RenderState &renderState);
void CleanupRenderState(M3TestFixture::DX11RenderState &renderState);
void CreateRenderTarget(M3TestFixture::DX11RenderState &renderState);
void RenderD3D(HWND hwnd, M3TestFixture::DX11RenderState &renderState);
} // namespace

void M3TestFixture::Initialize(const bool headless)
{
    m_headless = headless;
    Context::CreateM3Styles(ImGui::GetFont());

    ImGui::CreateContext();

    if (!IsHeadless())
    {
        m_hwnd = CreateWindowAndDevice(m_dx11RenderState, WndProc, WND_CLASS_NAME, this);
        ImGui_ImplWin32_Init(m_hwnd);
        ImGui_ImplDX11_Init(m_dx11RenderState.d3dDevice, m_dx11RenderState.d3DeviceContext);
    }
    else
    {
        ImGuiIO &io    = ImGui::GetIO();
        io.DisplaySize = ImVec2(1920, 1080);
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    }

    m_testEngine = ImGuiTestEngine_CreateContext();
    InitImGuiTestEngine(m_testEngine, IsHeadless());
}

void M3TestFixture::Shutdown()
{
    if (m_hwnd)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();

        CleanupRenderState(m_dx11RenderState);
        DestroyWindow(m_hwnd);
        UnregisterClassW(WND_CLASS_NAME, GetModuleHandle(nullptr));
    }

    ShutdownImGuiTestEngine(m_testEngine);
}

void M3TestFixture::RunLoop()
{
    ImGuiTestEngine_Start(m_testEngine, ImGui::GetCurrentContext());
    ImGuiTestEngine_InstallDefaultCrashHandler();
    bool done = false;
    while (!done)
    {
        if (!IsHeadless())
        {
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT) done = true;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
        }
        ImGui::NewFrame();

        static bool open_engine_ui = true;
        ImGuiTestEngine_ShowTestEngineWindows(m_testEngine, &open_engine_ui);

        ImGui::Render();
        ImGuiTestEngine_PostSwap(m_testEngine);

        if (!IsHeadless())
        {
            RenderD3D(m_hwnd, m_dx11RenderState);
        }
        else if (ImGuiTestEngine_IsTestQueueEmpty(m_testEngine))
        {
            done = true;
        }
    }

    ImGuiTestEngineResultSummary summary;
    ImGuiTestEngine_GetResultSummary(m_testEngine, &summary);

    ASSERT_EQ(summary.CountTested, summary.CountSuccess);
}

LRESULT M3TestFixture::WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) != 0)
    {
        return 1;
    }
    switch (msg)
    {
        case WM_NCCREATE: {
            const auto lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            auto      *pThis1         = static_cast<M3TestFixture *>(lpCreateStruct->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis1));
            pThis1->m_hwnd = hWnd;
            return TRUE;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

namespace
{
[[nodiscard]] auto CreateWindowAndDevice(
    M3TestFixture::DX11RenderState &renderState, const WNDPROC lpfnWndProc, const LPCWSTR lpszClassName, M3TestFixture *pFixture
) -> HWND
{
    const WNDCLASSEX wc = {
        sizeof(wc), CS_CLASSDC, lpfnWndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, lpszClassName, nullptr
    };
    RegisterClassExW(&wc);
    const HWND hwnd = CreateWindowW(
        wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, pFixture
    );

    if (!CreateDeviceD3D(hwnd, renderState))
    {
        CleanupRenderState(renderState);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return nullptr;
    }
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    return hwnd;
}

void InitImGuiTestEngine(ImGuiTestEngine *engine, bool headless)
{
    ImGuiTestEngineIO &test_io        = ImGuiTestEngine_GetIO(engine);
    test_io.ConfigVerboseLevel        = ImGuiTestVerboseLevel_Info;
    test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    test_io.ConfigLogToTTY            = true; // always log to tty for test, avoid missing logs when crash happens before log file is created.
    if (headless)
    {
        test_io.ConfigNoThrottle = true;
    }
}

void ShutdownImGuiTestEngine(ImGuiTestEngine *engine)
{
    ImGuiTestEngine_Stop(engine);

    ImGui::DestroyContext();
    ImGuiTestEngine_DestroyContext(engine);
}

bool CreateDeviceD3D(const HWND hWnd, M3TestFixture::DX11RenderState &renderState)
{
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

    constexpr UINT              createDeviceFlags = 0;
    D3D_FEATURE_LEVEL           featureLevel;
    constexpr D3D_FEATURE_LEVEL featureLevelArray[2] = {
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
        &renderState.swapChain,
        &renderState.d3dDevice,
        &featureLevel,
        &renderState.d3DeviceContext
    );
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            &renderState.swapChain,
            &renderState.d3dDevice,
            &featureLevel,
            &renderState.d3DeviceContext
        );
    if (res != S_OK) return false;

    CreateRenderTarget(renderState);
    return true;
}

void CleanupRenderState(M3TestFixture::DX11RenderState &renderState)
{
    renderState.mainRenderTargetView = nullptr;
    renderState.swapChain            = nullptr;
    renderState.d3DeviceContext      = nullptr;
    renderState.d3dDevice            = nullptr;
}

void CreateRenderTarget(M3TestFixture::DX11RenderState &renderState)
{
    ID3D11Texture2D *back_buffer = nullptr;
    if (renderState.swapChain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)) == S_OK && back_buffer != nullptr)
    {
        renderState.d3dDevice->CreateRenderTargetView(back_buffer, nullptr, &renderState.mainRenderTargetView);
        back_buffer->Release();
    }
}

void RenderD3D(const HWND hwnd, M3TestFixture::DX11RenderState &renderState)
{
    constexpr float clear_color_with_alpha[4] = {0.45f, 0.55f, 0.60f, 1.00f};

    // CComPtr prevent use `&` with an initialized pointer.
    // And the 2nd param in `OMSetRenderTargets` also require a array pointer.
    std::array<ID3D11RenderTargetView *, 1> rtvs{renderState.mainRenderTargetView};
    renderState.d3DeviceContext->OMSetRenderTargets(1, rtvs.data(), nullptr);
    renderState.d3DeviceContext->ClearRenderTargetView(renderState.mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    /*HRESULT hr = */ renderState.swapChain->Present(1, 0);
    // g_swap_chain_occluded = (hr == DXGI_STATUS_OCCLUDED);
    UpdateWindow(hwnd);
}

} // namespace

} // namespace ImGuiEx::M3::detail
