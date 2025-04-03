#include "hooks/Hooks.hpp"
#include "common/log.h"
#include "configs/CustomMessage.h"
#include "detours/detours.h"
#include <errhandlingapi.h>
#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        static MYHOOKDATA myHookData[NUMHOOKS];

        void InstallWindowsHooks()
        {
            myHookData[GET_MSG_PROC].nType = WH_GETMESSAGE;
            myHookData[GET_MSG_PROC].hkprc = MyGetMsgProc;

            for (auto &hookData : myHookData)
            {
                log_debug("Hooking Skyrim {}...", hookData.nType);
                const HHOOK hhk = SetWindowsHookExW(WH_GETMESSAGE, MyGetMsgProc, nullptr, GetCurrentThreadId());
                if (hhk == nullptr)
                {
                    log_error("Hook {} failed! error code: {}", hookData.nType, GetLastError());
                    return;
                }
                hookData.hhook = hhk;
            }
        }

        LRESULT CALLBACK MyGetMsgProc(int code, WPARAM wParam, LPARAM lParam)
        {
            const auto hookData = myHookData[GET_MSG_PROC];
            if (code < 0)
            {
                return CallNextHookEx(hookData.hhook, code, wParam, lParam);
            }
            const auto msg      = reinterpret_cast<MSG *>(lParam);
            UINT const original = msg->message;
            switch (msg->message)
            {
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
                                msg->message = CM_IME_COMPOSITION;
                                break;
                            case WM_IME_CHAR:
                                msg->message = CM_IME_CHAR;
                                break;
                            case WM_CHAR:
                                msg->message = CM_CHAR;
                                break;
                            default:
                                break;
                        }
                        log_debug("Replace {:#x} to {:#x}: {:#x}", original, msg->message, msg->wParam);
                    }
                    break;
                default:
                    break;
            }
            return CallNextHookEx(hookData.hhook, code, wParam, lParam);
        }

        void InstallRegisterClassHook()
        {
            auto *pszModule   = "User32.dll";
            auto *pszFunction = "RegisterClassA";
            PVOID pVoid       = DetourFindFunction(pszModule, pszFunction);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            RealRegisterClassExA = reinterpret_cast<FuncRegisterClass>(pVoid);
            DetourAttach(&reinterpret_cast<PVOID &>(RealRegisterClassExA),
                         reinterpret_cast<void *>(MyRegisterClassExA));
            if (LONG const error = DetourTransactionCommit(); error == NO_ERROR)
            {
                log_debug("{}: Detoured {}.", pszModule, pszFunction);
            }
            else
            {
                log_error("{}: Error Detouring {}.", pszModule, pszFunction);
            }
        }

        auto MyRegisterClassExA(const WNDCLASSA *wndClass) -> ATOM
        {
            InstallWindowsHooks();
            return RealRegisterClassExA(wndClass);
        }

    }
}