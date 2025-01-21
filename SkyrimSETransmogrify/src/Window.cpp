#include "Window.hpp"
#include "ImeUI.hpp"
#include "imgui.h"

Transmogrify::HideWnd::HideWnd()
{
    m_hInst      = NULL;
    m_hParentWnd = NULL;
    m_hWnd       = NULL;
    m_pImeUI     = NULL;
}

Transmogrify::HideWnd::~HideWnd()
{
    if (m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
    }
}

BOOL Transmogrify::HideWnd::Initialize(HWND a_parent)
{
    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(wc));

    m_hInst          = GetModuleHandle(nullptr);
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_CLASSDC;
    wc.cbClsExtra    = 0;
    wc.lpfnWndProc   = HideWnd::_WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = m_hInst;
    wc.lpszClassName = g_tMainClassName;
    if (!::RegisterClassExW(&wc))
    {
        return false;
    }

    RECT rect = {0, 0, 0, 0};
    GetClientRect(a_parent, &rect);
    m_hWnd = ::CreateWindowExW(0, g_tMainClassName, L"Hide",
                               //   WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_CAPTION,
                               WS_CHILD, 0, 0,                                 //
                               rect.right - rect.left, rect.bottom - rect.top, // width, height
                               a_parent,                                       // a_parent,
                               nullptr, wc.hInstance, (LPVOID)this);
    //
    ImGui::GetPlatformIO().Platform_SetImeDataFn = MyPlatform_SetImeDataFn;

    if (m_hWnd == NULL) return false;
    return true;
}

// we are override WM_COMPOSITION, so the default COMPOSITION window is unavailble,
// So we need update our ImeUI::m_caretPos to render our COMPOSITION window.
void Transmogrify::HideWnd::MyPlatform_SetImeDataFn([[maybe_unused]] ImGuiContext *ctx, ImGuiViewport *viewport,
                                                    ImGuiPlatformImeData *data)
{
    HideWnd *pThis = (HideWnd *)viewport->PlatformUserData;
    if (NULL == pThis) return;
    auto inputPos = data->InputPos;
    pThis->m_pImeUI->UpdateCaretPos(inputPos.x, inputPos.y);
}

void Transmogrify::HideWnd::Focus()
{
    ::SetFocus(m_hWnd);
}

LRESULT
Transmogrify::HideWnd::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HideWnd *pThis = (HideWnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if ((NULL == pThis) && (uMsg != WM_NCCREATE))
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    switch (uMsg)
    {
    case WM_NCCREATE: {
        LPCREATESTRUCT lpCs = (LPCREATESTRUCT)lParam;
        pThis               = (HideWnd *)(lpCs->lpCreateParams);
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
        ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
        return pThis->OnDestroy();
    }
    case WM_KEYUP:
    case WM_KEYDOWN:
        ::SendMessageA(pThis->m_hParentWnd, uMsg, wParam, lParam);
        return S_OK;
    case WM_INPUTLANGCHANGE: {
        pThis->m_pImeUI->UpdateLanguage((HKL)lParam);
        return S_OK;
    }
    case WM_IME_STARTCOMPOSITION:
        return pThis->OnStartComposition();
    case WM_IME_ENDCOMPOSITION:
        return pThis->OnEndComposition();
    case WM_CUSTOM_IME_COMPPOSITION:
    case WM_IME_COMPOSITION:
        return pThis->OnComposition(hWnd, wParam, lParam);
    case WM_CUSTOM_CHAR:
    case WM_CHAR: {
        // never received WM_CHAR msg becaus wo handled WM_IME_COMPOSITION
        break;
    }
    }
    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT Transmogrify::HideWnd::OnCreate()
{
    m_pImeUI = new ImeUI();
    if (!m_pImeUI)
    {
        return -1;
    }
    return S_OK;
}

LRESULT Transmogrify::HideWnd::OnDestroy()
{
    PostQuitMessage(0);
    return S_OK;
}

LRESULT Transmogrify::HideWnd::OnStartComposition()
{
    m_pImeUI->StartComposition();
    return S_OK;
}

LRESULT Transmogrify::HideWnd::OnEndComposition()
{
    m_pImeUI->EndComposition();
    return S_OK;
}

LRESULT Transmogrify::HideWnd::OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    m_pImeUI->CompositionString(hWnd, lParam);
    return S_OK;
}

void Transmogrify::HideWnd::RenderImGui()
{
    ImGuiWindowFlags flags;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("SkyrimSE IME", 0, flags);
    if (ImGui::Button("Require Focus"))
    {
        Focus();
    }
    m_pImeUI->RenderCompStr();
    ImGui::End();
}

bool Transmogrify::HideWnd::IsSkyrimIMEEnabled()
{
    return m_pImeUI->IsSkyrimIMEEnabled();
}
