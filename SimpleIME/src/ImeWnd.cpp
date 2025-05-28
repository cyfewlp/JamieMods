#include "ImeWnd.hpp"

#include "ImeUI.h"
#include "Utils.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/log.h"
#include "configs/AppConfig.h"
#include "configs/CustomMessage.h"
#include "context.h"
#include "core/State.h"
#include "ime/ITextServiceFactory.h"
#include "ime/ImeManagerComposer.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "misc/freetype/imgui_freetype.h"

#include <d3d11.h>
#include <msctf.h>
#include <windows.h>
#include <windowsx.h>

// extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern auto ImGui_ImplWin32_GetDpiScaleForHwnd(void *hwnd) -> float;

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
ImeWnd::ImeWnd(Settings &settings) : m_settings(settings)
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
    m_pTextService->RegisterCallback(OnCompositionResult);
    m_pLangProfileUtil = new LangProfileUtil();
}

static void TryEnableImeWndDpiAware()
{
    log_info("Try to enable DPI aware for IME Wnd...");
    using PFN_SetThreadDpiAwarenessContext = DPI_AWARENESS_CONTEXT(WINAPI *)(DPI_AWARENESS_CONTEXT);

    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    auto    pSetThreadDpiAwarenessContext =
        reinterpret_cast<PFN_SetThreadDpiAwarenessContext>(GetProcAddress(hUser32, "SetThreadDpiAwarenessContext"));

    if (pSetThreadDpiAwarenessContext)
    {
        pSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        log_info("Enable DPI aware successful!");
    }
}

void ImeWnd::Initialize() noexcept(false)
{
    TryEnableImeWndDpiAware();
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

auto ImeWnd::IsImeWantMessage(const MSG &msg, ITfKeystrokeMgr *pKeystrokeMgr)
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

void ImeWnd::Start(HWND hWndParent, Settings *pSettings)
{
    log_info("Start ImeWnd Thread...");
    m_hWnd =
        CreateWindowExW(0, g_tMainClassName, L"Hide", WS_CHILD, 0, 0, 0, 0, hWndParent, nullptr, wc.hInstance, this);
    if (m_hWnd == nullptr)
    {
        throw SimpleIMEException("Create ImeWnd failed");
    }
    OnStart(pSettings);

    MSG msg = {};
    ZeroMemory(&msg, sizeof(msg));
    auto const &tsfSupport    = Tsf::TsfSupport::GetSingleton();
    auto const  pMessagePump  = tsfSupport.GetMessagePump();
    auto const  pKeystrokeMgr = tsfSupport.GetKeystrokeMgr();
    bool        done          = false;
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

        if (GetMessageW(&msg, nullptr, 0, 0) <= 0)
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

void ImeWnd::OnStart(Settings *pSettings)
{
    m_pTextService->OnStart(m_hWnd);
    Context::GetInstance()->SetHwndIme(m_hWnd);

    ImeManagerComposer::Init(this, m_hWndParent, pSettings);
    ApplyUiSettings(pSettings);
}

void ImeWnd::OnDpiChanged(HWND hWnd) const
{
    m_settings.dpiScale        = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);
    m_settings.wantRebuildFont = true;
}

void ImeWnd::RebuildFont(const Settings & settings)
{
    const auto &uiConfig   = AppConfig::GetConfig().GetAppUiConfig();
    float       pxFontSize = settings.dpiScale * settings.fontSize;

    auto &io = ImGui::GetIO();
    io.Fonts->Clear();
    if (!io.Fonts->AddFontFromFileTTF(uiConfig.EastAsiaFontFile().c_str(), pxFontSize))
    {
        io.Fonts->AddFontDefault();
    }

    // config font
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode                     = true;
    cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    io.Fonts->AddFontFromFileTTF(uiConfig.EmojiFontFile().c_str(), pxFontSize, &cfg);
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
        case WM_DPICHANGED: {
            if (pThis == nullptr) break;
            pThis->OnDpiChanged(hWnd);
            return S_OK;
        }
        case CM_EXECUTE_TASK: {
            TaskQueue::GetInstance().ExecuteImeThreadTasks();
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
        case WM_KILLFOCUS: {
            if (pThis == nullptr) break;
            pThis->m_fFocused = false;
            ImGui::GetIO().ClearInputKeys();
            log_info("IME window lost focus.");
            return S_OK;
        }
        default:
            // ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
            break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

auto ImeWnd::GetThis(HWND hWnd) -> ImeWnd *
{
    const auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (ptr == 0) return nullptr;
    return reinterpret_cast<ImeWnd *>(ptr);
}

void ImeWnd::InitImGui(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context, Settings &settings) const
    noexcept(false)
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
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Keyboard Controls
    io.ConfigNavMoveSetMousePos = false;
    GetClientRect(m_hWndParent, &rect);
    io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

    settings.dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);
    RebuildFont(settings);

    ImGuiStyle &style = ImGui::GetStyle();
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        style.WindowRounding              = 0.0F;
        style.Colors[ImGuiCol_WindowBg].w = 1.0F;
    }
    if (settings.dpiScale != 1.0F)
    {
        style.ScaleAllSizes(settings.dpiScale);
    }
    log_info("ImGui initialized!");
}

auto ImeWnd::OnCreate() -> LRESULT
{
    return S_OK;
}

void ImeWnd::ForwardKeyboardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
    static DWORD gameThread    = 0;
    static DWORD currentThread = GetCurrentThreadId();
    if (gameThread == 0)
    {
        gameThread = GetWindowThreadProcessId(m_hWndParent, nullptr);
    }
    if (gameThread != currentThread)
    {
        if (AttachThreadInput(gameThread, currentThread, TRUE) != FALSE)
        {
            switch (uMsg)
            {
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP: {
                    if (PostMessageA(m_hWndParent, uMsg, wParam, lParam) == FALSE)
                    {
                        log_debug("Post message to game failed.");
                    }
                    break;
                }
                default:
                    break;
            }
            AttachThreadInput(gameThread, currentThread, FALSE);
        }
    }
}

auto ImeWnd::SaveSettings() const -> void
{
    m_settings.fontSizeScale = ImGui::GetIO().FontGlobalScale;
    AppConfig::GetConfig().GetSettingsConfig().Set(m_settings);

    const auto *plugin         = SKSE::PluginDeclaration::GetSingleton();
    const auto  configFilePath = std::format(R"(Data\SKSE\Plugins\{}.ini)", plugin->GetName());
    AppConfig::SaveIni(configFilePath.c_str());
}

auto ImeWnd::OnDestroy() const -> LRESULT
{
    log_info("Save ui settings...");
    SaveSettings();

    log_info("Destroy IME Window");
    UnInitialize();
    PostQuitMessage(0);
    return S_OK;
}

auto ImeWnd::Focus() const -> void
{
    if (!IsFocused())
    {
        SetFocus(m_hWnd); // The return value only indicates which HWND had the focus previously.
    }
}

auto ImeWnd::SetTsfFocus(const bool focus) const -> bool
{
    return m_pTextService->OnFocus(focus);
}

auto ImeWnd::IsFocused() const -> bool
{
    return m_fFocused;
}

auto ImeWnd::SendMessageToIme(UINT uMsg, WPARAM wParam, LPARAM lParam) const -> bool
{
    if (m_hWnd == nullptr)
    {
        return false;
    }
    return SendMessageW(m_hWnd, uMsg, wParam, lParam) == S_OK;
}

auto ImeWnd::SendNotifyMessageToIme(UINT uMsg, WPARAM wParam, LPARAM lParam) const -> bool
{
    if (m_hWnd == nullptr)
    {
        return false;
    }
    return SendNotifyMessageW(m_hWnd, uMsg, wParam, lParam) != FALSE;
}

auto ImeWnd::GetImeThreadId() const -> DWORD
{
    return GetWindowThreadProcessId(m_hWnd, nullptr);
}

void ImeWnd::AbortIme() const
{
    if (State::GetInstance().HasAny(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
    {
        SetFocus(m_hWndParent);
    }
}

/**
 * If Game cursor no showing/update, update ImGui cursor from system cursor pos
 */
void ImeWnd::NewFrame(Settings &settings)
{
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        POINT cursorPos;
        if (ui->IsMenuOpen(RE::CursorMenu::MENU_NAME))
        {
            auto *menuCursor = RE::MenuCursor::GetSingleton();
            ImGui::GetIO().AddMousePosEvent(menuCursor->cursorPosX, menuCursor->cursorPosY);
        }
        else if (GetCursorPos(&cursorPos) != FALSE)
        {
            ImGui::GetIO().AddMousePosEvent(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
        }
    }
    if (settings.wantRebuildFont)
    {
        settings.wantRebuildFont = false;
        RebuildFont(settings);
    }
}

void ImeWnd::DrawIme(Settings &settings) const
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    NewFrame(settings);
    ImGui::NewFrame();

    ErrorNotifier::GetInstance().Show();
    m_pImeUi->RenderToolWindow(settings);
    m_pImeUi->Draw(settings);

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

void ImeWnd::ApplyUiSettings(Settings *pSettings) const
{
    m_pImeUi->ApplyUiSettings(*pSettings);
    ImeManagerComposer::GetInstance()->ApplyUiSettings(*pSettings);
}

auto ImeWnd::OnNccCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) -> LRESULT
{
    auto *pThis = static_cast<ImeWnd *>(lpCreateStruct->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, Utils::ToLongPtr(pThis));
    pThis->m_hWnd       = hWnd;
    pThis->m_hWndParent = lpCreateStruct->hwndParent;
    return TRUE;
}

inline void ImeWnd::OnCompositionResult(const std::wstring &compositionString)
{
    Utils::SendStringToGame(compositionString);
}
}
}