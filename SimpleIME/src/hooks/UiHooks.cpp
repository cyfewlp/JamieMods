//
// Created by jamie on 2025/3/3.
//

#include "hooks/UiHooks.h"
#include "Utils.h"
#include "hooks/WinHooks.h"
#include "ime/ImeManagerComposer.h"

#include <RE/Offsets_VTABLE.h>
#include <memory>

namespace
{
    bool       ctrlDown            = false;
    const auto CursorVtableAddress = RE::VTABLE_CursorMenu[0].address();

    void Free(RE::IUIMessageData *data)
    {
        if (auto *memoryManager = RE::MemoryManager::GetSingleton(); memoryManager != nullptr)
        {
            memoryManager->Deallocate(data, false);
        }
    }

    auto ToCharEvent(RE::UIMessage &uiMessage) -> RE::GFxCharEvent *
    {
        if (uiMessage.type == RE::UI_MESSAGE_TYPE::kScaleformEvent)
        {
            RE::BSUIScaleformData *data  = reinterpret_cast<RE::BSUIScaleformData *>(uiMessage.data);
            auto                  *event = data->scaleformEvent;
            switch (auto type = event->type.get())
            {
                case RE::GFxEvent::EventType::kKeyDown:
                case RE::GFxEvent::EventType::kKeyUp: {
                    auto *keyEvent = reinterpret_cast<RE::GFxKeyEvent *>(event);
                    if (keyEvent->keyCode == RE::GFxKey::kControl)
                    {
                        ctrlDown = type == RE::GFxEvent::EventType::kKeyDown;
                    }
                    break;
                }
                case RE::GFxEvent::EventType::kCharEvent: {
                    return reinterpret_cast<RE::GFxCharEvent *>(event);
                }
                default:
                    break;
            }
        }
        return nullptr;
    }
}

namespace LIBC_NAMESPACE_DECL
{
    namespace Hooks
    {
        void UiHooks::InstallHooks()
        {
            log_info("Install ui hooks...");
            UiAddMessage          = std::make_unique<UiAddMessageHookData>(AddMessageHook);
            MenuProcessMessage    = std::make_unique<MenuProcessMessageHook>(MyMenuProcessMessage);
            ConsoleProcessMessage = std::make_unique<ConsoleProcessMessageHook>(MyConsoleProcessMessage);
        }

        void UiHooks::UninstallHooks()
        {
            UiAddMessage          = nullptr;
            MenuProcessMessage    = nullptr;
            ConsoleProcessMessage = nullptr;
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

        auto UiHooks::MyMenuProcessMessage(RE::IMenu *self, RE::UIMessage &uiMessage) -> RE::UI_MESSAGE_RESULTS
        {
            auto vtable = reinterpret_cast<std::uintptr_t *>(self)[0];

            if (!Ime::ImeManagerComposer::GetInstance()->IsUnicodePasteEnabled() || vtable == CursorVtableAddress)
            {
                return MenuProcessMessage->Original(self, uiMessage);
            }

            if (RE::GFxCharEvent *charEvent = ToCharEvent(uiMessage); charEvent != nullptr)
            {
                if (charEvent->wcharCode == 'v' && ctrlDown)
                {
                    WinHooks::DisablePaste(true);
                    WinHooks::DisablePaste(false);
                    Ime::Utils::PasteText(nullptr); // Do our paste
                    return RE::UI_MESSAGE_RESULTS::kHandled;
                }
            }
            return MenuProcessMessage->Original(self, uiMessage);
        }

        auto UiHooks::MyConsoleProcessMessage(RE::IMenu *self, RE::UIMessage &uiMessage) -> RE::UI_MESSAGE_RESULTS
        {
            auto vtable = reinterpret_cast<std::uintptr_t *>(self)[0];
            if (!Ime::ImeManagerComposer::GetInstance()->IsUnicodePasteEnabled() || vtable == CursorVtableAddress)
            {
                return ConsoleProcessMessage->Original(self, uiMessage);
            }
            if (RE::GFxCharEvent *charEvent = ToCharEvent(uiMessage); charEvent != nullptr)
            {
                if (charEvent->wcharCode == 'v' && ctrlDown)
                {
                    WinHooks::DisablePaste(true);
                    auto result = ConsoleProcessMessage->Original(self, uiMessage);
                    WinHooks::DisablePaste(false);
                    Ime::Utils::PasteText(nullptr); // Do our paste
                    return result;
                }
            }
            return ConsoleProcessMessage->Original(self, uiMessage);
        }
    }
}
