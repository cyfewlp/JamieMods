#include "ImeWnd.hpp"

#include "FakeDirectInputDevice.h"
#include "ImeUI.h"
#include "ScaleformHook.h"
#include "Utils.h"
#include "common/log.h"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "ime/ITextServiceFactory.h"

#include <basetsd.h>
#include <cstdint>
#include <d3d11.h>
#include <gsl/pointers>
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

        class ConsoleMenuListener final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
            HWND m_hWnd;

        public:
            explicit ConsoleMenuListener(const HWND hWnd) : m_hWnd(hWnd)
            {
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent *a_event,
                                                  RE::BSTEventSource<RE::MenuOpenCloseEvent> *) override
            {
                if (a_event->menuName == RE::Console::MENU_NAME)
                {
                    Hooks::ScaleformAllowTextInput::AllowTextInput(a_event->opening);
                }
                return RE::BSEventNotifyControl::kContinue;
            }
        };

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
                DestroyWindow(m_hWnd);
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
            m_pTextService->RegisterCallback(Utils::SendStringToGame);
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
            m_pImeUi               = std::make_unique<ImeUI>(appConfig.GetAppUiConfig(), this, m_pTextService.get());

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

        void ImeWnd::Start(HWND hWndParent)
        {
            log_info("Start ImeWnd Thread...");
            DWORD dwExStyle = 0;
            DWORD dwStyle   = WS_CHILD;
            m_hWnd          = ::CreateWindowExW(dwExStyle, g_tMainClassName, L"Hide", dwStyle, //
                                                0, 0, 0, 0,                                    //
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
            while (TRUE)
            {
                BOOL fEaten  = FALSE;
                int  fResult = 0;
                if (pMessagePump->GetMessage(&msg, nullptr, 0, 0, &fResult) != S_OK)
                {
                    fResult = -1;
                }
                else
                {
                    if (m_pTextService->HasState(ImeState::IME_DISABLED))
                    {
                        continue;
                    }
                    if (msg.message == WM_KEYDOWN)
                    {
                        // does an ime want it?
                        if (pKeystrokeMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                            pKeystrokeMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
                        {
                            continue;
                        }
                    }
                    else if (msg.message == WM_KEYUP)
                    {
                        // does an ime want it?
                        if (pKeystrokeMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                            pKeystrokeMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
                        {
                            continue;
                        }
                    }
                }

                if (fResult <= 0) break;

                if (!TranslateAccelerator(m_hWnd, m_hAccelTable, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            log_info("Exit ImeWnd Thread...");
        }

        static std::unique_ptr<ConsoleMenuListener> g_pMenuOpenCloseEventSink(nullptr);

        void ImeWnd::OnStart()
        {
            if (!AppConfig::GetConfig().AlwaysActiveIme())
            {
                if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
                {
                    g_pMenuOpenCloseEventSink.reset(new ConsoleMenuListener(m_hWnd));
                    ui->AddEventSink(g_pMenuOpenCloseEventSink.get());
                }
            }

            Focus();
            // m_pLangProfileUtil->ActivateProfile(&GUID_NULL);
            m_pTextService->OnStart(m_hWnd);
            Context::GetInstance()->SetHwndIme(m_hWnd);

            ACCEL accelTable[] = {
                {FVIRTKEY | FCONTROL, 'C', ID_EDIT_COPY },
                {FVIRTKEY | FCONTROL, 'V', ID_EDIT_PASTE},
            };
            m_hAccelTable = CreateAcceleratorTableW(accelTable, 2);
        }

        void ImeWnd::ForwardKeyboardAndImeMessage(HWND hWndTarget, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            bool isForward = false;
            switch (uMsg)
            {
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYUP:
                case WM_SYSKEYDOWN:
                    isForward = true;
                    break;
                default:
                    break;
            }
            if (isForward && SendMessageA(hWndTarget, uMsg, wParam, lParam) != S_OK)
            {
                log_debug("Failed Forward Message {}", uMsg);
            }
        }

        auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            ImeWnd *pThis = GetThis(hWnd);
            if (pThis != nullptr)
            {
                ForwardKeyboardAndImeMessage(pThis->m_hWndParent, uMsg, wParam, lParam);
                if (pThis->m_pTextService->ProcessImeMessage(hWnd, uMsg, wParam, lParam))
                {
                    return S_OK;
                }
            }

            switch (uMsg)
            {
                case WM_NCCREATE: {
                    auto *lpCs   = reinterpret_cast<LPCREATESTRUCT>(lParam);
                    auto *pThis1 = static_cast<ImeWnd *>(lpCs->lpCreateParams);
                    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis1));
                    // set the window handle
                    pThis1->m_hWnd       = hWnd;
                    pThis1->m_hWndParent = lpCs->hwndParent;
                    break;
                }
                case WM_CREATE: {
                    if (pThis == nullptr) break;
                    HIMC hIMC = ImmCreateContext();
                    ImmAssociateContextEx(hWnd, hIMC, IACE_IGNORENOCONTEXT);
                    return pThis->OnCreate();
                }
                case WM_DESTROY: {
                    ImmAssociateContextEx(hWnd, nullptr, IACE_DEFAULT);
                    if (pThis == nullptr) break;
                    return pThis->OnDestroy();
                }
                case CM_IME_ENABLE: {
                    if (pThis == nullptr) break;
                    pThis->EnableIme(wParam != FALSE);
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
                    break;
                case WM_KILLFOCUS:
                    if (pThis == nullptr) break;
                    pThis->m_fFocused = false;
                    break;
                case WM_COMMAND: {
                    if (pThis == nullptr) break;
                    switch (LOWORD(wParam))
                    {
                        case ID_EDIT_PASTE:
                            log_trace("Ready paste Text...");
                            // if (!Utils::ConvertClipboardText(pThis->m_hWndParent))
                            // {
                            //     log_error("Can't paste Text! {}", GetLastError());
                            // }
                            break;
                        case ID_EDIT_COPY:
                            // PerformCopy();
                            break;
                        default:
                            break;
                    }
                    break;
                }
                default:
                    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
                    break;
            }
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }

        auto ImeWnd::GetThis(const HWND hWnd) -> ImeWnd *
        {
            const auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (ptr == 0) return nullptr;
            return reinterpret_cast<ImeWnd *>(ptr);
        }

        static void ThemeConfig(const AppUiConfig &uiConfig)
        {
            auto &style                     = ImGui::GetStyle();
            auto  colors                    = std::span(style.Colors);

            colors[ImGuiCol_WindowBg]       = ImColor(uiConfig.WindowBgColor());
            colors[ImGuiCol_Border]         = ImColor(uiConfig.WindowBorderColor());
            colors[ImGuiCol_Text]           = ImColor(uiConfig.TextColor());
            const auto btnCol               = (uiConfig.BtnColor() & 0x00FFFFFF) | 0x9A000000;
            colors[ImGuiCol_Button]         = ImColor(btnCol);
            colors[ImGuiCol_ButtonHovered]  = ImColor((btnCol & 0x00FFFFFF) | 0x66000000);
            colors[ImGuiCol_ButtonActive]   = ImColor((btnCol & 0x00FFFFFF) | 0xAA000000);
            colors[ImGuiCol_Header]         = colors[ImGuiCol_Button];
            colors[ImGuiCol_HeaderHovered]  = colors[ImGuiCol_ButtonHovered];
            colors[ImGuiCol_HeaderActive]   = colors[ImGuiCol_ButtonActive];
            colors[ImGuiCol_FrameBg]        = colors[ImGuiCol_Button];
            colors[ImGuiCol_FrameBgHovered] = colors[ImGuiCol_ButtonHovered];
            colors[ImGuiCol_FrameBgActive]  = colors[ImGuiCol_ButtonActive];
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
            ThemeConfig(AppConfig::GetConfig().GetAppUiConfig());
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

        auto ImeWnd::OnDestroy() const -> LRESULT
        {
            UnInitialize();
            PostQuitMessage(0);
            return S_OK;
        }

        void ImeWnd::Focus() const
        {
            if (m_pTextService->HasNoStates(ImeState::IME_DISABLED))
            {
                log_debug("focus to ime wnd");
                ::SetFocus(m_hWnd);
            }
        }

        auto ImeWnd::IsFocused() const -> bool
        {
            return m_fFocused;
        }

        auto ImeWnd::SendMessage_(UINT uMsg, WPARAM wparam, LPARAM lparam) const -> LRESULT
        {
            return SendMessageW(m_hWnd, uMsg, wparam, lparam);
        }

        auto ImeWnd::EnableIme(bool enable) const -> void
        {
            m_pTextService->Enable(enable);
            if (enable && !m_fFocused)
            {
                Focus();
            }
        }

        auto ImeWnd::EnableMod(bool enable) const -> bool
        {
            EnableIme(enable);

            auto *fakeKeyboard = Hooks::FakeDirectInputDevice::GetInstance();
            if (enable)
            {
                if (fakeKeyboard != nullptr && SUCCEEDED(fakeKeyboard->TrySetCooperativeLevel(m_hWndParent)))
                {
                    return ::SetFocus(m_hWnd) != nullptr;
                }
            }
            else
            {
                if (::SetFocus(m_hWndParent) != nullptr && fakeKeyboard != nullptr)
                {
                    return SUCCEEDED(fakeKeyboard->TryRestoreCooperativeLevel(m_hWndParent));
                }
            }

            return false;
        }

        void ImeWnd::AbortIme() const
        {
            if (m_pTextService->HasAnyStates(ImeState::IN_CAND_CHOOSING, ImeState::IN_COMPOSING))
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
                if (!ui->IsMenuOpen(RE::CursorMenu::MENU_NAME))
                {
                    POINT cursorPos;
                    if (GetCursorPos(&cursorPos) != FALSE)
                    {
                        ImGui::GetIO().AddMousePosEvent(static_cast<float>(cursorPos.x),
                                                        static_cast<float>(cursorPos.y));
                    }
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

        static auto IsModifierKeyDown() -> bool
        {
            auto isKeyDown = [](const uint32_t code) { return (GetKeyState(code) & 0x8000) != 0; };
            return isKeyDown(VK_CONTROL) || isKeyDown(VK_SHIFT) //
                   || isKeyDown(VK_MENU) || isKeyDown(VK_LWIN)  //
                   || isKeyDown(VK_RWIN);
        }

        auto ImeWnd::IsDiscardGameInputEvents(__in RE::InputEvent **events) const -> bool
        {
            if (events == nullptr || *events == nullptr)
            {
                return false;
            }
            auto *head         = *events;
            auto  sourceDevice = head->device;
            if (sourceDevice != RE::INPUT_DEVICE::kKeyboard)
            {
                return false;
            }
            if (const auto *buttonEvent = head->AsButtonEvent(); nullptr != buttonEvent)
            {
                const auto code = buttonEvent->GetIDCode();
                if (IsImeDisabledOrGameLoading() || IsModifierKeyDown() || IsImeNotActive())
                {
                    return false;
                }
                if (IsImeWantCaptureInput() || IsWillTriggerIme(code))
                {
                    return true;
                }
            }

            return false;
        }

        constexpr auto ImeWnd::IsImeDisabledOrGameLoading() const -> bool
        {
            return m_pTextService->HasState(ImeState::IME_DISABLED) || Context::GetInstance()->IsGameLoading();
        }

        constexpr auto ImeWnd::IsImeNotActive() const -> bool
        {
            auto isImeOpen = m_pLangProfileUtil->IsAnyProfileActivated();
            return !isImeOpen || m_pTextService->HasState(ImeState::IN_ALPHANUMERIC);
        }

        constexpr auto ImeWnd::IsImeWantCaptureInput() const -> bool
        {
            return m_pTextService->HasAnyStates(ImeState::IN_CAND_CHOOSING, ImeState::IN_COMPOSING);
        }

        auto ImeWnd::IsWillTriggerIme(const std::uint32_t code) -> bool
        {
            bool result = false;
            using Key   = RE::BSKeyboardDevice::Keys::Key;
            result |= code >= Key::kQ && code <= Key::kP;
            result |= code >= Key::kA && code <= Key::kL;
            result |= code >= Key::kZ && code <= Key::kM;
            return result;
        }
    }
}