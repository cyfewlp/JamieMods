#include "ImeWnd.hpp"
#include "ImeUI.h"
#include "common/log.h"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "ime/ITextServiceFactory.h"

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

        void SendStringToSkyrim(const std::wstring &compositionString)
        {
            log_debug("Ready result string to Skyrim...");
            auto *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
            auto *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
            if (pInterfaceStrings == nullptr || pFactoryManager == nullptr)
            {
                log_warn("Can't send string to Skyrim may game already close?");
                return;
            }

            const auto *pFactory =
                pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);
            if (pFactory == nullptr)
            {
                log_warn("Can't send string to Skyrim may game already close?");
                return;
            }

            // Start send message
            RE::BSFixedString menuName = pInterfaceStrings->topMenu;
            for (wchar_t wchar : compositionString)
            {
                uint32_t const code = wchar;
                if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
                {
                    continue;
                }
                auto *pCharEvent            = new GFxCharEvent(code, 0);
                auto *pScaleFormMessageData = pFactory->Create();
                if (pScaleFormMessageData == nullptr)
                {
                    log_error("Unable create BSTDerivedCreator.");
                    return;
                }
                pScaleFormMessageData->scaleformEvent = pCharEvent;
                log_debug("send code {:#x} to Skyrim", code);
                RE::UIMessageQueue::GetSingleton()->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kScaleformEvent,
                                                               pScaleFormMessageData);
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
            m_pTextService->RegisterCallback(SendStringToSkyrim);
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
            m_pImeUi               = std::make_unique<ImeUI>(appConfig.GetAppUiConfig(), m_pTextService.get());

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
            m_pTextService->UnInitialize();
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
            Focus();
            m_pLangProfileUtil->ActivateProfile(&GUID_NULL);
            m_pTextService->OnStart(m_hWnd);
            MSG msg = {};
            ZeroMemory(&msg, sizeof(msg));
            auto const &tsfSupport    = Tsf::TsfSupport::GetSingleton();
            auto const  pMessagePump  = tsfSupport.GetMessagePump();
            auto const  pKeystrokeMgr = tsfSupport.GetKeystrokeMgr();
            while (TRUE)
            {
                BOOL fEaten;
                int  fResult;
                if (pMessagePump->GetMessage(&msg, nullptr, 0, 0, &fResult) != S_OK)
                {
                    fResult = -1;
                }
                else if (msg.message == WM_KEYDOWN)
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

                if (fResult <= 0) break;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            log_info("Exit ImeWnd Thread...");
        }

        auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            const ImeWnd *pThis = GetThis(hWnd);
            if (pThis != nullptr)
            {
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
                case CM_ACTIVATE_PROFILE: {
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
            const auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (ptr == 0) return nullptr;
            return reinterpret_cast<ImeWnd *>(ptr);
        }

        void ThemeConfig(const AppUiConfig &uiConfig)
        {
            auto   &style                   = ImGui::GetStyle();
            ImVec4 *colors                  = style.Colors;

            colors[ImGuiCol_WindowBg]       = ImColor(uiConfig.WindowBgColor());
            colors[ImGuiCol_Border]         = ImColor(uiConfig.WindowBorderColor());
            colors[ImGuiCol_Text]           = ImColor(uiConfig.TextColor());
            const auto btnCol               = uiConfig.BtnColor() & 0x00FFFFFF | 0x9A000000;
            colors[ImGuiCol_Button]         = ImColor(btnCol);
            colors[ImGuiCol_ButtonHovered]  = ImColor(btnCol & 0x00FFFFFF | 0x66000000);
            colors[ImGuiCol_ButtonActive]   = ImColor(btnCol & 0x00FFFFFF | 0xAA000000);
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

        /**
         * If Game cursor no showing/update, update ImGui cursor from system cursor pos
         */
        void NewFrame()
        {
            if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
            {
                if (!ui->IsShowingMenus() || !ui->IsMenuOpen(RE::CursorMenu::MENU_NAME))
                {
                    POINT cursorPos;
                    auto &io = ImGui::GetIO();
                    if (GetCursorPos(&cursorPos))
                    {
                        io.AddMousePosEvent(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
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
            // may need more test
            if (Context::GetInstance()->IsGameLoading())
            {
                return false;
            }

            auto &imeState  = m_pTextService->GetState();
            auto  isImeOpen = m_pLangProfileUtil->IsAnyProfileActivated();
            if (!isImeOpen || imeState.any(ImeState::IN_ALPHANUMERIC))
            {
                return false;
            }
            auto *head         = *events;
            auto  sourceDevice = head->device;
            if (sourceDevice == RE::INPUT_DEVICE::kKeyboard)
            {
                if (imeState.any(ImeState::IN_CAND_CHOOSING, ImeState::IN_COMPOSING))
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