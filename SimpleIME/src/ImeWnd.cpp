#include "ImeWnd.hpp"
#include <d3d11.h>
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <winuser.inl>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
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
            if (m_hWnd != nullptr)
            {
                ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
                ::DestroyWindow(m_hWnd);
            }
        }

        void ImeWnd::Initialize(HWND a_parent) noexcept(false)
        {
            wc.hInstance = GetModuleHandle(nullptr);
            if (::RegisterClassExW(&wc) == 0U)
            {
                throw SimpleIMEException("Can't register class");
            }
            DWORD dwExStyle = 0;
            DWORD dwStyle   = WS_CHILD;
            m_hWnd          = ::CreateWindowExW(dwExStyle, g_tMainClassName, L"Hide", //
                                                dwStyle, 0, 0, 0, 0,                  // width, height
                                                a_parent,                             // a_parent,
                                                nullptr, wc.hInstance, (LPVOID)this);
            //
            if (m_hWnd == nullptr)
            {
                throw SimpleIMEException("Create ImeWnd failed");
            }
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
                    auto *lpCs = (LPCREATESTRUCT)lParam;
                    pThis      = (ImeWnd *)(lpCs->lpCreateParams);
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
                    return OnDestroy();
                }
                case WM_INPUTLANGCHANGE: {
                    pThis->m_pImeUI->UpdateLanguage();
                    return S_OK;
                }
                case WM_IME_NOTIFY: {
                    if (pThis->m_pImeUI->ImeNotify(hWnd, wParam, lParam))
                    {
                        return S_OK;
                    }
                    break;
                }
                case WM_SETFOCUS:
                    MAKE_CONTEXT(hWnd, pThis->m_pImeUI->UpdateConversionMode);
                    return S_OK;
                case WM_IME_STARTCOMPOSITION:
                    return pThis->OnStartComposition();
                case WM_IME_ENDCOMPOSITION:
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
            io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

            io.Fonts->AddFontFromFileTTF(fontConfig->GetEastAsiaFontFile().data(), fontConfig->GetFontSize(), nullptr,
                                         io.Fonts->GetGlyphRangesChineseFull());

            // config font
            static ImFontConfig cfg;
            cfg.OversampleH = cfg.OversampleV = 1;
            cfg.MergeMode                     = true;
            cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
            static const ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
            io.Fonts->AddFontFromFileTTF(fontConfig->GetEmojiFontFile().data(), fontConfig->GetFontSize(), &cfg,
                                         icons_ranges);
            io.Fonts->Build();
            ImGuiStyle &style = ImGui::GetStyle();
            if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
            {
                style.WindowRounding              = 0.0F;
                style.Colors[ImGuiCol_WindowBg].w = 1.0F;
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
            m_pImeUI = new ImeUI();
            return S_OK;
        }

        LRESULT ImeWnd::OnDestroy()
        {
            PostQuitMessage(0);
            return S_OK;
        }

        void ImeWnd::Focus() const
        {
            ::SetFocus(m_hWnd);
        }

        void ImeWnd::RenderIme()
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            m_pImeUI->RenderIme();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }

        void ImeWnd::ShowToolWindow()
        {
            m_pImeUI->ShowToolWindow();
        }

        void ImeWnd::SetImeOpenStatus(bool open) const
        {
            HIMC hIMC;
            if ((hIMC = ImmGetContext(m_hWnd)) != nullptr)
            {
                ImmSetOpenStatus(hIMC, static_cast<BOOL>(open));
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
            result |= code >= DIK_Q && code <= DIK_P;
            result |= code >= DIK_A && code <= DIK_L;
            result |= code >= DIK_Z && code <= DIK_M;
            return result;
        }

        bool ImeWnd::IsDiscardGameInputEvents(__in RE::InputEvent **events)
        {
            auto imeState = m_pImeUI->GetImeState();
            if (imeState.all(IME_UI_FOCUSED))
            {
                return true;
            }
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
                auto *head         = *events;
                auto  sourceDevice = head->device;
                bool  discard      = sourceDevice == RE::INPUT_DEVICE::kKeyboard;
                if (first)
                {
                    auto *idEvent = head->AsIDEvent();
                    auto  code    = (idEvent != nullptr) ? idEvent->GetIDCode() : 0;
                    discard &= IsImeKeyCode(code);
                }
                return discard;
            }
            return false;
        }
    }
}