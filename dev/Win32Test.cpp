#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <windows.h>

static HINSTANCE hInst;
static HWND      hWndMain;

void             InitLog();
BOOL             InitApplication(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE hInstance, INT nCmdShow);
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY     WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    // Your code here
    std::cout << "Hello from WinMain!" << std::endl;

    InitLog();
    MSG  msg      = {0};
    BOOL bRet     = FALSE;
    BOOL bSuccess = TRUE;

    if (!InitApplication(hInstance))
    {
        bSuccess = FALSE;
        goto exit_func;
    }

    if (!InitInstance(hInstance, nCmdShow))
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
    if (bSuccess)
    {
        return (int)(msg.wParam);
    }
    else
    {
        return 0;
    }
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
BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS wc       = {0};
    BOOL     bSuccess = TRUE;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = MainWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = LoadIcon(hInstance, TEXT("MyIcon"));
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
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
BOOL InitInstance(HINSTANCE hInstance, INT nCmdShow)
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
        // CreateWindowExW(0, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 10, 10, 200, 20, hWnd,
        //                 NULL, hInst, NULL);
        HIMC hIMC = ImmCreateContext();
        ImmAssociateContextEx(hWnd, hIMC, IACE_IGNORENOCONTEXT);
        return S_OK;
    }
    case WM_DESTROY: {
        ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
        PostQuitMessage(0);
        break;
    }
    case WM_IME_COMPOSITION:
        spdlog::info("WM_IME_COMPOSITION: {:#x}", wParam);
        break;
    case WM_CHAR:
        spdlog::info("WM_CHAR: {:#x}", wParam);
        break;
    case WM_INPUTLANGCHANGE: {
        HIMC hIMC;
        if ((hIMC = ImmGetContext(hWnd)))
        {
            COMPOSITIONFORM composition_form = {};
            composition_form.ptCurrentPos.x  = 200L;
            composition_form.ptCurrentPos.y  = 50L;
            composition_form.dwStyle         = CFS_FORCE_POSITION;
            ImmSetCompositionWindow(hIMC, &composition_form);
            
            CANDIDATEFORM candidate_form = {};
            candidate_form.dwStyle = CFS_CANDIDATEPOS;
            candidate_form.ptCurrentPos.x = 300L;
            candidate_form.ptCurrentPos.y = 300L;
            ImmSetCandidateWindow(hIMC, &candidate_form);
            ImmReleaseContext(hWnd, hIMC);
        }
        // switch (wParam)
        // {
        // case IMN_SETOPENSTATUS: {
        //     HIMC hIMC;
        //     if ((hIMC = ImmGetContext(hWnd)))
        //     {
        //         COMPOSITIONFORM composition_form = {};
        //         composition_form.ptCurrentPos.x  = 200L;
        //         composition_form.ptCurrentPos.y  = 50L;
        //         composition_form.dwStyle         = CFS_FORCE_POSITION;
        //         ImmSetCompositionWindow(hIMC, &composition_form);
        //         POINT point = {100, 100};
        //         ImmSetStatusWindowPos(hIMC, &point);
        //         CANDIDATEFORM candidate_form = {CFS_CANDIDATEPOS, 300, 300};
        //         ImmSetCandidateWindow(hIMC, &candidate_form);
        //         ImmReleaseContext(hWnd, hIMC);
        //     }
        //     return S_OK;
        // }
        // default:
        //     break;
        // }
        break;
    }
    default:
        break;
    }
    return ::DefWindowProcW(hWnd, message, wParam, lParam);
}