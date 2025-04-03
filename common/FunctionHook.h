//
// Created by jamie on 2025/4/3.
//

#ifndef FUNCTIONHOOK_H
#define FUNCTIONHOOK_H

#include "common/config.h"
#include "common/log.h"
#include "detours/detours.h"

#include <processthreadsapi.h>
#include <windows.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        template <typename func_t>
            requires requires {
                std::is_function_v<func_t>;
                std::is_pointer_v<func_t>;
            }
        class FunctionHook
        {
        };

        class DetourUtil
        {
        public:
            static auto DetourAttach(void **original, void *hook) -> bool
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
                                errorMsg =
                                    "Target function was changed by third party between steps of the transaction.";
                                break;
                            default:
                                errorMsg = "unexpected error when detour.";
                                break;
                        }
                        log_error("Failed detour: {}", errorMsg.data());
                    }
                }
            }

            static auto DetourDetach(void **original, void *hook) -> bool
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
                                errorMsg =
                                    "Target function was changed by third party between steps of the transaction.";
                                break;
                            default:
                                errorMsg = "unexpected error when detour.";
                                break;
                        }
                        log_error("Failed detour: {}", errorMsg.data());
                    }
                }
            }
        };

        template <typename Return, typename... Args>
        class FunctionHook<Return(Args...)>
        {
            void *m_originalFuncPtr;
            void *m_hook;
            bool  detoured = false;

        protected:
            std::uintptr_t m_address;
            using func_type = Return(Args...);

        public:
            explicit FunctionHook(REL::RelocationID id, Return (*funcPtr)(Args...))
            {
                m_address         = id.address();
                m_originalFuncPtr = reinterpret_cast<void *>(m_address);
                m_hook            = reinterpret_cast<void *>(funcPtr);

                if (DetourUtil::DetourAttach(&m_originalFuncPtr, m_hook))
                {
                    detoured = true;
                }
            }

            explicit FunctionHook(void *&realFuncPtr, Return (*funcPtr)(Args...))
            {
                m_address         = reinterpret_cast<std::uintptr_t>(realFuncPtr);
                m_originalFuncPtr = realFuncPtr;
                m_hook            = reinterpret_cast<void *>(funcPtr);

                if (DetourUtil::DetourAttach(&reinterpret_cast<PVOID &>(m_originalFuncPtr), m_hook))
                {
                    detoured = true;
                }
            }

            ~FunctionHook()
            {
                if (detoured)
                {
                    log_debug("Detour {:#x} detach {:#x}", reinterpret_cast<std::uintptr_t>(m_hook),
                              reinterpret_cast<std::uintptr_t>(m_originalFuncPtr));
                    DetourUtil::DetourDetach(&reinterpret_cast<PVOID &>(m_originalFuncPtr), m_hook);
                }
            }

            [[nodiscard]] bool Detoured() const
            {
                return detoured;
            }

            constexpr Return operator()(Args... args) const
            {
                if constexpr (std::is_void_v<Return>)
                {
                    reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
                else
                {
                    return reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
            }

            constexpr Return Original(Args... args) const
            {
                if constexpr (std::is_void_v<Return>)
                {
                    reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
                else
                {
                    return reinterpret_cast<Return (*)(Args...)>(m_originalFuncPtr)(args...);
                }
            }
        };
    }
}

#endif // FUNCTIONHOOK_H
