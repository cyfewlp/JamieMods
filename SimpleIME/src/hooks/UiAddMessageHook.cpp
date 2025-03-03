//
// Created by jamie on 2025/3/3.
//

#include "hooks/UiAddMessageHook.h"
#include <windows.h>

#include "detours/detours.h"

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
    }
}
