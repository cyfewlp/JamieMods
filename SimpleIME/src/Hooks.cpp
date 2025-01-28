#include "Hooks.hpp"
#include "ImeWnd.hpp"
#include "WCharUtils.h"
#include "detours/detours.h"
#include <windows.h>

namespace SimpleIME
{

    static Hooks::MYHOOKDATA myHookData[NUMHOOKS];

    void                     Hooks::InstallWindowsHooks()
    {
        myHookData[GET_MSG_PROC].nType = WH_GETMESSAGE;
        myHookData[GET_MSG_PROC].hkprc = MyGetMsgProc;

        for (auto hookData : myHookData)
        {
            logv(debug, "Hooking Skyrim {}...", hookData.nType);
            HHOOK hhk = SetWindowsHookExW(WH_GETMESSAGE, MyGetMsgProc, nullptr, GetCurrentThreadId());
            if (!hhk)
            {
                logv(err, "Hook {} failed! error code: {}", hookData.nType, GetLastError());
                return;
            }
            hookData.hhook = hhk;
        }
    }

    LRESULT CALLBACK Hooks::MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam)
    {
        auto hookData = myHookData[GET_MSG_PROC];
        if (code < 0)
        {
            return CallNextHookEx(hookData.hhook, code, wParam, lParam);
        }
        MSG *msg      = (MSG *)(lParam);
        UINT original = msg->message;
        switch (msg->message)
        {
            case 0x14fe:
            case 0x14fd:
            case 0x118:
            case WM_MOUSEMOVE:
                break;
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
            case WM_IME_COMPOSITION:
            case WM_IME_CHAR:
            case WM_IME_NOTIFY:
            case WM_CHAR:
                if (msg->wParam >= 0xd800 && msg->wParam <= 0xdfff) // emoji code
                {
                    switch (msg->message)
                    {
                        case WM_IME_COMPOSITION:
                            msg->message = WM_CUSTOM_IME_COMPPOSITION;
                            break;
                        case WM_IME_CHAR:
                            msg->message = WM_CUSTOM_IME_CHAR;
                            break;
                        case WM_CHAR:
                            msg->message = WM_CUSTOM_CHAR;
                            break;
                    }
                    logv(debug, "Replace {:#x} to {:#x}: {:#x}", original, msg->message, msg->wParam);
                }
                break;
            default:
                logv(debug, "MSG Hook: Wnd-{} {:#x}, {:#x}, {:#x}", (LPVOID)msg->hwnd, msg->message, msg->wParam,
                     msg->lParam);
                break;
        }
        return CallNextHookEx(hookData.hhook, code, wParam, lParam);
    }

    typedef ATOM(WINAPI *FuncRegisterClass)(CONST WNDCLASSA *);
    static inline FuncRegisterClass RealRegisterClassExA = NULL;

    static ATOM __stdcall MyRegisterClassExA(const WNDCLASSA *wndClass)
    {
        hinst = wndClass->hInstance;
        Hooks::InstallWindowsHooks();
        mainThreadId = GetCurrentThreadId();
        logv(debug, "thread id {}, hinst {}", mainThreadId, (LPVOID)hinst);
        return RealRegisterClassExA(wndClass);
    }

    void Hooks::InstallCreateWindowHook()
    {
        LPCSTR pszModule   = "User32.dll";
        LPCSTR pszFunction = "RegisterClassA";
        PVOID  pVoid       = DetourFindFunction(pszModule, pszFunction);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        RealRegisterClassExA = reinterpret_cast<FuncRegisterClass>(pVoid);
        DetourAttach(&(PVOID &)RealRegisterClassExA, MyRegisterClassExA);
        LONG error = DetourTransactionCommit();
        if (error == NO_ERROR)
        {
            logv(debug, "{}: Detoured {}.", pszModule, pszFunction);
        }
        else
        {
            logv(err, "{}: Error Detouring {}.", pszModule, pszFunction);
        }
    }
}