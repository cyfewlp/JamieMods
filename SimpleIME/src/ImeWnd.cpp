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
        m_hWndParent = nullptr;
        m_hWnd       = nullptr;
        m_pImeUI     = nullptr;
        wc           = {};
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_PARENTDC;
        wc.cbClsExtra    = 0;
        wc.lpfnWndProc   = ImeWnd::WndProc;
        wc.cbWndExtra    = 0;
        wc.lpszClassName = g_tMainClassName;
    }

    ImeWnd::~ImeWnd()
    {
        delete m_pImeUI;
        if (m_hWnd)
        {
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            ::DestroyWindow(m_hWnd);
        }
    }

    void ImeWnd::Initialize(HWND a_parent) noexcept(false)
    {
        wc.hInstance = GetModuleHandle(nullptr);
        if (!::RegisterClassExW(&wc)) throw SimpleIMEException("Can't register class");
        DWORD dwExStyle = 0;
        DWORD dwStyle   = WS_CHILD;
        m_hWnd          = ::CreateWindowExW(dwExStyle, g_tMainClassName, L"Hide", //
                                            dwStyle, 0, 0, 0, 0,                  // width, height
                                            a_parent,                             // a_parent,
                                            nullptr, wc.hInstance, (LPVOID)this);
        //
        if (m_hWnd == nullptr) throw SimpleIMEException("Create ImeWnd failed");
        m_pImeUI->QueryAllInstalledIME();
        m_pImeUI->UpdateLanguage();
        m_pImeUI->UpdateActiveLangProfile();
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
                pThis->m_hWnd       = hWnd;
                pThis->m_hWndParent = lpCs->hwndParent;
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
                logv(debug, "WM_INPUTLANGCHANGE hkl: {}", lParam);
                pThis->m_pImeUI->UpdateLanguage();
                return S_OK;
            }
            case WM_IME_NOTIFY: {
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
            default:
                ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                break;
        }
        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    void ImeWnd::InitImGui(ID3D11Device *device, ID3D11DeviceContext *context, FontConfig *fontConfig) const
        noexcept(false)
    {
        logv(info, "Initializing ImGui...");
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        if (!ImGui_ImplWin32_Init(m_hWndParent))
        {
            throw SimpleIMEException("ImGui initialization failed (Win32)");
        }

        if (!ImGui_ImplDX11_Init(device, context))
        {
            throw SimpleIMEException("ImGui initialization failed (DX11)");
        }

        RECT     rect = {0, 0, 0, 0};
        ImGuiIO &io   = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        ImGui::StyleColorsDark();
        GetClientRect(m_hWndParent, &rect);
        io.DisplaySize              = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
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

        logv(info, "ImGui initialized!");
    }

    LRESULT ImeWnd::OnStartComposition()
    {
        m_pImeUI->StartComposition();
        return 0;
    }

    LRESULT ImeWnd::OnEndComposition()
    {
        m_pImeUI->EndComposition();
        return 0;
    }

    LRESULT ImeWnd::OnComposition(HWND hWnd, LPARAM lParam)
    {
        MAKE_CONTEXT(hWnd, m_pImeUI->CompositionString, lParam);
        return 0;
    }

    LRESULT ImeWnd::OnCreate()
    {
        m_pImeUI = new ImeUI(m_hWnd);
        return S_OK;
    }

    LRESULT ImeWnd::OnDestroy()
    {
        PostQuitMessage(0);
        return 0;
    }

    void ImeWnd::Focus() const
    {
        ::SetFocus(m_hWnd);
    }

    void ImeWnd::RenderImGui()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        m_pImeUI->RenderImGui();

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImeWnd::SetImeOpenStatus(bool open)
    {
        HIMC hIMC;
        if ((hIMC = ImmGetContext(m_hWnd)))
        {
            ImmSetOpenStatus(hIMC, open);
            ImmReleaseContext(m_hWnd, hIMC);
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

    bool ImeWnd::IsDiscardGameInputEvents(RE::InputEvent **events)
    {
        auto imeState = m_pImeUI->GetImeState();
        if (imeState.none(IME_OPEN) || imeState.any(IME_IN_ALPHANUMERIC))
        {
            return false;
        }

        auto first = (imeState == IME_OPEN);
        if (imeState.any(IME_OPEN, IME_IN_CANDCHOOSEN, IME_IN_COMPOSITION))
        {
            if (events == nullptr || *events == nullptr)
            {
                return false;
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
            return discard;
        }
        return false;
    }
}