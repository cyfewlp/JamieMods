#include <SDKDDKVer.h>
#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // 某些 CString 构造函数将是显式的

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include <atlbase.h>
#include <atlcomcli.h>
#include <atlctl.h>

#include "spdlog/sinks/stdout_color_sinks.h"
#include <dinput.h>
#include <iostream>
#include <spdlog/spdlog.h>

// #pragma comment(lib, "dxguid.lib")
// #pragma comment(lib, "dinput8.lib")

static HINSTANCE                            hInst;
static HWND                                 hWndMain;
static HWND                                 hWndEdit;
static ATL::CWindow                         g_cWindow;
static void                                 InitLog();
auto                                        InitApplication(HINSTANCE hInstance) -> BOOL;
BOOL                                        InitInstance(HINSTANCE hInstance, INT nCmdShow);
LRESULT CALLBACK                            MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void                                        DisableIME();

int APIENTRY WinMain(HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine,
                     INT nCmdShow)
{
    AllocConsole();
    // Your code here
    std::cout << "Hello from WinMain!" << '\n';

    InitLog();
    MSG  msg      = {0};
    BOOL bRet     = FALSE;
    BOOL bSuccess = TRUE;

    if (InitApplication(hInstance) == 0)
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (InitInstance(hInstance, nCmdShow) == 0)
    {
        bSuccess = FALSE;
        goto exit_func;
    }
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            bSuccess = FALSE;
            goto exit_func;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

exit_func:
    if (bSuccess != 0)
    {
        return static_cast<int>(msg.wParam);
    }
    return 0;
}

void InitLog()
{
    auto console    = spdlog::stdout_color_mt("console");
    auto err_logger = spdlog::stderr_color_mt("stderr");
    console->set_level(spdlog::level::debug);

    spdlog::set_default_logger(std::move(console));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
}

/**********************************************************************/
/*                                                                    */
/*    InitApplication(HANDLE)                                         */
/*                                                                    */
/**********************************************************************/
auto InitApplication(HINSTANCE hInstance) -> BOOL
{
    WNDCLASS wc       = {0};
    BOOL     bSuccess = TRUE;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = MainWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = LoadIcon(hInstance, TEXT("MyIcon"));
    wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground  = static_cast<HBRUSH>(GetStockObject(LTGRAY_BRUSH));
    wc.lpszMenuName   = TEXT("ImeAppsMenu");
    wc.lpszClassName  = TEXT("ImeAppsWClass");

    if (!RegisterClass(&wc))
    {
        bSuccess = FALSE;
        goto exit_func;
    }
exit_func:
    return bSuccess;
}

/**********************************************************************/
/*                                                                    */
/*    InitInstance(HANDLE, int)                                       */
/*                                                                    */
/**********************************************************************/
auto InitInstance(HINSTANCE hInstance, INT nCmdShow) -> BOOL
{
#ifdef USEWAPI
    TCHAR szTitle[] = TEXT("ImeApps W API version");
#else
    TCHAR szTitle[] = TEXT("ImeApps");
#endif

    hInst = hInstance;

    if (!(hWndMain = CreateWindowEx(0L, TEXT("ImeAppsWClass"), (LPTSTR)szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL)))
    {
        return FALSE;
    }

    /* display each windows */
    ShowWindow(hWndMain, nCmdShow);
    UpdateWindow(hWndMain);

    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/*    MainWndProc(HWND, UINT, WPARAM, LPARAM)                         */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE: {
            // CreateWindowExW(0, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 10, 10, 200, 20,
            //                 hWnd, NULL, hInst, NULL);
            HIMC hIMC = ImmCreateContext();
            ImmAssociateContextEx(hWnd, hIMC, IACE_IGNORENOCONTEXT);
            spdlog::info("Message: WM_CREATE");
            AddClipboardFormatListener(hWnd);
            g_cWindow.Attach(hWnd);
            ATL::CComBSTR bstr;
            g_cWindow.ShowCaret();
            if (g_cWindow.GetWindowTextW(&bstr))
            {
                OutputDebugStringW(bstr);
                OutputDebugStringW(L"\n");
            }
            return S_OK;
        }
        case WM_DESTROY: {
            ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
            PostQuitMessage(0);
            break;
        }
        case WM_HOTKEY: {
            spdlog::info("Message: WM_HOTKEY {:#x}, {:#x}", wParam, lParam);
            break;
        }
        case WM_CHAR: {
            if (wParam == VK_IME_ON)
            {
                spdlog::info("Message: WM_CHAR VK_IME_ON");
            }
            break;
        }
        case WM_DESTROYCLIPBOARD:
        case WM_DRAWCLIPBOARD:
        case WM_RENDERFORMAT:
        case WM_RENDERALLFORMATS:
        case WM_PAINTCLIPBOARD:
        case WM_HSCROLLCLIPBOARD:
        case WM_VSCROLLCLIPBOARD:
        case WM_CHANGECBCHAIN:
        case WM_SIZECLIPBOARD: {
            spdlog::info("Message: WM_SIZECLIPBOARD");
            break;
        }
        case WM_CLIPBOARDUPDATE: {
            spdlog::info("Message: WM_CLIPBOARDUPDATE");
            break;
        }
        default:
            break;
    }
    return ::DefWindowProcW(hWnd, message, wParam, lParam);
}
