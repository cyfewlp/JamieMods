//
// Created by jamie on 2025/3/1.
//

#ifndef GAMESTRINGSENDER_H
#define GAMESTRINGSENDER_H

#include "common/WCharUtils.h"
#include "common/log.h"

#include "hooks/UiHooks.h"

#include <string>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        // language id for english keyboard
        class GFxCharEvent : public RE::GFxEvent
        {
        public:
            GFxCharEvent() = default;

            explicit GFxCharEvent(UINT32 a_wcharCode, UINT8 a_keyboardIndex = 0)
                : GFxEvent(EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
            {
            }

            // @members
            std::uint32_t wcharCode{};     // 04
            std::uint32_t keyboardIndex{}; // 08
        };

        static_assert(sizeof(GFxCharEvent) == 0x0C);

        class Utils
        {
            static constexpr auto ASCII_GRAVE_ACCENT = 0x60; // `
            static constexpr auto ASCII_MIDDLE_DOT   = 0xB7; // ·

        public:
            template <typename Ptr>
            static auto ToLongPtr(Ptr *ptr) -> LONG_PTR
            {
                return reinterpret_cast<LONG_PTR>(ptr);
            }

            static auto PasteText(HWND hWnd) -> bool
            {
                bool result = false;
                if (OpenClipboard(hWnd))
                {
                    BOOL unicode = IsClipboardFormatAvailable(CF_UNICODETEXT);
                    BOOL utf8    = IsClipboardFormatAvailable(CF_TEXT);
                    if (unicode || utf8)
                    {
                        if (HANDLE handle = GetClipboardData(unicode ? CF_UNICODETEXT : CF_TEXT); handle != nullptr)
                        {
                            if (auto const textData = static_cast<LPTSTR>(GlobalLock(handle)); textData != nullptr)
                            {
                                if (unicode)
                                {
                                    SendStringToGame(std::wstring(textData));
                                }
                                else
                                {
                                    SendStringToGame(std::string(reinterpret_cast<LPSTR>(textData)));
                                }
                                GlobalUnlock(handle);
                                result = true;
                            }
                        }
                    }
                    CloseClipboard();
                }
                return result;
            }

            template <typename String>
            static void SendStringToGame(const String &sourceString)
            {
                log_debug("Ready result string to Skyrim...");
                auto *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
                auto *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
                if (pInterfaceStrings == nullptr || pFactoryManager == nullptr)
                {
                    log_warn("Can't send string to Skyrim may game already close?");
                    return;
                }

                const auto *pFactory =
                    pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);
                if (pFactory == nullptr)
                {
                    log_warn("Can't send string to Skyrim may game already close?");
                    return;
                }

                // Start send message
                RE::BSFixedString const menuName = pInterfaceStrings->topMenu;
                for (uint32_t const code : sourceString)
                {
                    if (!Send(menuName, pFactory, code))
                    {
                        return;
                    }
                }
            }

        private:
            static auto Send(const RE::BSFixedString &menuName, auto *pFactory, const uint32_t code) -> bool
            {
                if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
                {
                    return true;
                }
                auto *pCharEvent            = new GFxCharEvent(code, 0);
                auto *pScaleFormMessageData = pFactory->Create();
                if (pScaleFormMessageData == nullptr)
                {
                    log_error("Unable create BSTDerivedCreator.");
                    return false;
                }
                pScaleFormMessageData->scaleformEvent = pCharEvent;
                log_debug("send code {:#x} to Skyrim", code);
                RE::UIMessageQueue::GetSingleton()->AddMessage(
                    Hooks::IME_MESSAGE_FAKE_MENU, RE::UI_MESSAGE_TYPE::kScaleformEvent, pScaleFormMessageData);
                return true;
            }
        };
    }
}

#endif // GAMESTRINGSENDER_H
