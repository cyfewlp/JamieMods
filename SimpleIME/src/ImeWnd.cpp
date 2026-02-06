#include "ImeWnd.hpp"

#include "ImeUI.h"
#include "Utils.h"
#include "common/imgui/ErrorNotifier.h"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "core/State.h"
#include "ime/ITextServiceFactory.h"
#include "ime/ImeController.h"
#include "ui/LanguageBar.h"

#include <codecvt>
#include <msctf.h>
#include <windows.h>
#include <windowsx.h>

namespace Ime
{
namespace Global
{
extern HINSTANCE g_hModule;
}

namespace
{
auto GetThis(HWND hWnd) -> ImeWnd *
{
    const auto ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (ptr == 0) return nullptr;
    return reinterpret_cast<ImeWnd *>(ptr);
}

bool RegisterImeWindowClass(const WNDPROC wndProc)
{
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_PARENTDC;
    wc.cbClsExtra    = 0;
    wc.lpfnWndProc   = wndProc;
    wc.cbWndExtra    = 0;
    wc.lpszClassName = g_tMainClassName;
    wc.hInstance     = Global::g_hModule;

    WNDCLASSEXW existingClass{};
    if (!GetClassInfoExW(Global::g_hModule, wc.lpszClassName, &existingClass))
    {
        return RegisterClassExW(&wc) != 0;
    }
    return true;
}
} // namespace

ImeWnd::~ImeWnd()
{
    UnInitialize();
    if (m_hWnd != nullptr)
    {
        DestroyWindow(m_hWnd);
    }
}

void ImeWnd::InitializeTextService()
{
    m_pTextService = TextServiceFactory::Create(m_settings.enableTsf);
    m_pTextService->RegisterCallback(OnCompositionResult);
}

static void TryEnableImeWndDpiAware()
{
    logger::info("Try to enable DPI aware for IME Wnd...");
    using PFN_SetThreadDpiAwarenessContext = DPI_AWARENESS_CONTEXT(WINAPI *)(DPI_AWARENESS_CONTEXT);

    HMODULE    hUser32 = GetModuleHandleW(L"user32.dll");
    const auto pSetThreadDpiAwarenessContext =
        reinterpret_cast<PFN_SetThreadDpiAwarenessContext>(GetProcAddress(hUser32, "SetThreadDpiAwarenessContext"));

    if (pSetThreadDpiAwarenessContext)
    {
        pSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        logger::info("Enable DPI aware successful!");
    }
}

// FIXME: ImeUI should not dependency ImeWnd!!!
void ImeWnd::Initialize() noexcept(false)
{
    TryEnableImeWndDpiAware();
    if (!RegisterImeWindowClass(WndProc))
    {
        throw SimpleIMEException("Can't register class");
    }

    auto &tsfSupport = Tsf::TsfSupport::GetSingleton();
    if (SUCCEEDED(tsfSupport.InitializeTsf(true)))
    {
        m_settings.enableTsf = false;
    }

    InitializeTextService();
    m_pImeWindow = std::make_unique<ImeWindow>();
    m_pImeUi     = std::make_unique<ImeUI>(this);

    // FIXME: may TSF is disabled!
    m_pInputMethodManager = new InputMethodManager();
    if (FAILED(m_pInputMethodManager->Initialize(tsfSupport.GetThreadMgr())))
    {
        throw SimpleIMEException("Can't initialize LangProfileUtil");
    }
    m_pImeUi->Initialize();
}

void ImeWnd::UnInitialize() const noexcept
{
    if (m_pTextService != nullptr)
    {
        m_pTextService->UnInitialize();
    }
    if (m_pInputMethodManager != nullptr)
    {
        m_pInputMethodManager->UnInitialize();
    }
}

void ImeWnd::CreateHost(HWND hWndParent, Settings &settings)
{
    logger::info("Start ImeWnd Thread...");
    m_hWnd = CreateWindowExW(
        0, g_tMainClassName, L"Hide", WS_CHILD, 0, 0, 0, 0, hWndParent, nullptr, Global::g_hModule, this
    );
    if (m_hWnd == nullptr)
    {
        throw SimpleIMEException("Create ImeWnd failed");
    }
    OnCreated(settings);
}

void ImeWnd::Run() const
{
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

    logger::info("Exit ImeWnd Thread...");
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

auto ImeWnd::SendMessageToIme(const UINT uMsg, const WPARAM wParam, const LPARAM lParam) const -> LRESULT
{
    if (m_hWnd == nullptr)
    {
        return FALSE;
    }
    return SendMessageW(m_hWnd, uMsg, wParam, lParam);
}

auto ImeWnd::SendNotifyMessageToIme(UINT uMsg, WPARAM wParam, LPARAM lParam) const -> bool
{
    if (m_hWnd == nullptr)
    {
        return true;
    }
    return SendNotifyMessageW(m_hWnd, uMsg, wParam, lParam) != FALSE;
}

auto ImeWnd::ActivateLanguageProfile(const GUID &guidProfile) const -> HRESULT
{
    return m_pInputMethodManager->ActivateProfile(guidProfile);
}

void ImeWnd::AbortIme() const
{
    if (State::GetInstance().HasAny(State::IN_CAND_CHOOSING, State::IN_COMPOSING))
    {
        SetFocus(m_hWndParent);
    }
}

void ImeWnd::DrawIme(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles)
{
    ImGui::PushFont(nullptr, settings.state.fontSize);
    {
        ErrorNotifier::GetInstance().Show();
        m_pImeWindow->Draw(m_pTextService->GetTextEditor(), m_pTextService->GetCandidateUi(), settings, m3Styles);

        {
            const auto &activeLang   = m_pInputMethodManager->GetActiveLangProfile();
            const auto &langProfiles = m_pInputMethodManager->GetLangProfiles();
            const auto  state        = LanguageBar::Draw(m_fWantToggleToolWindow, activeLang, langProfiles);
            if (LanguageBar::IsOpenSettings(state))
            {
                settings.appearance.showSettings = true;
            }
            m_fWantToggleToolWindow = false;

            if (LanguageBar::IsShowing(state))
            {
                m_pImeUi->DrawSettings(settings, m3Styles);
            }
        }
    }
    ImGui::PopFont();
}

void ImeWnd::ToggleToolWindow()
{
    m_fWantToggleToolWindow = true;
}

void ImeWnd::ApplyUiSettings(Settings &settings, ImGuiEx::M3::M3Styles &m3Styles) const
{
    m_pImeUi->ApplySettings(settings.appearance, m3Styles);
}

auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    logger::debug("Message: {:#X} {} {}", uMsg, wParam, lParam);
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
            return 0;
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
        case WM_IME_SETCONTEXT:
            lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUICANDIDATEWINDOW);
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        case WM_SETFOCUS:
            if (pThis == nullptr) break;
            pThis->m_fFocused = true;
            logger::info("IME window get focus.");
            return 0;
        case WM_KILLFOCUS: {
            if (pThis == nullptr) break;
            pThis->m_fFocused = false;
            ImGui::GetIO().ClearInputKeys();
            logger::info("IME window lost focus.");
            return 0;
        }
        case WM_CHAR: {
            if (pThis == nullptr) break;
            if (ImeController::GetInstance()->IsModEnabled() &&
                Core::State::GetInstance().Has(State::LANG_PROFILE_ACTIVATED))
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
                logger::error("TSF message pump failed irrecoverably (HRESULT: {:#x}). Exiting loop.", hr);
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

void ImeWnd::OnCreated(Settings &settings)
{
    logger::info("Ime window created, init TSF and core...");
    m_pTextService->OnStart(m_hWnd);

    ImeController::GetInstance()->Init(this, m_hWndParent, settings);
}

auto ImeWnd::OnDestroy() const -> LRESULT
{
    logger::info("Destroy IME Window");
    ImeController::GetInstance()->Shutdown();
    UnInitialize();
    PostQuitMessage(0);
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
                        logger::debug("Post message to game failed.");
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
} // namespace Ime
