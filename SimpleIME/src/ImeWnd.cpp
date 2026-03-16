#include "ImeWnd.hpp"

#include "configs/CustomMessage.h"
#include "core/State.h"
#include "i18n/translator_manager.h"
#include "ime/ITextServiceFactory.h"
#include "ime/ImeController.h"
#include "imgui_impl_win32.h"
#include "imguiex/ErrorNotifier.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "log.h"
#include "menu/MenuNames.h"
#include "ui/ImeUI.h"
#include "ui/LanguageBar.h"
#include "utils/Utils.h"

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

auto RegisterImeWindowClass(WNDPROC wndProc) -> bool
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
    if (GetClassInfoExW(Global::g_hModule, wc.lpszClassName, &existingClass) == FALSE)
    {
        return RegisterClassExW(&wc) != 0;
    }
    return true;
}

//! DPI awareness for support different monitor that has different physical DPI, avoid blurry when move between
//! monitors.
// ！@see WndProc::WM_DIPCHANGED
void TryEnableImeWndDpiAware()
{
    logger::info("Try to enable DPI aware for IME Wnd...");
    using PFN_SetThreadDpiAwarenessContext = DPI_AWARENESS_CONTEXT(WINAPI *)(DPI_AWARENESS_CONTEXT);

    HMODULE    hUser32 = GetModuleHandleW(L"user32.dll");
    const auto pSetThreadDpiAwarenessContext =
        reinterpret_cast<PFN_SetThreadDpiAwarenessContext>(GetProcAddress(hUser32, "SetThreadDpiAwarenessContext"));

    if (pSetThreadDpiAwarenessContext != nullptr)
    {
        pSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        logger::info("Enable DPI aware successful!");
    }
}

//! Try hide game cursor when ImGui want capture mouse, and show game cursor when ImGui release mouse capture.
void AutoToggleGameCursorIfNeeded(bool &justWantCaptureMouse)
{
    auto      &io                = ImGui::GetIO();
    const bool cWantCaptureMouse = io.WantCaptureMouse;
    io.MouseDrawCursor           = cWantCaptureMouse;
    if (justWantCaptureMouse == cWantCaptureMouse)
    {
        return;
    }
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        if (auto toolWindow = ui->GetMenu(ToolWindowMenuName); toolWindow != nullptr)
        {
            if (cWantCaptureMouse)
            {
                toolWindow->menuFlags.reset(RE::UI_MENU_FLAGS::kUsesCursor);
            }
            else
            {
                toolWindow->menuFlags.set(RE::UI_MENU_FLAGS::kUsesCursor);
            }
        }
    }
    justWantCaptureMouse = cWantCaptureMouse;
}

/**
 * The toolwindow and ImeWnd handle the shortcut at the same time.
 * - ToolWindow already released ? -> ImeWnd handle shortcut to response the open ToolWindow request.
 *                      alive    ? -> ToolWindow handle shortcut to response the open/pin/unpin/close ToolWindow request.
 * - Debounce timer passed? -> If toolwindow is alive, close it and release translator; otherwise, open toolwindow and load translator.
 */
inline void ManageImeUIOnDemand(std::unique_ptr<UI::ImeUI> &imeUI, DebounceTimer &debounceTimer, const Settings &settings)
{
    bool shouldOpenImeUI = false;
    if (imeUI == nullptr && ImGui::IsKeyChordPressed(settings.shortcut))
    {
        shouldOpenImeUI = true;
    }
    // pass the first call
    if (!debounceTimer.IsWaiting() || debounceTimer.Check())
    {
        if (imeUI != nullptr)
        {
            imeUI.reset();
        }
        else if (shouldOpenImeUI)
        {
            imeUI = std::make_unique<UI::ImeUI>(settings.shortcut, settings.appearance.language);
        }
    }
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
    m_pTextService = TextServiceFactory::Create(m_fEnabledTsf);
    m_pTextService->RegisterCallback([](std::wstring_view compositionString) static -> void {
        Skyrim::SendUiString(compositionString);
    });
}

void ImeWnd::Initialize(const bool enableTsf) noexcept(false)
{
    TryEnableImeWndDpiAware();
    if (!RegisterImeWindowClass(WndProc))
    {
        throw SimpleIMEException("Can't register class");
    }
    m_fEnabledTsf = enableTsf;

    auto &tsfSupport = Tsf::TsfSupport::GetSingleton();
    if (FAILED(tsfSupport.InitializeTsf(true)))
    {
        m_fEnabledTsf = false;
    }

    InitializeTextService();
    m_pImeWindow = std::make_unique<ImeWindow>();

    m_pInputMethodManager = new InputMethodManager();
    if (FAILED(m_pInputMethodManager->Initialize(tsfSupport.GetThreadMgr())))
    {
        throw SimpleIMEException("Can't initialize LangProfileUtil");
    }
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
    m_hWnd = CreateWindowExW(0, g_tMainClassName, L"Hide", WS_CHILD, 0, 0, 0, 0, hWndParent, nullptr, Global::g_hModule, this);
    if (m_hWnd == nullptr)
    {
        throw SimpleIMEException("Create ImeWnd failed");
    }
    OnCreated(settings);
}

void ImeWnd::Run() const
{
    if (m_fEnabledTsf)
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
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) != FALSE)
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

auto ImeWnd::FocusTextService(const bool focus) const -> bool
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

void ImeWnd::Draw(Settings &settings)
{
    if (m_fWantUpdateUiScale)
    {
        m_fWantUpdateUiScale = false;
        ImGuiEx::M3::Context::GetM3Styles().UpdateScaling(m_uiScale);
    }
    m_pTextService->UpdateIfDirty();

    AutoToggleGameCursorIfNeeded(m_fJustWantCaptureMouse);

    ManageImeUIOnDemand(m_imeUI, m_translatorLoadDebounceTimer, settings);

    {
        auto      &m3Styles  = ImGuiEx::M3::Context::GetM3Styles();
        const auto fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
        ErrorNotifier::GetInstance().Show();
        m_pImeWindow->Draw(m_pTextService->GetCompositionInfo(), m_pTextService->GetCandidateUi(), settings);

        const auto &activeLang   = m_pInputMethodManager->GetActiveLangProfile();
        const auto &langProfiles = m_pInputMethodManager->GetLangProfiles();

        if (m_imeUI != nullptr)
        {
            if (m_imeUI->Draw(activeLang, langProfiles, settings))
            {
                m_translatorLoadDebounceTimer.Poke();
            }
        }
    }
}

auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    // logger::debug("Message: {:#X} {} {}", uMsg, wParam, lParam);
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
        case WM_SETTINGCHANGE: {
            if (pThis == nullptr) break;
            pThis->m_uiScale            = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);
            pThis->m_fWantUpdateUiScale = true;
            return 0;
        }
        case WM_DPICHANGED: {
            if (pThis == nullptr) break;
            const float g_dpi           = HIWORD(wParam);
            const auto  scale           = g_dpi / USER_DEFAULT_SCREEN_DPI;
            pThis->m_uiScale            = scale;
            pThis->m_fWantUpdateUiScale = true;
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
            logger::debug("IME window get focus.");
            return 0;
        case WM_KILLFOCUS: {
            if (pThis == nullptr) break;
            pThis->m_fFocused = false;
            ImGui::GetIO().ClearInputKeys();
            logger::debug("IME window lost focus.");
            // FIXME: crash when focus leaves ImeWnd during active composition via an OS-level window switch
            // (e.g. Win+Shift+S snipping tool at the right timing). Root cause and exact site are unknown
            // because no PDB is generated for that build configuration. Hypothesis: TSF/IMM32 posts a
            // composition-end message that arrives after the ImeWnd or its related objects have been
            // partially torn down, causing a use-after-free or null-deref.
            // Repro: open any text field, start composing CJK text, immediately press Win+Shift+S and
            // quickly click away to another window before the screenshot tool captures.
            return 0;
        }
        case WM_CHAR: {
            if (pThis == nullptr) break;
            const auto &state = Core::State::GetInstance();
            if (ImeController::GetInstance()->IsModEnabled() && (state.NotHas(State::IME_DISABLED) && state.Has(State::LANG_PROFILE_ACTIVATED)))
            {
                const auto wcharCode = static_cast<std::uint32_t>(wParam);

                // The direct keys(arrow keys, etc.) are not sent via WM_CHAR messages
                static const auto ignoredKeys = {VK_TAB, VK_RETURN, VK_BACK, VK_ESCAPE /*, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT*/};
                if (std::ranges::find(ignoredKeys, wcharCode) == std::end(ignoredKeys))
                {
                    const std::wstring wstring(1, LOWORD(wParam));
                    Skyrim::SendUiString(wstring);
                }
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
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
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
                logger::error("TSF message pump failed irrecoverably (HRESULT: {:#x}). Exiting loop.", hr);
                break;
            }
            MsgWaitForMultipleObjectsEx(0, nullptr, 1, QS_INPUT, MWMO_INPUTAVAILABLE);
            continue;
        }

        if (fResult == FALSE)
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
            if (SUCCEEDED(keystrokeMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten)) && (fEaten != FALSE) &&
                SUCCEEDED(keystrokeMgr->KeyDown(msg.wParam, msg.lParam, &fEaten)) && (fEaten != FALSE))
            {
                continue;
            }
        }
        else if (msg.message == WM_KEYUP)
        {
            if (SUCCEEDED(keystrokeMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten)) && (fEaten != FALSE) &&
                SUCCEEDED(keystrokeMgr->KeyUp(msg.wParam, msg.lParam, &fEaten)) && (fEaten != FALSE))
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
    m_gameThreadId = GetWindowThreadProcessId(m_hWndParent, nullptr);
    m_pTextService->OnStart(m_hWnd);
    m_uiScale            = ImGui_ImplWin32_GetDpiScaleForHwnd(m_hWnd);
    m_fWantUpdateUiScale = true;
    float uiScale        = settings.appearance.zoom;
    uiScale              = static_cast<float>(align_to(static_cast<int>(uiScale * 100), Settings::ZOOM_STEP_PERCENT)) / 100.F;
    if (uiScale > 0.0F)
    {
        m_uiScale = std::clamp(uiScale, Settings::ZOOM_MIN, Settings::ZOOM_MAX);
    }

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
    const DWORD currentThread = GetCurrentThreadId();
    if (m_gameThreadId != currentThread)
    {
        if (AttachThreadInput(m_gameThreadId, currentThread, TRUE) != FALSE)
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
            AttachThreadInput(m_gameThreadId, currentThread, FALSE);
        }
    }
}
} // namespace Ime
