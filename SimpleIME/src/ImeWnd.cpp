#include "ImeWnd.hpp"
#include "imgui_freetype.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include <dxgi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace SimpleIME
{

    static bool g_SwapChainOccluded = false;

    ImeWnd::ImeWnd()
    {
        m_hInst      = nullptr;
        m_hWndParent = nullptr;
        m_hWnd       = nullptr;
        m_pImeUI     = nullptr;
    }

    ImeWnd::~ImeWnd()
    {
        logv(info, "Release ImeWNd");
        if (m_hWnd)
        {
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            ::DestroyWindow(m_hWnd);
        }
    }

    void ImeWnd::Initialize(HWND a_parent, FontConfig *fontConfig) noexcept(false)
    {
        ZeroMemory(&wc, sizeof(wc));

        m_hInst          = GetModuleHandle(nullptr);
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_PARENTDC;
        wc.cbClsExtra    = 0;
        wc.lpfnWndProc   = ImeWnd::WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = m_hInst;
        wc.lpszClassName = g_tMainClassName;
        if (!::RegisterClassExW(&wc)) throw SimpleIMEException("Can't register class");
        DWORD dwExStyle = 0; // WS_EX_LAYERED | WS_EX_TOOLWINDOW;
        DWORD dwStyle   = WS_CHILD;
        int   nWidth    = 400;
        int   nHeight   = 300;
        m_hWnd          = ::CreateWindowExW(dwExStyle, g_tMainClassName, L"Hide", //
                                            dwStyle, 0, 0, nWidth, nHeight,       // width, height
                                            a_parent,                             // a_parent,
                                            nullptr, wc.hInstance, (LPVOID)this);
        //
        if (m_hWnd == nullptr) throw SimpleIMEException("Create ImeWnd failed");

        m_hWndParent = a_parent;
        ::SetLayeredWindowAttributes(m_hWnd, 0x00000000, 100, LWA_COLORKEY);
        m_pImeUI->QueryAllInstalledIME();
        //InitImGui(fontConfig);
        logv(info, "ImGui initialized.");

        ::ShowWindow(m_hWnd, SW_SHOWDEFAULT);
        ::UpdateWindow(m_hWnd);

        HIMC hIMC;
        MAKE_CONTEXT(m_hWnd, m_pImeUI->UpdateConversionMode);
        if ((hIMC = ImmGetContext(m_hWnd)))
        {
            if (ImmGetOpenStatus(hIMC))
            {
                DWORD conversion, sentence;
                ImmGetConversionStatus(hIMC, &conversion, &sentence);
                ImmSetConversionStatus(hIMC, conversion | IME_CMODE_ALPHANUMERIC, sentence);
            }
            ImmReleaseContext(m_hWnd, hIMC);
        }

        bool done = false;
        while (done)
        {
            /*MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT) done = true;
            }*/
            if (done) break;

            // Handle window being minimized or screen locked
            if (g_SwapChainOccluded && m_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
            {
                ::Sleep(10);
                continue;
            }
            g_SwapChainOccluded = false;

            // Handle window resize (we don't resize directly in the WM_SIZE handler)
            if (m_ResizeWidth != 0 && m_ResizeHeight != 0)
            {
                CleanupRenderTarget();
                m_pSwapChain->ResizeBuffers(0, m_ResizeWidth, m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
                m_ResizeWidth = m_ResizeHeight = 0;
                CreateRenderTarget();
            }

            RenderImGui();

            HRESULT hr          = m_pSwapChain->Present(1, 0);
            g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
        }
    }

    LRESULT ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        auto *pThis = (ImeWnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if ((nullptr == pThis) && (uMsg != WM_NCCREATE))
        {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        switch (uMsg)
        {
            case WM_NCCREATE: {
                auto lpCs = (LPCREATESTRUCT)lParam;
                pThis     = (ImeWnd *)(lpCs->lpCreateParams);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
                // set the window handle
                pThis->m_hInst = lpCs->hInstance;
                pThis->m_hWnd  = hWnd;
                // pThis->m_hWndParent = lpCs->hwndParent;
                break;
            }
            case WM_CREATE: {
                HIMC hIMC = ImmCreateContext();
                ImmAssociateContextEx(hWnd, hIMC, IACE_IGNORENOCONTEXT);
                return pThis->OnCreate();
            }
            case WM_DESTROY: {
                ImmAssociateContextEx(hWnd, nullptr, IACE_DEFAULT);
                return pThis->OnDestroy();
            }
            case WM_INPUTLANGCHANGE: {
                pThis->m_pImeUI->UpdateLanguage();
                return S_OK;
            }
            case WM_KEYDOWN:
                if (!pThis->IsImeEnabled())
                {
                    ::SetFocus(pThis->m_hWndParent);
                }
                return S_OK;
            case WM_IME_NOTIFY: {
                logv(debug, "ImeWnd {:#x}", wParam);
                if (pThis->m_pImeUI->ImeNotify(hWnd, wParam, lParam)) return S_OK;
                break;
            }
            case WM_SETFOCUS:
                MAKE_CONTEXT(hWnd, pThis->m_pImeUI->UpdateConversionMode);
                return S_OK;
            case WM_IME_STARTCOMPOSITION:
                return pThis->OnStartComposition();
            case WM_IME_ENDCOMPOSITION:
                logv(debug, "End composition, send test msg");
                return pThis->OnEndComposition();
            case WM_CUSTOM_IME_COMPPOSITION:
            case WM_IME_COMPOSITION:
                return pThis->OnComposition(hWnd, lParam);
            case WM_CUSTOM_CHAR:
            case WM_CHAR: {
                // never received WM_CHAR msg becaus wo handled WM_IME_COMPOSITION
                break;
            }
            case WM_IME_SETCONTEXT:
                // lParam &= ~ISC_SHOWUICANDIDATEWINDOW;
                return DefWindowProcW(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
            case WM_SIZE:
                if (wParam == SIZE_MINIMIZED) return 0;
                pThis->m_ResizeWidth  = (UINT)LOWORD(lParam); // Queue resize
                pThis->m_ResizeHeight = (UINT)HIWORD(lParam);
                break;
            default:
                ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                break;
        }
        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    LRESULT ImeWnd::OnStartComposition()
    {
        m_pImeUI->StartComposition();
        return 0;
    }

    LRESULT ImeWnd::OnEndComposition()
    {
        m_pImeUI->EndComposition();
        ::SetFocus(m_hWndParent);
        return 0;
    }

    LRESULT ImeWnd::OnComposition(HWND hWnd, LPARAM lParam)
    {
        MAKE_CONTEXT(hWnd, m_pImeUI->CompositionString, lParam);
        return 0;
    }

    LRESULT ImeWnd::OnCreate()
    {
        m_pImeUI = new ImeUI(m_hWnd, m_hWndParent);
        m_pImeUI->UpdateLanguage();
        return S_OK;
    }

    LRESULT ImeWnd::OnDestroy()
    {
        PostQuitMessage(0);
        return 0;
    }

    void ImeWnd::Focus() const
    {
        logv(debug, "Focus to ime wnd");
        ::SetFocus(m_hWnd);
    }

    // static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    void          ImeWnd::RenderImGui()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        m_pImeUI->RenderImGui();

        ImGui::Render();
        const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                                                 clear_color.z * clear_color.w, clear_color.w};
        m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
        m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    bool ImeWnd::IsImeEnabled() const
    {
        auto imeState = m_pImeUI->GetImeState();
        return imeState.all(IME_OPEN) && imeState.none(IME_IN_ALPHANUMERIC);
    }

    void ImeWnd::SendMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ::SendMessage(m_hWnd, msg, wParam, lParam);
    }

    static bool IsImeKeyCode(std::uint32_t code)
    {
        bool result = false;
        // q-p
        result |= code >= 0x10 && code <= 0x19;
        // a-l
        result |= code >= 0x1E && code <= 0x26;
        // z-m
        result |= code >= 0x2C && code <= 0x32;
        return result;
    }

    RE::InputEvent **ImeWnd::FilterInputEvent(RE::InputEvent **events)
    {
        static RE::InputEvent *dummy[]  = {nullptr};

        auto                   imeState = m_pImeUI->GetImeState();
        if (imeState.none(IME_OPEN) || imeState.any(IME_IN_ALPHANUMERIC))
        {
            return events;
        }

        auto first = (imeState == IME_OPEN);
        if (imeState.any(IME_OPEN, IME_IN_CANDCHOOSEN, IME_IN_COMPOSITION))
        {
            if (events == nullptr || *events == nullptr)
            {
                return events;
            }
            auto head         = *events;
            auto sourceDevice = head->device;
            bool discard      = sourceDevice == RE::INPUT_DEVICE::kKeyboard;
            if (first)
            {
                auto idEvent = head->AsIDEvent();
                auto code    = idEvent ? idEvent->GetIDCode() : 0;
                discard &= IsImeKeyCode(code);
            }
            if (discard) return dummy;
        }
        return events;
    }

    void ImeWnd::InitImGui(FontConfig *fontConfig) noexcept(false)
    {
        if (!CreateDeviceD3D())
        {
            throw SimpleIMEException("ImGui create device-d3d failed");
        }
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(m_hWnd);
        ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);
        io.MouseDrawCursor          = true;
        io.ConfigNavMoveSetMousePos = true;

        io.Fonts->AddFontFromFileTTF(fontConfig->eastAsiaFontFile.c_str(), fontConfig->fontSize, nullptr,
                                     io.Fonts->GetGlyphRangesChineseFull());
        // config font
        static ImFontConfig cfg;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.MergeMode                     = true;
        cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
        static const ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
        io.Fonts->AddFontFromFileTTF(fontConfig->emojiFontFile.c_str(), fontConfig->fontSize, &cfg, icons_ranges);
        io.Fonts->Build();

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding              = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
    }

    bool ImeWnd::CreateDeviceD3D() noexcept
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
        sd.OutputWindow                       = m_hWnd;
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
                                                    featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain,
                                                    &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
                                                featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain,
                                                &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
        if (res != S_OK) return false;

        CreateRenderTarget();
        return true;
    }

    void ImeWnd::CreateRenderTarget()
    {
        ID3D11Texture2D *pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
        pBackBuffer->Release();
    }

    void ImeWnd::CleanupDeviceD3D()
    {
        CleanupRenderTarget();
        if (m_pSwapChain)
        {
            m_pSwapChain->Release();
            m_pSwapChain = nullptr;
        }
        if (m_pd3dDeviceContext)
        {
            m_pd3dDeviceContext->Release();
            m_pd3dDeviceContext = nullptr;
        }
        if (m_pd3dDevice)
        {
            m_pd3dDevice->Release();
            m_pd3dDevice = nullptr;
        }
    }

    void ImeWnd::CleanupRenderTarget()
    {
        if (m_mainRenderTargetView)
        {
            m_mainRenderTargetView->Release();
            m_mainRenderTargetView = nullptr;
        }
    }
}