#include "ImeWnd.hpp"

SimpleIME::ImeWnd::ImeWnd()
{
    m_hInst      = nullptr;
    m_hParentWnd = nullptr;
    m_hWnd       = nullptr;
    m_pImeUI     = nullptr;
    m_show       = false;
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
    wc.style         = CS_CLASSDC;
    wc.cbClsExtra    = 0;
    wc.lpfnWndProc   = ImeWnd::WndProc;
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
    m_hWnd = ::CreateWindowExW(0, g_tMainClassName, L"Hide", WS_CHILD, 0, 0,   //
                               rect.right - rect.left, rect.bottom - rect.top, // width, height
                               a_parent,                                       // a_parent,
                               nullptr, wc.hInstance, (LPVOID)this);
    //
    ImGui::GetPlatformIO().Platform_SetImeDataFn = MyPlatform_SetImeDataFn;
    if (m_hWnd == nullptr) return false;
    return true;
}

LRESULT SimpleIME::ImeWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto *pThis = (ImeWnd *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if ((nullptr == pThis) && (uMsg != WM_NCCREATE))
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    if (nullptr == pThis) return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
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
        case WM_KEYUP:
        case WM_KEYDOWN:
            if (wParam == VK_F2)
            {
                pThis->m_show = !pThis->m_show;
            }
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
        default:
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
LRESULT SimpleIME::ImeWnd::OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    m_pImeUI->CompositionString(hWnd, lParam);
    return 0;
}

LRESULT SimpleIME::ImeWnd::OnCreate()
{
    m_pImeUI = new ImeUI();
    return S_OK;
}

LRESULT SimpleIME::ImeWnd::OnDestroy()
{
    PostQuitMessage(0);
    return 0;
}

void SimpleIME::ImeWnd::MyPlatform_SetImeDataFn(ImGuiContext *ctx, ImGuiViewport *viewport, ImGuiPlatformImeData *data)
{
    auto inputPos = data->InputPos;
    SimpleIME::ImeUI::UpdateCaretPos(inputPos.x, inputPos.y);
}

void SimpleIME::ImeWnd::Focus()
{
    ::SetFocus(m_hWnd);
}

void SimpleIME::ImeWnd::RenderImGui()
{
    if (m_show)
    {
        m_pImeUI->RenderImGui();
    }
}

bool SimpleIME::ImeWnd::IsImeEnabled()
{
    return m_pImeUI->IsEnabled();
}
