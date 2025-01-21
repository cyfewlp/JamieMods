#include "Hooks.hpp"
#include "HookUtils.h"
#include "ImeWnd.hpp"
#include <windows.h>

void SimpleIME::HookGetMsgProc()
{
    logv(debug, "Hooking Skyrim GetMsgProc...");
    HHOOK hhk = SetWindowsHookExW(WH_GETMESSAGE, MyGetMsgProc, nullptr, GetCurrentThreadId());
    if (!hhk)
    {
        logv(err, "Hook MsgProc failed! error code: {}", GetLastError());
    }
}

LRESULT CALLBACK SimpleIME::MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code < 0)
    {
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }
    MSG *msg = (MSG *)(lParam);
    switch (msg->message)
    {
        case WM_IME_COMPOSITION:
        case WM_IME_CHAR:
        case WM_CHAR:
            logv(trace, "MSG Hook: {:#x}, {:#x}, {:#x}", msg->message, msg->wParam, msg->lParam);
            if (msg->wParam >= 0xd800 && msg->wParam <= 0xdfff) // emoji code
            {
                UINT original = msg->message;
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
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}