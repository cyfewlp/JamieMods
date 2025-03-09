#include "core/EventHandler.h"

#include "Utils.h"
#include "configs/AppConfig.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime::Core
    {

        constexpr auto EventHandler::IsImeNotActivateOrGameLoading() -> bool
        {

            return State::GetInstance()->HasAny(State::IME_DISABLED, State::IN_ALPHANUMERIC, State::GAME_LOADING) ||
                   State::GetInstance()->NotHas(State::LANG_PROFILE_ACTIVATED);
        }

        constexpr auto EventHandler::IsImeWantCaptureInput() -> bool
        {
            return State::GetInstance()->HasAny(State::IN_CAND_CHOOSING, State::IN_COMPOSING);
        }

        auto EventHandler::IsWillTriggerIme(const std::uint32_t code) -> bool
        {
            bool result = false;
            using Key   = RE::BSKeyboardDevice::Keys::Key;
            result |= code >= Key::kQ && code <= Key::kP;
            result |= code >= Key::kA && code <= Key::kL;
            result |= code >= Key::kZ && code <= Key::kM;
            return result;
        }

        constexpr auto EventHandler::IsPasteShortcutPressed(auto &code)
        {
            return code == ENUM_DIK_V && (::GetKeyState(ENUM_VK_CONTROL) & 0x8000) != 0;
        }

        auto EventHandler::HandleKeyboardEvent(const RE::ButtonEvent *buttonEvent) -> void
        {
            const auto code = buttonEvent->GetIDCode();
            if (IsImeNotActivateOrGameLoading())
            {
                Hooks::UiHooks::EnableMessageFilter(false);
            }
            else
            {
                if (IsImeWantCaptureInput() || IsWillTriggerIme(code))
                {
                    Hooks::UiHooks::EnableMessageFilter(true);
                }
                else
                {
                    Hooks::UiHooks::EnableMessageFilter(false);
                }
            }
        }

        auto EventHandler::PostHandleKeyboardEvent() -> void
        {
        }
    }
}