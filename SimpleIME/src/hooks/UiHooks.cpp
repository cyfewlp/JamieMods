//
// Created by jamie on 2025/3/3.
//

#include "hooks/UiHooks.h"
#include "RE/GFxCharEvent.h"
#include "common/config.h"
#include "common/log.h"
#include "hooks/WinHooks.h"
#include "ime/ImeManagerComposer.h"

#include <RE/B/BSFixedString.h>
#include <RE/B/BSUIScaleformData.h>
#include <RE/G/GFxEvent.h>
#include <RE/G/GFxKey.h>
#include <RE/G/GFxMovieView.h>
#include <RE/I/IMenu.h>
#include <RE/I/IUIMessageData.h>
#include <RE/I/InterfaceStrings.h>
#include <RE/M/MemoryManager.h>
#include <RE/Offsets_VTABLE.h>
#include <RE/U/UIMessage.h>
#include <RE/U/UIMessageQueue.h>
#include <cstdint>
#include <memory>
#include <string>

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

        void UiHooks::ScaleformPasteText(RE::GFxMovieView *const uiMovie, RE::GFxCharEvent *const charEvent)
        {
            if (::OpenClipboard(nullptr) == FALSE)
            {
                return;
            }
            const bool unicode = IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE;
            const bool utf8    = IsClipboardFormatAvailable(CF_TEXT) != FALSE;
            if (unicode || utf8)
            {
                if (HANDLE handle = ::GetClipboardData(unicode ? CF_UNICODETEXT : CF_TEXT); handle != nullptr)
                {
                    if (auto *const textData = static_cast<LPTSTR>(GlobalLock(handle)); textData != nullptr)
                    {
                        const auto action = [uiMovie, charEvent](uint32_t code) {
                            charEvent->wcharCode = code;
                            uiMovie->HandleEvent(*charEvent);
                        };
                        if (unicode)
                        {
                            for (const uint32_t code : std::wstring(textData))
                            {
                                action(code);
                            }
                        }
                        else
                        {
                            for (const uint32_t code : std::string(reinterpret_cast<LPSTR>(textData)))
                            {
                                action(code);
                            }
                        }

                        GlobalUnlock(handle);
                    }
                }
            }
            CloseClipboard();
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
#ifdef ENABLE_MENU_PROCESSMESSAGE // except for console menu, other menu not need call ProcessMessage
                    WinHooks::DisablePaste(true);
                    MenuProcessMessage->Original(self, uiMessage);
                    WinHooks::DisablePaste(false);
#endif // ENABLE_MENU_PROCESSMESSAGE
                    ScaleformPasteText(self->uiMovie.get(), charEvent);
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
                    ConsoleProcessMessage->Original(self, uiMessage);
                    WinHooks::DisablePaste(false);
                    ScaleformPasteText(self->uiMovie.get(), charEvent);
                    return RE::UI_MESSAGE_RESULTS::kHandled;
                }
            }
            return ConsoleProcessMessage->Original(self, uiMessage);
        }
    }
}
