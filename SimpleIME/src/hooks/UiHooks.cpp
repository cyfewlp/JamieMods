//
// Created by jamie on 2025/3/3.
//

#include "hooks/UiHooks.h"
#include "hooks/ScaleformHook.h"
#include <windows.h>

#include "detours/detours.h"

#include <Utils.h>

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

        void UiHooks::InstallHooks()
        {
            log_info("Install ui hooks...");
            UiAddMessage = std::make_unique<UiAddMessageHookData>(AddMessageHook);
        }

        void UiHooks::AddMessageHook(RE::UIMessageQueue *self, RE::BSFixedString &menuName,
                                     RE::UI_MESSAGE_TYPE messageType, RE::IUIMessageData *pMessageData)
        {
            if (!g_fEnableMessageFilter || RE::InterfaceStrings::GetSingleton() == nullptr)
            {
                UiAddMessage->Original(self, menuName, messageType, pMessageData);
                return;
            }
            static RE::BSFixedString &topMenu = RE::InterfaceStrings::GetSingleton()->topMenu;

            if (messageType == RE::UI_MESSAGE_TYPE::kScaleformEvent)
            {
                auto *scaleformData = reinterpret_cast<RE::BSUIScaleformData *>(pMessageData);
                // is char event?
                auto *event = scaleformData->scaleformEvent;
                if (event->type == RE::GFxEvent::EventType::kCharEvent)
                {
                    // is IME open and not IME sent message?
                    if (menuName == IME_MESSAGE_FAKE_MENU)
                    {
                        UiAddMessage->Original(self, topMenu, messageType, pMessageData);
                    }
                    else
                    {
                        if (spdlog::should_log(spdlog::level::trace))
                        {
                            Ime::GFxCharEvent *gfxCharEvent = reinterpret_cast<Ime::GFxCharEvent *>(event);
                            log_trace("Discard char message: {}", gfxCharEvent->wcharCode);
                        }
                        if (auto *memoryManager = RE::MemoryManager::GetSingleton(); memoryManager != nullptr)
                        {
                            memoryManager->Deallocate(pMessageData, false);
                        }
                    }
                    return;
                }
            }

            UiAddMessage->Original(self, menuName, messageType, pMessageData);
        }
    }
}
