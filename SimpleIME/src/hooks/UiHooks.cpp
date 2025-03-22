//
// Created by jamie on 2025/3/3.
//

#include "hooks/UiHooks.h"
#include "Utils.h"
#include "hooks/WinHooks.h"

#include <RE/Offsets_VTABLE.h>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        void UiHooks::InstallHooks()
        {
            log_info("Install ui hooks...");
            UiAddMessage          = std::make_unique<UiAddMessageHookData>(AddMessageHook);
            ConsoleProcessMessage = std::make_unique<MenuProcessMessageHook>(MyConsoleProcessMessage);
        }

        void UiHooks::UninstallHooks()
        {
            UiAddMessage          = nullptr;
            ConsoleProcessMessage = nullptr;
        }

        static bool ctrlDown = false;

        static void Free(RE::IUIMessageData *data)
        {
            if (auto *memoryManager = RE::MemoryManager::GetSingleton(); memoryManager != nullptr)
            {
                memoryManager->Deallocate(data, false);
            }
        }

        void UiHooks::AddMessageHook(RE::UIMessageQueue *self, RE::BSFixedString &menuName,
                                     RE::UI_MESSAGE_TYPE messageType, RE::IUIMessageData *pMessageData)
        {
            if (auto *strings = RE::InterfaceStrings::GetSingleton(); strings != nullptr)
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
                            RE::GFxCharEvent *gfxCharEvent = reinterpret_cast<RE::GFxCharEvent *>(event);
                            log_trace("menu {} Char message: {}", menuName.c_str(), gfxCharEvent->wcharCode);
                        }
                        if (!g_fEnableMessageFilter)
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
                            Free(pMessageData);
                        }
                        return;
                    }
                }
            }
            UiAddMessage->Original(self, menuName, messageType, pMessageData);
        }

        auto UiHooks::MyConsoleProcessMessage(RE::IMenu *self, RE::UIMessage &uiMessage) -> RE::UI_MESSAGE_RESULTS
        {
            if (!IsEnableUnicodePaste() || self == nullptr)
            {
                return ConsoleProcessMessage->Original(self, uiMessage);
            }

            if (uiMessage.type == RE::UI_MESSAGE_TYPE::kScaleformEvent)
            {
                RE::BSUIScaleformData *data  = reinterpret_cast<RE::BSUIScaleformData *>(uiMessage.data);
                auto                  *event = data->scaleformEvent;
                if (event->type == RE::GFxEvent::EventType::kKeyDown)
                {
                    auto *keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
                    if (keyEvent->keyCode == RE::GFxKey::kControl)
                    {
                        ctrlDown = true;
                    }
                }
                else if (event->type == RE::GFxEvent::EventType::kKeyUp)
                {
                    auto *keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
                    if (keyEvent->keyCode == RE::GFxKey::kControl)
                    {
                        ctrlDown = false;
                    }
                }
                else if (event->type == RE::GFxEvent::EventType::kCharEvent)
                {
                    RE::GFxCharEvent *charEvent = reinterpret_cast<RE::GFxCharEvent *>(event);
                    auto              vtable    = reinterpret_cast<std::uintptr_t *>(self)[0];
                    if (vtable != RE::VTABLE_CursorMenu[0].address()) // ignore cursor menu
                    {
                        if (charEvent->wcharCode == 'v' && ctrlDown)
                        {
                            auto result = RE::UI_MESSAGE_RESULTS::kHandled;
                            WinHooks::DisablePaste(true);
                            {
                                // Console implemented it copy/paste feature, we should not discard message
                                if (vtable == RE::VTABLE_Console[0].address())
                                {
                                    result = ConsoleProcessMessage->Original(self, uiMessage);
                                }
                            }
                            WinHooks::DisablePaste(false);
                            Ime::Utils::PasteText(nullptr); // Do our paste
                            return result;
                        }
                    }
                }
            }
            return ConsoleProcessMessage->Original(self, uiMessage);
        }
    }
}
