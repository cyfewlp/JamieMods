#include "Hooks.hpp"
#include "Configs.h"
#include "ImeWnd.hpp"
#include "detours/detours.h"
#include <errhandlingapi.h>
#include <minwindef.h>
#include <processthreadsapi.h>
#include <windows.h>
#include "FakeDirectInputDevice.h"

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
                                msg->message = CM_IME_COMPOSITION;
                                break;
                            case WM_IME_CHAR:
                                msg->message = CM_IME_CHAR;
                                break;
                            case WM_CHAR:
                                msg->message = CM_CHAR;
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

        void Hooks::InstallDirectInPutHook()
        {
            LPCSTR pszModule   = "dinput8.dll";
            LPCSTR pszFunction = "DirectInput8Create";
            PVOID  pVoid       = DetourFindFunction(pszModule, pszFunction);
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            RealDirectInput8Create = reinterpret_cast<FuncDirectInput8Create>(pVoid);
            DetourAttach(&(PVOID &)RealDirectInput8Create, reinterpret_cast<void *>(MyDirectInput8Create));
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

        auto Hooks::MyRegisterClassExA(const WNDCLASSA *wndClass) -> ATOM
        {
            Hooks::InstallWindowsHooks();
            return RealRegisterClassExA(wndClass);
        }

        auto Hooks::MyDirectInput8Create(HINSTANCE hinst, DWORD dwVersion, const IID &riidltf, LPVOID *ppvOut,
                                         LPUNKNOWN punkOuter) -> HRESULT
        {
            IDirectInput8A	* dinput;
            HRESULT           hresult = RealDirectInput8Create(hinst, dwVersion, riidltf, reinterpret_cast<void **>(&dinput), punkOuter);
            if(hresult != DI_OK) return hresult;

            *((IDirectInput8A**)ppvOut) = new FakeDirectInput(dinput);
            log_debug("detour to FakeDirectInput...");
            return DI_OK;
        }
    }
}