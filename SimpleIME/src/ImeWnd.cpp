#include "ImeWnd.hpp"
#include "ImeUI.h"
#include "configs/AppConfig.h"
#include <basetsd.h>
#include <cstddef>
#include <cstdint>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        ImeWnd::ImeWnd() : m_tsfSupport(), m_hWnd(nullptr), m_hWndParent(nullptr), m_pImeUI(nullptr)
        {
            wc = {};
            ZeroMemory(&wc, sizeof(wc));
            wc.cbSize        = sizeof(wc);
            wc.style         = CS_PARENTDC;
            wc.cbClsExtra    = 0;
            wc.lpfnWndProc   = WndProc;
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

        void ImeWnd::Initialize() noexcept(false)
        {
            wc.hInstance = GetModuleHandle(nullptr);
            if (::RegisterClassExW(&wc) == 0U)
            {
                throw SimpleIMEException("Can't register class");
            }
            auto *pAppConfig = AppConfig::GetConfig();
            m_pImeUI         = new ImeUI(pAppConfig->GetAppUiConfig());
            if (!m_tsfSupport.InitializeTsf())
            {
                throw SimpleIMEException("Can't initialize TsfSupport");
            }
            if (!m_pImeUI->Initialize(m_tsfSupport))
            {
                throw SimpleIMEException("Can't initialize ImeUI");
            }
        }

        void ImeWnd::Start(HWND hWndParent)
        {
            log_info("Start ImeWnd Thread...");
            DWORD dwExStyle = 0;
            DWORD dwStyle   = WS_CHILD;
            m_hWnd          = ::CreateWindowExW(dwExStyle, g_tMainClassName, L"Hide", //
                                                dwStyle, 0, 0, 0, 0,                  // width, height
                                                hWndParent,                           // a_parent,
                                                nullptr, wc.hInstance, (LPVOID)this);
            //
            if (m_hWnd == nullptr)
            {
                throw SimpleIMEException("Create ImeWnd failed");
            }

            // start message loop
            Focus();
            MSG  msg = {nullptr};
            BOOL bRet;
            while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
            {
                if (bRet == -1)
                {
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                {
                    break;
                }
            }
            log_info("Exit ImeWnd Thread...");
        }

        auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            switch (uMsg)
            {
                case WM_NCCREATE: {
                    auto *lpCs  = reinterpret_cast<LPCREATESTRUCT>(lParam);
                    auto *pThis = static_cast<ImeWnd *>(lpCs->lpCreateParams);
                    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
                    // set the window handle
                    pThis->m_hWnd       = hWnd;
                    pThis->m_hWndParent = lpCs->hwndParent;
                    break;
                }
                case WM_CREATE: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    HIMC hIMC = ImmCreateContext();
                    ImmAssociateContextEx(hWnd, hIMC, IACE_IGNORENOCONTEXT);
                    return pThis->OnCreate();
                }
                case WM_DESTROY: {
                    ImmAssociateContextEx(hWnd, nullptr, IACE_DEFAULT);
                    return OnDestroy();
                }
                case WM_INPUTLANGCHANGE: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    pThis->m_pImeUI->UpdateLanguage();
                    return S_OK;
                }
                case WM_IME_NOTIFY: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    if (pThis->m_pImeUI->ImeNotify(hWnd, wParam, lParam))
                    {
                        return S_OK;
                    }
                    break;
                }
                case WM_IME_STARTCOMPOSITION: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    return pThis->OnStartComposition();
                }
                case WM_IME_ENDCOMPOSITION: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    return pThis->OnEndComposition();
                }
                case CM_IME_COMPOSITION:
                case WM_IME_COMPOSITION: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    return pThis->OnComposition(hWnd, lParam);
                }
                case CM_CHAR:
                case WM_CHAR: {
                    // never received WM_CHAR msg because wo handled WM_IME_COMPOSITION
                    break;
                }
                case WM_IME_SETCONTEXT:
                    return DefWindowProcW(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
                case CM_ACTIVATE_PROFILE: {
                    const auto pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    pThis->m_pImeUI->ActivateProfile(reinterpret_cast<GUID *>(lParam));
                    return S_OK;
                }
                default:
                    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    break;
            }
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }

        auto ImeWnd::GetThis(const HWND hWnd) -> ImeWnd *
        {
            auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (ptr == 0) return nullptr;
            return reinterpret_cast<ImeWnd *>(ptr);
        }

        void ImeWnd::InitImGui(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context) const noexcept(false)
        {
            log_info("Initializing ImGui...");
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            if (!ImGui_ImplWin32_Init(hWnd)) // avoid use member m_hWndParent: async with ImeWnd thread
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
            io.ConfigNavMoveSetMousePos = false;
            ImGui::StyleColorsDark();
            GetClientRect(m_hWndParent, &rect);
            io.DisplaySize =
                ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

            const auto &uiConfig = AppConfig::GetConfig()->GetAppUiConfig();
            io.Fonts->AddFontFromFileTTF(uiConfig.EastAsiaFontFile().c_str(), uiConfig.FontSize(), nullptr,
                                         io.Fonts->GetGlyphRangesChineseFull());

            // config font
            static ImFontConfig cfg;
            cfg.OversampleH = cfg.OversampleV = 1;
            cfg.MergeMode                     = true;
            cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
            static const ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
            io.Fonts->AddFontFromFileTTF(uiConfig.EmojiFontFile().c_str(), uiConfig.FontSize(), &cfg, icons_ranges);
            io.Fonts->Build();
            ImGuiStyle &style = ImGui::GetStyle();
            if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
            {
                style.WindowRounding              = 0.0F;
                style.Colors[ImGuiCol_WindowBg].w = 1.0F;
            }

            log_info("ImGui initialized!");
        }

        auto ImeWnd::OnStartComposition() const -> LRESULT
        {
            m_pImeUI->StartComposition();
            return 0;
        }

        auto ImeWnd::OnEndComposition() const -> LRESULT
        {
            m_pImeUI->EndComposition();
            return 0;
        }

        auto ImeWnd::OnComposition(HWND hWnd, LPARAM lParam) const -> LRESULT
        {
            HIMC hIMC = ImmGetContext(hWnd);
            if (hIMC != nullptr)
            {
                m_pImeUI->CompositionString(hIMC, lParam);
                ImmReleaseContext(hWnd, hIMC);
            }
            return 0;
        }

        auto ImeWnd::OnCreate() const -> LRESULT
        {
            m_pImeUI->SetHWND(m_hWnd);
            return S_OK;
        }

        auto ImeWnd::OnDestroy() -> LRESULT
        {
            PostQuitMessage(0);
            return S_OK;
        }

        void ImeWnd::Focus() const
        {
            log_debug("focus to ime wnd");
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

        void ImeWnd::ShowToolWindow() const
        {
            m_pImeUI->ShowToolWindow();
        }

        void ImeWnd::SetImeOpenStatus(bool open) const
        {
            if (HIMC hImc; (hImc = ImmGetContext(m_hWnd)) != nullptr)
            {
                ImmSetOpenStatus(hImc, static_cast<BOOL>(open));
                ImmReleaseContext(m_hWnd, hImc);
            }
        }

        static auto IsImeKeyCode(const std::uint32_t code) -> bool
        {
            bool result = false;
            result |= code >= DIK_Q && code <= DIK_P;
            result |= code >= DIK_A && code <= DIK_L;
            result |= code >= DIK_Z && code <= DIK_M;
            return result;
        }

        auto ImeWnd::IsDiscardGameInputEvents(__in RE::InputEvent **events) -> bool
        {
            auto imeState = m_pImeUI->GetImeState();
            if (imeState.none(ImeState::OPEN) || imeState.any(ImeState::IN_ALPHANUMERIC))
            {
                return false;
            }

            auto first = (imeState == ImeState::OPEN);
            if (imeState.any(ImeState::OPEN, ImeState::IN_CANDCHOOSEN, ImeState::IN_COMPOSITION))
            {
                if (events == nullptr || *events == nullptr)
                {
                    return false;
                }
                auto *head         = *events;
                auto  sourceDevice = head->device;
                bool  discard      = (sourceDevice == RE::INPUT_DEVICE::kKeyboard);
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