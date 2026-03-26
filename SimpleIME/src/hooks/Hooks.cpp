#include "hooks/Hooks.hpp"

#include "configs/CustomMessage.h"
#include "detours/detours.h"
#include "log.h"

#include <errhandlingapi.h>
#include <processthreadsapi.h>
#include <windows.h>

namespace Hooks
{
auto DetourUtil::DetourAttach(PVOID *original, PVOID hook) -> bool
{
    LONG error = NO_ERROR;
    if (error = DetourTransactionBegin(); error == NO_ERROR)
    {
        if (error = DetourUpdateThread(GetCurrentThread()); error == NO_ERROR)
        {
            if (error = ::DetourAttach(original, hook); error == NO_ERROR)
            {
                error = DetourTransactionCommit();
            }
        }
    }

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
        logger::error("Failed detour: {}", errorMsg.data());
    }

    return error == NO_ERROR;
}

auto DetourUtil::DetourDetach(void **original, void *hook) -> bool
{
    LONG error = NO_ERROR;
    if (error = DetourTransactionBegin(); error == NO_ERROR)
    {
        if (error = DetourUpdateThread(GetCurrentThread()); error == NO_ERROR)
        {
            if (error = ::DetourDetach(original, hook); error == NO_ERROR)
            {
                error = DetourTransactionCommit();
            }
        }
    }
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
        logger::error("Failed detour: {}", errorMsg.data());
    }

    return error == NO_ERROR;
}

} // namespace Hooks
