//
// Created by jamie on 2025/3/1.
//

#ifndef GAMESTRINGSENDER_H
#define GAMESTRINGSENDER_H

#include "RE/GFxCharEvent.h"
#include "common/log.h"

#include "core/State.h"
#include "hooks/UiHooks.h"
#include "ime/ImeManagerComposer.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        class Utils
        {
            static constexpr auto ASCII_GRAVE_ACCENT = 0x60; // `
            static constexpr auto ASCII_MIDDLE_DOT   = 0xB7; // ·
            using State                              = Core::State;

        public:
            template <typename Ptr>
            static auto ToLongPtr(Ptr *ptr) -> LONG_PTR
            {
                return reinterpret_cast<LONG_PTR>(ptr);
            }

            static constexpr auto IsVkCodeDown(uint32_t vkCode) -> bool
            {
                return (::GetKeyState(vkCode) & 0x8000) != 0;
            }

            static constexpr auto IsCapsLockOn() -> bool
            {
                SHORT capsState = GetKeyState(VK_CAPITAL);
                return (capsState & 0x0001) != 0;
            }

            static constexpr auto IsImeNotActivateOrGameLoading() -> bool
            {
                auto &state = State::GetInstance();
                return !ImeManagerComposer::GetInstance()->IsModEnabled() || //
                       state.HasAny(State::IME_DISABLED, State::IN_ALPHANUMERIC, State::GAME_LOADING) ||
                       state.NotHas(State::LANG_PROFILE_ACTIVATED);
            }

            static constexpr auto IsImeInputting() -> bool
            {
                return State::GetInstance().HasAny(State::IN_CAND_CHOOSING, State::IN_COMPOSING);
            }

            static constexpr auto IsKeyWillTriggerIme(const uint32_t keycode) -> bool
            {
                if (IsVkCodeDown(VK_CONTROL) || IsVkCodeDown(VK_SHIFT) || IsVkCodeDown(VK_MENU) ||
                    IsVkCodeDown(VK_LWIN) || IsVkCodeDown(VK_RWIN))
                {
                    return false;
                }

                bool result = false;
                using Key   = RE::BSKeyboardDevice::Keys::Key;
                result      = result || (keycode >= Key::kQ && keycode <= Key::kP);
                result      = result || (keycode >= Key::kA && keycode <= Key::kL);
                result      = result || (keycode >= Key::kZ && keycode <= Key::kM);
                return result;
            }

            template <typename String>
            static void SendStringToGame(String &&sourceString)
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
                for (uint32_t const code : sourceString)
                {
                    if (!Send(pFactory, code))
                    {
                        return;
                    }
                }
            }

        private:
            static auto Send(auto *pFactory, const uint32_t code) -> bool
            {
                if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
                {
                    return true;
                }
                auto *pCharEvent            = new RE::GFxCharEvent(code, 0);
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
