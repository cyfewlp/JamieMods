#include "ImeWnd.hpp"
#include "ImeUI.h"
#include "configs/AppConfig.h"

#include <basetsd.h>
#include <cstddef>
#include <cstdint>
#include <d3d11.h>
#include <dinput.h>
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <windows.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        ImeWnd::ImeWnd()
        {
            wc = {};
            ZeroMemory(&wc, sizeof(wc));
            wc.cbSize        = sizeof(wc);
            wc.style         = CS_PARENTDC;
            wc.cbClsExtra    = 0;
            wc.lpfnWndProc   = WndProc;
            wc.cbWndExtra    = 0;
            wc.lpszClassName = g_tMainClassName;

            m_pInputMethod   = std::make_unique<InputMethod>();
            m_pTextStore     = std::make_unique<Tsf::TextStore>(m_pInputMethod.get());
            m_immImeHandler  = std::make_unique<ImmImeHandler>(m_pInputMethod.get());
        }

        ImeWnd::~ImeWnd()
        {
            UnInitialize();
            if (m_hWnd != nullptr)
            {
                UnregisterClassW(wc.lpszClassName, wc.hInstance);
                DestroyWindow(m_hWnd);
            }
        }

        void ImeWnd::Initialize() noexcept(false)
        {
            wc.hInstance = GetModuleHandle(nullptr);
            if (RegisterClassExW(&wc) == 0U)
            {
                throw SimpleIMEException("Can't register class");
            }
            auto *pAppConfig = AppConfig::GetConfig();
            m_fEnableTsf     = pAppConfig->EnableTsf();
            m_pImeUi         = std::make_unique<ImeUI>(pAppConfig->GetAppUiConfig(), m_pInputMethod.get());

            HRESULT hresult  = m_tsfSupport.InitializeTsf(true);
            if (FAILED(hresult))
            {
                throw SimpleIMEException("Can't initialize TsfSupport");
            }
            if (FAILED(m_pLangProfileUtil->Initialize(m_tsfSupport.GetThreadMgr())))
            {
                throw SimpleIMEException("Can't initialize LangProfileUtil");
            }
            hresult = m_pTextStore->Initialize(m_tsfSupport.GetThreadMgr(), m_tsfSupport.GetTfClientId());
            if (FAILED(hresult))
            {
                throw SimpleIMEException("Can't initialize TextStore");
            }
            hresult = m_pTsfCompartment->Initialize(m_tsfSupport.GetThreadMgr(),
                                                    GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
            if (FAILED(hresult))
            {
                throw SimpleIMEException("Can't initialize TsfCompartment");
            }
            if (!m_pImeUi->Initialize(m_pLangProfileUtil))
            {
                throw SimpleIMEException("Can't initialize ImeUI");
            }
        }

        void ImeWnd::UnInitialize() const noexcept
        {
            if (m_pTsfCompartment != nullptr)
            {
                m_pTsfCompartment->UnInitialize();
            }
            if (m_pTextStore != nullptr)
            {
                m_pTextStore->UnInitialize();
            }
            if (m_pTsfCompartment != nullptr)
            {
                m_pTsfCompartment->UnInitialize();
            }
            if (m_pLangProfileUtil != nullptr)
            {
                m_pLangProfileUtil->UnInitialize();
            }
        }

        void ImeWnd::Start(HWND hWndParent)
        {
            log_info("Start ImeWnd Thread...");
            DWORD dwExStyle = 0;
            DWORD dwStyle   = WS_CHILD;
            m_hWnd = ::CreateWindowExW(dwExStyle, g_tMainClassName, L"Hide", dwStyle, 0, 0, 0, 0, hWndParent, nullptr,
                                       wc.hInstance, (LPVOID)this);
            if (m_hWnd == nullptr)
            {
                throw SimpleIMEException("Create ImeWnd failed");
            }

            // start message loop
            if (m_fEnableTsf)
            {
                m_pTextStore->SetHWND(m_hWnd);
                m_pTextStore->Focus();
            }
            Focus();
            MSG msg = {};
            ZeroMemory(&msg, sizeof(msg));
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
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    return pThis->OnDestroy();
                }
                case WM_INPUTLANGCHANGE: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    pThis->m_pImeUi->UpdateLanguage();
                    return S_OK;
                }
                case WM_IME_NOTIFY: {
                    const auto *pThis = GetThis(hWnd);
                    if (pThis == nullptr) break;
                    if (pThis->m_fEnableTsf && pThis->m_pTextStore->IsSupportCandidateUi())
                    {
                        return S_OK;
                    }
                    if (pThis->m_immImeHandler->ImeNotify(hWnd, wParam, lParam))
                    {
                        return S_OK;
                    }
                    break;
                }
                // case WM_IME_STARTCOMPOSITION: {
                //     const auto *pThis = GetThis(hWnd);
                //     if (pThis == nullptr) break;
                //     return pThis->OnStartComposition();
                // }
                // case WM_IME_ENDCOMPOSITION: {
                //     const auto *pThis = GetThis(hWnd);
                //     if (pThis == nullptr) break;
                //     return pThis->OnEndComposition();
                // }
                // case CM_IME_COMPOSITION:
                // case WM_IME_COMPOSITION: {
                //     const auto *pThis = GetThis(hWnd);
                //     if (pThis == nullptr) break;
                //     return pThis->OnComposition(hWnd, lParam);
                // }
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
                    pThis->m_pLangProfileUtil->ActivateProfile(reinterpret_cast<GUID *>(lParam));
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
            static constexpr ImWchar icons_ranges[] = {0x1, 0x1FFFF, 0}; // Will not be copied
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
            // m_pImeUi->StartComposition();
            return 0;
        }

        auto ImeWnd::OnEndComposition() const -> LRESULT
        {
            // m_pImeUi->EndComposition();
            return 0;
        }

        auto ImeWnd::OnComposition(HWND hWnd, LPARAM lParam) const -> LRESULT
        {
            HIMC hIMC = ImmGetContext(hWnd);
            if (hIMC != nullptr)
            {
                m_pImeUi->CompositionString(hIMC, lParam);
                ImmReleaseContext(hWnd, hIMC);
            }
            return 0;
        }

        auto ImeWnd::OnCreate() const -> LRESULT
        {
            m_pImeUi->SetHWND(m_hWnd);
            return S_OK;
        }

        auto ImeWnd::OnDestroy() const -> LRESULT
        {
            UnInitialize();
            PostQuitMessage(0);
            return S_OK;
        }

        void ImeWnd::Focus() const
        {
            log_debug("focus to ime wnd");
            ::SetFocus(m_hWnd);
        }

        void ImeWnd::RenderIme() const
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            m_pImeUi->RenderIme();

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
            m_pImeUi->ShowToolWindow();
        }

        static auto IsImeKeyCode(const std::uint32_t code) -> bool
        {
            bool result = false;
            result |= code >= DIK_Q && code <= DIK_P;
            result |= code >= DIK_A && code <= DIK_L;
            result |= code >= DIK_Z && code <= DIK_M;
            return result;
        }

        auto ImeWnd::IsDiscardGameInputEvents(__in RE::InputEvent **events) const -> bool
        {
            if (events == nullptr || *events == nullptr)
            {
                return false;
            }

            auto &imeState  = m_pInputMethod->GetState();
            auto  isImeOpen = m_pLangProfileUtil->IsAnyProfileActivated();
            if (!isImeOpen || imeState.any(InputMethod::State::IN_ALPHANUMERIC))
            {
                return false;
            }
            auto *head         = *events;
            auto  sourceDevice = head->device;
            if (sourceDevice == RE::INPUT_DEVICE::kKeyboard)
            {
                if (imeState.any(InputMethod::State::IN_CANDCHOOSEN, InputMethod::State::IN_COMPOSITION))
                {
                    return true;
                }
                // discard the first keyboard event when ime waiting input.
                auto *idEvent = head->AsIDEvent();
                auto  code    = idEvent != nullptr ? idEvent->GetIDCode() : 0;
                return IsImeKeyCode(code);
            }
            return false;
        }
    }
}