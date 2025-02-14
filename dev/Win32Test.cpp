#include "spdlog/sinks/stdout_color_sinks.h"
#include <dinput.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <windows.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

static HINSTANCE     hInst;
static HWND          hWndMain;
static HWND          hWndEdit;
LPDIRECTINPUT8       g_pDirectInput    = nullptr;
LPDIRECTINPUTDEVICE8 g_pKeyboardDevice = nullptr;
static void          InitLog();
auto                 InitApplication(HINSTANCE hInstance) -> BOOL;
BOOL                 InitInstance(HINSTANCE hInstance, INT nCmdShow);
LRESULT CALLBACK     MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void                 DisableIME();

int APIENTRY         WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
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
    if (g_pDirectInput == nullptr)
    {
        delete g_pDirectInput;
    }
    if (g_pKeyboardDevice == nullptr)
    {
        delete g_pKeyboardDevice;
    }
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
            // hWnd,
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
        case WM_IME_NOTIFY: {
            spdlog::info("WM_IME_NOTIFY: {:#x}", wParam);
            switch (wParam)
            {
                case IMN_OPENCANDIDATE:
                case IMN_CHANGECANDIDATE:
                case IMN_SETCANDIDATEPOS:
                case IMN_CLOSECANDIDATE:
                case 0xF:
                    HIMC hIMC;
                    hIMC         = ImmGetContext(hWnd);
                    DWORD bufLen = ImmGetCandidateListA(hIMC, 0, nullptr, 0);
                    spdlog::info("candidate len: {}", bufLen);
                    ImmReleaseContext(hWnd, hIMC);
                    break;
            }
            return S_OK;
        }
        case WM_IME_SETCONTEXT:
            return DefWindowProcW(hWnd, WM_IME_SETCONTEXT, wParam, NULL);
        default:
            break;
    }
    return ::DefWindowProcW(hWnd, message, wParam, lParam);
}

void DisableIME()
{
    if (FAILED(DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8,
                                  reinterpret_cast<void **>(&g_pDirectInput), nullptr)))
    {
        throw std::runtime_error("DirectInput8Create failed");
    }
    if (FAILED(g_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pKeyboardDevice, nullptr)))
    {
        throw std::runtime_error("CreateDevice failed");
    }
    const DWORD dwFlags = DISCL_BACKGROUND | DISCL_EXCLUSIVE;
    HRESULT     hr      = g_pKeyboardDevice->SetCooperativeLevel(hWndMain, dwFlags);
    if (FAILED(hr))
    {
        switch (hr)
        {
            case DIERR_INVALIDPARAM:
                spdlog::error("error DIERR_INVALIDPARAM");
                break;
            case DIERR_NOTINITIALIZED:
                spdlog::error("error DIERR_NOTINITIALIZED");
                break;
            case E_HANDLE:
                spdlog::error("error DIERR_NOTINITIALIZED");
                break;
            default:
                spdlog::error("unknown error, {:#x}", (ULONG)hr);
                break;
        }
        throw std::runtime_error("SetCooperativeLevel failed");
    }
    if (FAILED(g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
    {
        throw std::runtime_error("SetDataFormat failed");
    }
    if (FAILED(g_pKeyboardDevice->Acquire()))
    {
        throw std::runtime_error("Acquire device failed");
    }
}
