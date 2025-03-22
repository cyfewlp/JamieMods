#include "ImeWnd.hpp"

#include "ImeUI.h"
#include "SimpleImeSupport.h"
#include "Utils.h"
#include "common/log.h"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "core/State.h"
#include "ime/ITextServiceFactory.h"
#include "ime/ImeManager.h"
#include "ime/ImeSupportUtils.h"

#include <d3d11.h>
#include <imgui.h>
#include <imgui_freetype.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <msctf.h>
#include <windows.h>
#include <windowsx.h>

// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
        }

        ImeWnd::~ImeWnd()
        {
            UnInitialize();
            if (m_hWnd != nullptr)
            {
                UnregisterClassW(wc.lpszClassName, wc.hInstance);
                ::DestroyWindow(m_hWnd);
            }
        }

        void ImeWnd::InitializeTextService(const AppConfig &pAppConfig)
        {
            m_fEnableTsf               = pAppConfig.EnableTsf();
            ITextService *pTextService = nullptr;
            ITextServiceFactory::CreateInstance(m_fEnableTsf, &pTextService);
            if (FAILED(pTextService->Initialize()))
            {
                throw SimpleIMEException("Can't initialize TextService");
            }
            m_pTextService.reset(pTextService);
            m_pTextService->RegisterCallback(OnCompositionResult);
            m_pLangProfileUtil = new LangProfileUtil();
        }

        void ImeWnd::Initialize() noexcept(false)
        {
            wc.hInstance = GetModuleHandle(nullptr);
            if (RegisterClassExW(&wc) == 0U)
            {
                throw SimpleIMEException("Can't register class");
            }

            const auto &appConfig = AppConfig::GetConfig();
            InitializeTextService(appConfig);
            m_pImeUi = std::make_unique<ImeUI>(appConfig.GetAppUiConfig(), this, m_pTextService.get());

            auto const &tsfSupport = Tsf::TsfSupport::GetSingleton();
            if (FAILED(m_pLangProfileUtil->Initialize(tsfSupport.GetThreadMgr())))
            {
                throw SimpleIMEException("Can't initialize LangProfileUtil");
            }
            if (!m_pImeUi->Initialize(m_pLangProfileUtil))
            {
                throw SimpleIMEException("Can't initialize ImeUI");
            }
        }

        void ImeWnd::UnInitialize() const noexcept
        {
            if (m_pTextService != nullptr)
            {
                m_pTextService->UnInitialize();
            }
            if (m_pLangProfileUtil != nullptr)
            {
                m_pLangProfileUtil->UnInitialize();
            }
        }

        auto ImeWnd::IsImeWantMessage(MSG &msg, ITfKeystrokeMgr *pKeystrokeMgr)
        {
            BOOL fEaten = FALSE;
            if (msg.message == WM_KEYDOWN)
            {
                // does an IME want it?
                if (pKeystrokeMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                    pKeystrokeMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
                {
                    return true;
                }
            }
            else if (msg.message == WM_KEYUP)
            {
                // does an IME want it?
                if (pKeystrokeMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                    pKeystrokeMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
                {
                    return true;
                }
            }
            return false;
        }

        void ImeWnd::Start(HWND hWndParent)
        {
            log_info("Start ImeWnd Thread...");
            m_hWnd = ::CreateWindowExW(0, g_tMainClassName, L"Hide", WS_CHILD, //
                                       0, 0, 0, 0,                             // x,y,w,h
                                       hWndParent, nullptr, wc.hInstance, this);
            if (m_hWnd == nullptr)
            {
                throw SimpleIMEException("Create ImeWnd failed");
            }
            OnStart();

            MSG msg = {};
            ZeroMemory(&msg, sizeof(msg));
            auto const &tsfSupport    = Tsf::TsfSupport::GetSingleton();
            auto const  pMessagePump  = tsfSupport.GetMessagePump();
            auto const  pKeystrokeMgr = tsfSupport.GetKeystrokeMgr();
            bool done = false;
            while (!done)
            {
                BOOL fResult = 0;
                if (pMessagePump->PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE, &fResult) != S_OK)
                {
                    done = true;
                }

                if (fResult != FALSE)
                {
                    continue;
                }

                if (::GetMessageW(&msg, nullptr, 0, 0) <= 0)
                {
                    break;
                }
                if (IsImeWantMessage(msg, pKeystrokeMgr))
                {
                    continue;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT)
                {
                    done = true;
                }
                if (done)
                {
                    break;
                }
            }
            log_info("Exit ImeWnd Thread...");
        }

        void ImeWnd::OnStart()
        {
            m_pTextService->OnStart(m_hWnd);
            Context::GetInstance()->SetHwndIme(m_hWnd);

            ImeManagerComposer::Init(this, m_hWndParent);
            ImeManagerComposer::GetInstance()->Use(FocusManageType::Permanent);
        }

        auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            ImeWnd *pThis = GetThis(hWnd);
            if (pThis != nullptr)
            {
                pThis->ForwardKeyboardMessage(uMsg, wParam, lParam);
                if (pThis->m_pTextService->ProcessImeMessage(hWnd, uMsg, wParam, lParam))
                {
                    return S_OK;
                }
            }

            switch (uMsg)
            {
                HANDLE_MSG(hWnd, WM_NCCREATE, OnNccCreate);
                case WM_CREATE: {
                    if (pThis == nullptr) break;
                    return pThis->OnCreate();
                }
                case WM_DESTROY: {
                    if (pThis == nullptr) break;
                    return pThis->OnDestroy();
                }
                case CM_IME_ENABLE: {
                    ImeManagerComposer::GetInstance()->EnableIme(wParam != FALSE);
                    return S_OK;
                }
                case CM_MOD_ENABLE: {
                    ImeManagerComposer::GetInstance()->EnableMod(wParam != FALSE);
                    return S_OK;
                }
                case CM_ACTIVATE_PROFILE: {
                    if (pThis == nullptr) break;
                    pThis->m_pLangProfileUtil->ActivateProfile(reinterpret_cast<GUID *>(lParam));
                    return S_OK;
                }
                case WM_SETFOCUS:
                    if (pThis == nullptr) break;
                    pThis->m_fFocused = true;
                    log_info("IME window get focus.");
                    return S_OK;
                case WM_KILLFOCUS:
                    if (pThis == nullptr) break;
                    pThis->m_fFocused = false;
                    log_info("IME window lost focus.");
                    return S_OK;
                default:
                    // ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    break;
            }
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }

        auto ImeWnd::GetThis(HWND hWnd) -> ImeWnd *
        {
            const auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
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
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigNavMoveSetMousePos = false;
            m_pImeUi->SetTheme();
            GetClientRect(m_hWndParent, &rect);
            io.DisplaySize =
                ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

            const auto &uiConfig = AppConfig::GetConfig().GetAppUiConfig();
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

        auto ImeWnd::OnCreate() const -> LRESULT
        {
            return S_OK;
        }

        auto ImeWnd::OnImeEnable(bool enable) -> bool
        {
            return m_pTextService->OnFocus(enable);
        }

        void ImeWnd::ForwardKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const
        {
            static DWORD gameThread    = 0;
            static DWORD currentThread = ::GetCurrentThreadId();
            if (gameThread == 0)
            {
                gameThread = ::GetWindowThreadProcessId(m_hWndParent, NULL);
            }
            if (gameThread != currentThread)
            {
                if (::AttachThreadInput(gameThread, currentThread, TRUE) != FALSE)
                {
                    switch (uMsg)
                    {
                        case WM_KEYDOWN:
                        case WM_KEYUP:
                        case WM_SYSKEYDOWN:
                        case WM_SYSKEYUP: {
                            if (::PostMessageA(m_hWndParent, uMsg, wParam, lParam) == FALSE)
                            {
                                log_debug("Post message to game failed.");
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    ::AttachThreadInput(gameThread, currentThread, FALSE);
                }
            }
        }

        auto ImeWnd::OnDestroy() const -> LRESULT
        {
            log_info("Destroy IME Window");
            UnInitialize();
            PostQuitMessage(0);
            return S_OK;
        }

        auto ImeWnd::Focus() const -> bool
        {
            if (!IsFocused())
            {
                ::SetFocus(m_hWnd);
            }
            return IsFocused();
        }

        auto ImeWnd::SetTsfFocus(bool focus) -> bool
        {
            return m_pTextService->OnFocus(focus);
        }

        auto ImeWnd::IsFocused() const -> bool
        {
            return m_fFocused;
        }

        auto ImeWnd::SendMessageToIme(UINT uMsg, WPARAM wParam, LPARAM lParam) const -> BOOL
        {
            if (m_hWnd == nullptr)
            {
                return 0;
            }
            return ::SendMessageW(m_hWnd, uMsg, wParam, lParam);
        }

        auto ImeWnd::SendNotifyMessageToIme(UINT uMsg, WPARAM wParam, LPARAM lParam) const -> BOOL
        {
            if (m_hWnd == nullptr)
            {
                return 0;
            }
            return ::SendNotifyMessageW(m_hWnd, uMsg, wParam, lParam);
        }

        auto ImeWnd::GetImeThreadId() const -> DWORD
        {
            return ::GetWindowThreadProcessId(m_hWnd, NULL);
        }

        void ImeWnd::AbortIme() const
        {
            if (State::GetInstance().HasAny(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
            {
                ::SetFocus(m_hWndParent);
            }
        }

        /**
         * If Game cursor no showing/update, update ImGui cursor from system cursor pos
         */
        void ImeWnd::NewFrame()
        {
            if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
            {
                POINT cursorPos;
                if (ui->IsMenuOpen(RE::CursorMenu::MENU_NAME))
                {
                    auto *meunCursor = RE::MenuCursor::GetSingleton();
                    ImGui::GetIO().AddMousePosEvent(meunCursor->cursorPosX, meunCursor->cursorPosY);
                }
                else if (GetCursorPos(&cursorPos) != FALSE)
                {
                    ImGui::GetIO().AddMousePosEvent(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
                }
            }
        }

        void ImeWnd::RenderIme() const
        {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            NewFrame();
            ImGui::NewFrame();

            m_pImeUi->RenderToolWindow();
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

        auto ImeWnd::OnNccCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) -> LRESULT
        {
            auto *pThis = static_cast<ImeWnd *>(lpCreateStruct->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, Utils::ToLongPtr(pThis));
            pThis->m_hWnd       = hWnd;
            pThis->m_hWndParent = lpCreateStruct->hwndParent;
            return TRUE;
        }

        void ImeWnd::OnCompositionResult(const std::wstring &compositionString)
        {
            if (State::GetInstance().IsSupportOtherMod())
            {
                std::wstring resultCopy(compositionString);
                ImeSupportUtils::BroadcastImeMessage(SimpleIME::SkseImeMessage::IME_COMPOSITION_RESULT,
                                                     resultCopy.data(), resultCopy.size() * sizeof(wchar_t));
            }
            else
            {
                Utils::SendStringToGame(compositionString);
            }
        }
    }
}