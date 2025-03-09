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
        void UiHooks::InstallHooks()
        {
            log_info("Install ui hooks...");
            UiAddMessage = std::make_unique<UiAddMessageHookData>(AddMessageHook);
        }

        void UiHooks::AddMessageHook(RE::UIMessageQueue *self, RE::BSFixedString &menuName,
                                     RE::UI_MESSAGE_TYPE messageType, RE::IUIMessageData *pMessageData)
        {
            if (messageType == RE::UI_MESSAGE_TYPE::kScaleformEvent)
            {
                auto *scaleformData = reinterpret_cast<RE::BSUIScaleformData *>(pMessageData);
                // is char event?
                auto *event = scaleformData->scaleformEvent;
                if (event->type == RE::GFxEvent::EventType::kCharEvent)
                {
                    if (spdlog::should_log(spdlog::level::trace))
                    {
                        Ime::GFxCharEvent *gfxCharEvent = reinterpret_cast<Ime::GFxCharEvent *>(event);
                        log_trace("menu {} Char message: {}", menuName.c_str(), gfxCharEvent->wcharCode);
                    }

                    if (!g_fEnableMessageFilter || RE::InterfaceStrings::GetSingleton() == nullptr)
                    {
                        UiAddMessage->Original(self, menuName, messageType, pMessageData);
                        return;
                    }
                    static RE::BSFixedString &topMenu = RE::InterfaceStrings::GetSingleton()->topMenu;
                    // is IME open and not IME sent message?
                    if (menuName == IME_MESSAGE_FAKE_MENU)
                    {
                        UiAddMessage->Original(self, topMenu, messageType, pMessageData);
                    }
                    else
                    {
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
