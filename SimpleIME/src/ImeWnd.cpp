#include "ImeWnd.hpp"

#include "ImeUI.h"
#include "Utils.h"
#include "common/WCharUtils.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "hooks/Hooks.hpp"
#include "ime/ITextServiceFactory.h"
#include "ime/ImeController.h"

#include <codecvt>
#include <msctf.h>
#include <windows.h>
#include <windowsx.h>

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

void ImeWnd::InitializeTextService()
{
    ITextService *pTextService = nullptr;
    ITextServiceFactory::CreateInstance(m_settings.enableTsf, &pTextService);
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

// FIXME: ImeUI should not dependency ImeWnd!!!
void ImeWnd::Initialize(ImGuiEx::M3::M3Styles &styles) noexcept(false)
{
    TryEnableImeWndDpiAware();
    wc.hInstance = GetModuleHandle(nullptr);
    if (RegisterClassExW(&wc) == 0U)
    {
        throw SimpleIMEException("Can't register class");
    }

    InitializeTextService();
    m_pImeWindow = std::make_unique<ImeWindow>(*this, m_pTextService.get(), styles);
    m_pImeUi     = std::make_unique<ImeUI>(this, styles);

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

    if (m_settings.enableTsf)
    {
        TsfMessageLoop();
    }
    else
    {
        MSG msg = {};
        ZeroMemory(&msg, sizeof(msg));
        bool done = false;
        while (!done)
        {
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    done = TRUE;
                    break;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            MsgWaitForMultipleObjects(0, nullptr, FALSE, 10, QS_ALLINPUT);
        }
    }

    log_info("Exit ImeWnd Thread...");
}

void ImeWnd::OnStart(Settings *pSettings)
{
    m_pTextService->OnStart(m_hWnd);

    ImeController::Init(this, m_hWndParent, pSettings);
    ApplyUiSettings(pSettings);
}

auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    // log_debug("Message: {} {} {}", uMsg, wParam, lParam);
    ImeWnd *pThis = GetThis(hWnd);
    if (pThis != nullptr)
    {
        pThis->ForwardKeyboardMessage(uMsg, wParam, lParam);
        if (pThis->m_pTextService->ProcessImeMessage(hWnd, uMsg, wParam, lParam))
        {
            return 0;
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
            ImmAssociateContextEx(hWnd, nullptr, IACE_DEFAULT);
            return pThis->OnDestroy();
        }
        case WM_DPICHANGED: {
            if (pThis == nullptr) break;
            // pThis->OnDpiChanged(hWnd);
            return 0;
        }
        case CM_EXECUTE_TASK: {
            TaskQueue::GetInstance().ExecuteImeThreadTasks();
            return 0;
        }
        case CM_ACTIVATE_PROFILE: {
            if (pThis == nullptr) break;
            pThis->m_pLangProfileUtil->ActivateProfile(reinterpret_cast<GUID *>(lParam));
            return 0;
        }
        case WM_IME_SETCONTEXT:
            lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUICANDIDATEWINDOW);
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        case WM_SETFOCUS:
            if (pThis == nullptr) break;
            pThis->m_fFocused = true;
            log_info("IME window get focus.");
            return 0;
        case WM_KILLFOCUS: {
            if (pThis == nullptr) break;
            pThis->m_fFocused = false;
            ImGui::GetIO().ClearInputKeys();
            log_info("IME window lost focus.");
            return 0;
        }
        case WM_CHAR: {
            if (pThis == nullptr) break;
            if (Core::State::GetInstance().Has(State::LANG_PROFILE_ACTIVATED))
            {
                Utils::SendStringToGame(std::wstring(1, wParam));
            }
            return 0;
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

auto ImeWnd::OnCreate() -> LRESULT
{
    return 0;
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

auto ImeWnd::OnDestroy() const -> LRESULT
{
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
        return true;
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

void ImeWnd::DrawIme(Settings &settings) const
{
    ImGui::PushFont(nullptr, settings.state.fontSize);
    {
        ErrorNotifier::GetInstance().Show();
        m_pImeWindow->Draw(settings);
        m_pImeUi->DrawToolWindow(settings);
    }
    ImGui::PopFont();
}

void ImeWnd::ShowToolWindow() const
{
    m_pImeUi->ShowToolWindow();
}

void ImeWnd::ApplyUiSettings(Settings *pSettings) const
{
    m_pImeUi->ApplySettings(*pSettings);
    ImeController::GetInstance()->ApplyUiSettings(*pSettings);
}

auto ImeWnd::OnNccCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) -> LRESULT
{
    auto *pThis = static_cast<ImeWnd *>(lpCreateStruct->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, Utils::ToLongPtr(pThis));
    pThis->m_hWnd       = hWnd;
    pThis->m_hWndParent = lpCreateStruct->hwndParent;
    return TRUE;
}

void ImeWnd::TsfMessageLoop()
{
    auto const &tsfSupport   = Tsf::TsfSupport::GetSingleton();
    auto const  pMessagePump = tsfSupport.GetMessagePump();
    const auto &keystrokeMgr = tsfSupport.GetKeystrokeMgr();
    MSG         msg{};
    BOOL        fResult = FALSE;
    while (true)
    {
        HRESULT hr = pMessagePump->PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE, &fResult);

        if (FAILED(hr))
        {
            if (hr == E_FAIL || hr == E_UNEXPECTED)
            {
                log_error("TSF message pump failed irrecoverably (HRESULT: {:#x}). Exiting loop.", hr);
                break;
            }
            MsgWaitForMultipleObjectsEx(0, nullptr, 1, QS_INPUT, MWMO_INPUTAVAILABLE);
            continue;
        }

        if (!fResult)
        {
            MsgWaitForMultipleObjectsEx(0, nullptr, 10, QS_INPUT, MWMO_INPUTAVAILABLE);
            continue;
        }
        if (msg.message == WM_QUIT)
        {
            break;
        }

        BOOL fEaten = FALSE;
        if (msg.message == WM_KEYDOWN)
        {
            if (SUCCEEDED(keystrokeMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten)) && fEaten &&
                SUCCEEDED(keystrokeMgr->KeyDown(msg.wParam, msg.lParam, &fEaten)) && fEaten)
            {
                continue;
            }
        }
        else if (msg.message == WM_KEYUP)
        {
            if (SUCCEEDED(keystrokeMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten)) && fEaten &&
                SUCCEEDED(keystrokeMgr->KeyUp(msg.wParam, msg.lParam, &fEaten)) && fEaten)
            {
                continue;
            }
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

inline void ImeWnd::OnCompositionResult(const std::wstring &compositionString)
{
    Utils::SendStringToGame(compositionString);
}
}
}