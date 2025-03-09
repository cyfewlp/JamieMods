#include "hooks/Hooks.hpp"
#include "FakeDirectInputDevice.h"
#include "ImeWnd.hpp"
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
        auto DetourUtil::DetourAttach(PVOID *original, PVOID hook) -> bool
        {
            LONG error = NO_ERROR;
            __try
            {
                if (error = DetourTransactionBegin(); error != NO_ERROR)
                {
                    __leave;
                }

                if (error = DetourUpdateThread(GetCurrentThread()); error != NO_ERROR)
                {
                    __leave;
                }

                if (error = ::DetourAttach(original, hook); error != NO_ERROR)
                {
                    __leave;
                }

                error = DetourTransactionCommit();
                return error == NO_ERROR;
            }
            __finally
            {
                std::string_view errorMsg;
                if (error != NO_ERROR)
                {
                    DetourTransactionAbort();
                    switch (error)
                    {
                        case ERROR_INVALID_OPERATION:
                            errorMsg = "No pending or Already exists transaction.";
                            break;
                        case ERROR_INVALID_HANDLE:
                            errorMsg = "The ppPointer parameter is NULL or points to a NULL pointer.";
                            break;
                        case ERROR_INVALID_BLOCK:
                            errorMsg = "The function referenced is too small to be detoured.";
                            break;
                        case ERROR_NOT_ENOUGH_MEMORY:
                            errorMsg = "Not enough memory exists to complete the operation.";
                            break;
                        case ERROR_INVALID_DATA:
                            errorMsg = "Target function was changed by third party between steps of the transaction.";
                            break;
                        default:
                            errorMsg = "unexpected error when detour.";
                            break;
                    }
                    log_error("Failed detour: {}", errorMsg);
                }
            }
        }

        auto DetourUtil::DetourDetach(void **original, void *hook) -> bool
        {
            LONG error = NO_ERROR;
            __try
            {
                if (error = DetourTransactionBegin(); error != NO_ERROR)
                {
                    __leave;
                }

                if (error = DetourUpdateThread(GetCurrentThread()); error != NO_ERROR)
                {
                    __leave;
                }

                if (error = ::DetourDetach(original, hook); error != NO_ERROR)
                {
                    __leave;
                }

                error = DetourTransactionCommit();
                return error == NO_ERROR;
            }
            __finally
            {
                std::string_view errorMsg;
                if (error != NO_ERROR)
                {
                    DetourTransactionAbort();
                    switch (error)
                    {
                        case ERROR_INVALID_OPERATION:
                            errorMsg = "No pending or Already exists transaction.";
                            break;
                        case ERROR_INVALID_HANDLE:
                            errorMsg = "The ppPointer parameter is NULL or points to a NULL pointer.";
                            break;
                        case ERROR_INVALID_BLOCK:
                            errorMsg = "The function referenced is too small to be detoured.";
                            break;
                        case ERROR_NOT_ENOUGH_MEMORY:
                            errorMsg = "Not enough memory exists to complete the operation.";
                            break;
                        case ERROR_INVALID_DATA:
                            errorMsg = "Target function was changed by third party between steps of the transaction.";
                            break;
                        default:
                            errorMsg = "unexpected error when detour.";
                            break;
                    }
                    log_error("Failed detour: {}", errorMsg);
                }
            }
        }

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
            auto        pszModule   = "User32.dll";
            auto        pszFunction = "RegisterClassA";
            const PVOID pVoid       = DetourFindFunction(pszModule, pszFunction);
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