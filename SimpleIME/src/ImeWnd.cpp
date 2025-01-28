#include "ImeWnd.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandlerEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                                               ImGuiIO &io); // Doesn't use ImGui::GetCurrentContext()

SimpleIME::ImeWnd::ImeWnd()
{
    m_hInst      = nullptr;
    m_hParentWnd = nullptr;
    m_hWnd       = nullptr;
    m_pImeUI     = nullptr;
}

SimpleIME::ImeWnd::~ImeWnd()
{
    if (m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
    }
}

BOOL SimpleIME::ImeWnd::Initialize(HWND a_parent)
{
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));

    m_hInst          = GetModuleHandle(nullptr);
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_PARENTDC;
    wc.cbClsExtra    = 0;
    wc.lpfnWndProc   = ImeWnd::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_hInst;
    wc.lpszClassName = g_tMainClassName;
    if (!::RegisterClassExW(&wc)) return false;

    RECT rect = {0, 0, 0, 0};
    GetClientRect(a_parent, &rect);
    m_hWnd = ::CreateWindowExW(0, g_tMainClassName, L"Hide", //
                               WS_POPUP, 0, 0, 100, 100,     // width, height
                               a_parent,                     // a_parent,
                               nullptr, wc.hInstance, (LPVOID)this);
    //
    if (m_hWnd == nullptr) return false;
    m_pImeUI->QueryAllInstalledIME();
    return true;
}

LRESULT SimpleIME::ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto *pThis = (ImeWnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if ((nullptr == pThis) && (uMsg != WM_NCCREATE))
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    switch (uMsg)
    {
        case WM_NCCREATE: {
            auto lpCs = (LPCREATESTRUCT)lParam;
            pThis     = (ImeWnd *)(lpCs->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            // set the window handle
            pThis->m_hInst      = lpCs->hInstance;
            pThis->m_hWnd       = hWnd;
            pThis->m_hParentWnd = lpCs->hwndParent;
            break;
        }
        case WM_CREATE: {
            HIMC hIMC = ImmCreateContext();
            ImmAssociateContextEx(hWnd, hIMC, IACE_IGNORENOCONTEXT);
            return pThis->OnCreate();
        }
        case WM_DESTROY: {
            ImmAssociateContextEx(hWnd, nullptr, IACE_DEFAULT);
            return pThis->OnDestroy();
        }
        case WM_INPUTLANGCHANGE: {
            pThis->m_pImeUI->UpdateLanguage();
            return S_OK;
        }
        case WM_IME_NOTIFY: {
            logv(debug, "ImeWnd {:#x}", wParam);
            if (pThis->m_pImeUI->ImeNotify(hWnd, wParam, lParam)) return S_OK;
            break;
        }
        case WM_IME_STARTCOMPOSITION:
            return pThis->OnStartComposition();
        case WM_IME_ENDCOMPOSITION:
            logv(debug, "End composition, send test msg");
            ::SendMessageA(pThis->m_hParentWnd, WM_CHAR, 0x41, 0x1);
            ::SendMessageA(pThis->m_hParentWnd, WM_CHAR, 0x41, 0x6211);
            return pThis->OnEndComposition();
        case WM_CUSTOM_IME_COMPPOSITION:
        case WM_IME_COMPOSITION:
            return pThis->OnComposition(hWnd, lParam);
        case WM_CUSTOM_CHAR:
        case WM_CHAR: {
            // never received WM_CHAR msg becaus wo handled WM_IME_COMPOSITION
            break;
        }
        case WM_IME_SETCONTEXT:
            // lParam &= ~ISC_SHOWUICANDIDATEWINDOW;
            return DefWindowProcW(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
        default:
            ImGui_ImplWin32_WndProcHandlerEx(hWnd, uMsg, wParam, lParam, ImGui::GetIO());
            break;
    }
    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT SimpleIME::ImeWnd::OnStartComposition()
{
    m_pImeUI->StartComposition();
    return 0;
}

LRESULT SimpleIME::ImeWnd::OnEndComposition()
{
    m_pImeUI->EndComposition();
    return 0;
}

LRESULT SimpleIME::ImeWnd::OnComposition(HWND hWnd, LPARAM lParam)
{
    MAKE_CONTEXT(hWnd, m_pImeUI->CompositionString, lParam);
    return 0;
}

LRESULT SimpleIME::ImeWnd::OnCreate()
{
    m_pImeUI = new ImeUI(m_hWnd);
    m_pImeUI->UpdateLanguage();
    return S_OK;
}

LRESULT SimpleIME::ImeWnd::OnDestroy()
{
    PostQuitMessage(0);
    return 0;
}

void SimpleIME::ImeWnd::Focus() const
{
    logv(debug, "Focus to ime wnd");
    ::SetFocus(m_hWnd);
}

void SimpleIME::ImeWnd::RenderImGui()
{
    m_pImeUI->RenderImGui();
}

bool SimpleIME::ImeWnd::IsShow() const
{
    auto imeState = m_pImeUI->GetImeState();
    return imeState.any(IME_IN_CANDCHOOSEN, IME_IN_COMPOSITION);
}

static bool IsImeKeyCode(std::uint32_t code)
{
    bool result = false;
    // q-p
    result |= code >= 0x10 && code <= 0x19;
    // a-l
    result |= code >= 0x1E && code <= 0x26;
    // z-m
    result |= code >= 0x2C && code <= 0x32;
    return result;
}

RE::InputEvent **SimpleIME::ImeWnd::FilterInputEvent(RE::InputEvent **events)
{
    static RE::InputEvent *dummy[]  = {nullptr};

    auto                   imeState = m_pImeUI->GetImeState();
    if (imeState.none(IME_OPEN) || imeState.any(IME_IN_ALPHANUMERIC))
    {
        return events;
    }

    auto first = (imeState == IME_OPEN);
    if (imeState.any(IME_OPEN, IME_IN_CANDCHOOSEN, IME_IN_COMPOSITION))
    {
        if (events == nullptr || *events == nullptr)
        {
            return events;
        }
        auto head         = *events;
        auto sourceDevice = head->device;
        bool discard      = sourceDevice == RE::INPUT_DEVICE::kKeyboard;
        if (first)
        {
            auto idEvent = head->AsIDEvent();
            auto code    = idEvent ? idEvent->GetIDCode() : 0;
            discard &= IsImeKeyCode(code);
        }
        if (discard) return dummy;
    }
    return events;
}