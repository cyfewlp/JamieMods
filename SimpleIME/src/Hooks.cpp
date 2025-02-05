#include "Hooks.hpp"
#include "Configs.h"
#include "ImeWnd.hpp"
#include "detours/detours.h"
#include <errhandlingapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {

        static Hooks::MYHOOKDATA myHookData[NUMHOOKS];

        void                     Hooks::InstallWindowsHooks()
        {
            myHookData[GET_MSG_PROC].nType = WH_GETMESSAGE;
            myHookData[GET_MSG_PROC].hkprc = MyGetMsgProc;

            for (auto &hookData : myHookData)
            {
                log_debug("Hooking Skyrim {}...", hookData.nType);
                HHOOK hhk = SetWindowsHookExW(WH_GETMESSAGE, MyGetMsgProc, nullptr, GetCurrentThreadId());
                if (hhk == nullptr)
                {
                    log_error("Hook {} failed! error code: {}", hookData.nType, GetLastError());
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
            MSG       *msg      = reinterpret_cast<MSG *>(lParam);
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
                                msg->message = WM_CUSTOM_IME_COMPPOSITION;
                                break;
                            case WM_IME_CHAR:
                                msg->message = WM_CUSTOM_IME_CHAR;
                                break;
                            case WM_CHAR:
                                msg->message = WM_CUSTOM_CHAR;
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

        using FuncRegisterClass                              = ATOM (*)(const WNDCLASSA *);
        static inline FuncRegisterClass RealRegisterClassExA = nullptr;

        static ATOM __stdcall MyRegisterClassExA(const WNDCLASSA *wndClass)
        {
            Hooks::InstallWindowsHooks();
            return RealRegisterClassExA(wndClass);
        }

        void Hooks::InstallRegisterClassHook()
        {
            LPCSTR pszModule   = "User32.dll";
            LPCSTR pszFunction = "RegisterClassA";
            PVOID  pVoid       = DetourFindFunction(pszModule, pszFunction);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            RealRegisterClassExA = reinterpret_cast<FuncRegisterClass>(pVoid);
            DetourAttach(&(PVOID &)RealRegisterClassExA, reinterpret_cast<void *>(MyRegisterClassExA));
            LONG const error = DetourTransactionCommit();
            if (error == NO_ERROR)
            {
                log_debug("{}: Detoured {}.", pszModule, pszFunction);
            }
            else
            {
                log_error("{}: Error Detouring {}.", pszModule, pszFunction);
            }
        }
    }
}