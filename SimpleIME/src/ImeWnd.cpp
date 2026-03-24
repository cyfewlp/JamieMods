#include "ImeWnd.hpp"

#include "configs/CustomMessage.h"
#include "core/State.h"
#include "i18n/translator_manager.h"
#include "icons.h"
#include "ime/ITextServiceFactory.h"
#include "ime/ImeController.h"
#include "imgui_impl_win32.h"
#include "imguiex/ErrorNotifier.h"
#include "imguiex/Material3.h"
#include "imguiex/imguiex_enum_wrap.h"
#include "imguiex/imguiex_m3.h"
#include "log.h"
#include "menu/MenuNames.h"
#include "ui/ImeOverlay.h"
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
    wc.lpszClassName = g_MainClassName;
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
inline void ManageImeOverlayOnDemand(std::unique_ptr<UI::ImeOverlay> &imeOverlay, DebounceTimer &debounceTimer, const Settings &settings)
{
    bool shouldOpenImeOverlay = false;
    if (imeOverlay == nullptr && ImGui::IsKeyChordPressed(settings.shortcut))
    {
        shouldOpenImeOverlay = true;
    }
    // pass the first call
    if (!debounceTimer.IsWaiting() || debounceTimer.Check())
    {
        if (imeOverlay != nullptr)
        {
            imeOverlay.reset();
        }
        else if (shouldOpenImeOverlay)
        {
            imeOverlay = std::make_unique<UI::ImeOverlay>(settings.shortcut, settings.appearance.language);
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
    m_textService = TextServiceFactory::Create(m_fEnabledTsf);
    m_textService->RegisterCallback([](std::wstring_view compositionString) static -> void {
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
    m_imeWindow = std::make_unique<ImeWindow>();

    m_inputMethodManager = new InputMethodManager();
    if (FAILED(m_inputMethodManager->Initialize(tsfSupport.GetThreadMgr(), tsfSupport.GetTfClientId())))
    {
        throw SimpleIMEException("Can't initialize LangProfileUtil");
    }
}

void ImeWnd::UnInitialize() noexcept
{
    m_imeOverlay.reset(); // must release before ImGui shutdown
    if (m_textService != nullptr)
    {
        m_textService->UnInitialize();
    }
    if (m_inputMethodManager != nullptr)
    {
        m_inputMethodManager->UnInitialize();
    }
    Tsf::TsfSupport::GetSingleton().UnInitializeTsf();
}

void ImeWnd::CreateHost(HWND hWndParent, Settings &settings)
{
    logger::info("Start ImeWnd Thread...");
    m_hWnd = CreateWindowExW(0, g_MainClassName, L"Hide", WS_CHILD, 0, 0, 0, 0, hWndParent, nullptr, Global::g_hModule, this);
    if (m_hWnd == nullptr)
    {
        throw SimpleIMEException("Create ImeWnd failed");
    }
    OnCreated(settings);
}

void ImeWnd::Run() const
{
    MSG  msg  = {};
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
    return m_textService->OnFocus(focus);
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
    return m_inputMethodManager->ActivateProfile(guidProfile);
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
    m_textService->UpdateIfDirty();

    AutoToggleGameCursorIfNeeded(m_fJustWantCaptureMouse);

    ManageImeOverlayOnDemand(m_imeOverlay, m_translatorLoadDebounceTimer, settings);

    {
        auto      &m3Styles  = ImGuiEx::M3::Context::GetM3Styles();
        const auto fontScope = m3Styles.UseTextRole<ImGuiEx::M3::Spec::TextRole::LabelLarge>();
        DrawImeStates(); // draw ing ImGui default Debug window;
        ErrorNotifier::GetInstance().Show();
        m_imeWindow->Draw(m_textService->GetCompositionInfo(), m_textService->GetCandidateUi(), settings);

        const auto &activeLang   = m_inputMethodManager->GetActiveLangProfile();
        const auto &langProfiles = m_inputMethodManager->GetLangProfiles();

        if (m_imeOverlay != nullptr)
        {
            m_imeOverlay->Draw(activeLang, langProfiles, settings);
            if (settings.runtimeData.overlayShowing)
            {
                m_translatorLoadDebounceTimer.Poke();
            }
        }
    }
    ImeController::GetInstance()->SaveSettings(settings);
}

auto ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    // logger::debug("Message: {:#X} {} {}", uMsg, wParam, lParam);
    ImeWnd *pThis = GetThis(hWnd);
    if (pThis != nullptr)
    {
        pThis->ForwardKeyboardMessage(uMsg, wParam, lParam);
        if (pThis->m_textService->ProcessImeMessage(hWnd, uMsg, wParam, lParam))
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
            if (ImeController::GetInstance()->IsModEnabled() && (state.NotHas(State::IME_DISABLED) && state.Has(State::INPUT_PROCESSOR_ACTIVATED)))
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

/**
 * @deprecated Use Run() with a plain Win32 PeekMessage loop instead.
 *
 * ## Why ITfMessagePump / ITfKeystrokeMgr are unnecessary for SimpleIME
 *
 * ITfMessagePump is a TSF wrapper around GetMessage/PeekMessage that adds pre- and
 * post-processing hooks so the TSF manager can intercept preserved keys (e.g. Shift for
 * Microsoft Pinyin's CN/EN toggle) before they reach the application.
 * ITfKeystrokeMgr routes those intercepted keys to the active text service.
 *
 * In a normal text editor this makes sense. In SimpleIME it does not, for two reasons:
 *
 * 1. **Skyrim does not expose keyboard messages to mods.**
 *    The base game handles all raw input internally; WM_KEYDOWN/WM_KEYUP never reach
 *    mod-side code through the normal game loop. The only keyboard signal that matters
 *    here is WM_CHAR, which is already handled by the WndProc WM_CHAR branch and forwarded
 *    to the game via the existing "message compensation" mechanism. ITfKeystrokeMgr adds
 *    nothing to that path.
 *
 * 2. **Microsoft Pinyin (mspy) has a confirmed OS-level bug when ITfMessagePump and
 *    ITfKeystrokeMgr are used together.**
 *    When the user presses Shift during an active composition (mspy's preserved key for
 *    CN/EN mode toggle), mspy triggers a nested message loop inside the ITfMessagePump
 *    pre-processing stage. That nested loop devours all subsequent messages until
 *    AssociateFocus is called to clear the document focus, effectively freezing the
 *    application. Switching to a plain PeekMessage loop bypasses the TSF pre-processing
 *    layer entirely and avoids the hang.
 *
 *    This is a known Microsoft bug, acknowledged in KB4564002:
 *    https://support.microsoft.com/en-us/topic/kb4564002-you-might-have-issues-on-windows-10-version-20h2-and-windows-10-version-2004-when-using-some-microsoft-imes-63696506-47d2-9997-0b72-41a68e328692
 *
 *    The same freeze has been independently reported and confirmed by multiple projects:
 *    - KiCad issue #9882 (open since 2021, marked out-of-scope — Microsoft's bug):
 *      https://gitlab.com/kicad/code/kicad/-/issues/9882
 *    - Audacity issue #1618:
 *      https://github.com/audacity/audacity/issues/1618
 *    - SimpleIME author's StackOverflow question (note: the accepted answer still uses
 *      ITfMessagePump and therefore still carries the bug):
 *      https://stackoverflow.com/questions/79527691/tsf-prevent-keyboard-event-when-use-microsoft-pinyin
 *
 * The current message loop in Run() uses plain PeekMessageW and is the correct approach.
 * This function is kept only for reference and must not be called.
 */
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
    m_textService->OnStart(m_hWnd);
    m_uiScale            = ImGui_ImplWin32_GetDpiScaleForHwnd(m_hWnd);
    m_fWantUpdateUiScale = true;
    float uiScale        = settings.appearance.zoom;
    uiScale              = static_cast<float>(align_to(static_cast<int>(uiScale * 100.0F), Settings::ZOOM_STEP_PERCENT)) / 100.F;
    if (uiScale > 0.0F)
    {
        m_uiScale = std::clamp(uiScale, Settings::ZOOM_MIN, Settings::ZOOM_MAX);
    }

    ImeController::GetInstance()->Init(this, m_hWndParent, settings);
}

auto ImeWnd::OnDestroy() -> LRESULT
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

void ImeWnd::DrawImeStates()
{
#ifdef _DEBUG
    const auto &state     = Core::State::GetInstance();
    auto        stateIcon = [](bool enable) constexpr -> void {
        if (enable)
        {
            ImGuiEx::M3::Icon(ICON_EYE, ImGuiEx::M3::Spec::SizeTips::SMALL);
        }
        else
        {
            ImGuiEx::M3::Icon(ICON_EYE_OFF, ImGuiEx::M3::Spec::SizeTips::SMALL);
        }
    };

    const auto styleGuard = ImGuiEx::StyleGuard().Style<ImGuiStyleVar_WindowPadding>(ImVec2(20.0F, 20.0F));
    if (!ImGui::Begin("SimpleIme Debug", nullptr, ImGuiEx::WindowFlags().AlwaysAutoResize()))
    {
        ImGui::End();
        return;
    }
    ImGui::Value("IME focused", m_fFocused);
    bool textServiceFocused = state.Has(Core::State::TEXT_SERVICE_FOCUS);
    if (ImGui::Checkbox("Toggle TSF Focus", &textServiceFocused))
    {
        if (!m_textService->OnFocus(textServiceFocused))
        {
            ErrorNotifier::GetInstance().Debug("Failed to toggle text service focus.");
        }
    }

    // clang-format off
    const auto & conversionMode = state.GetConversionMode();
    stateIcon(textServiceFocused);                                 ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("TEXT_SERVICE_FOCUS");
    stateIcon(state.Has(Core::State::IN_COMPOSING));        ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IN_COMPOSING");
    stateIcon(state.Has(Core::State::IN_CAND_CHOOSING));    ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IN_CAND_CHOOSING");
    stateIcon(conversionMode.IsAlphanumeric());             ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMODE Native"); ImGui::SameLine();
    stateIcon(conversionMode.IsNative());                   ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: NATIVE"); ImGui::SameLine();
    stateIcon(conversionMode.IsKatakana());                 ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: KATAKANA"); ImGui::SameLine();
    stateIcon(conversionMode.IsFullShape());                ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: FULLSHAPE"); ImGui::SameLine();
    stateIcon(conversionMode.IsRoman());                    ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: ROMAN"); ImGui::SameLine();
    stateIcon(conversionMode.IsCharCode());                 ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: CHARCODE"); ImGui::SameLine();
    stateIcon(conversionMode.IsSoftKeyboard());             ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: SOFTKEYBOARD"); ImGui::SameLine();
    stateIcon(conversionMode.IsNoConversion());             ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: NOCONVERSION"); ImGui::SameLine();
    stateIcon(conversionMode.IsEudc());                     ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: EUDC"); ImGui::SameLine();
    stateIcon(conversionMode.IsSymbol());                   ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: SYMBOL"); ImGui::SameLine();
    stateIcon(conversionMode.IsFixed());                    ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("CMode: FIXED");
    stateIcon(state.Has(Core::State::INPUT_PROCESSOR_ACTIVATED)); ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("INPUT_PROCESSOR_ACTIVATED");
    stateIcon(state.Has(Core::State::IME_DISABLED));        ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("IME_DISABLED");
    stateIcon(state.Has(Core::State::GAME_LOADING));        ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("GAME_LOADING");
    stateIcon(state.Has(Core::State::KEYBOARD_OPEN));       ImGui::SameLine(); ImGuiEx::M3::AlignedLabel("KEYBOARD_OPEN");
    // clang-format on
    ImGui::End();
#endif
}
} // namespace Ime
