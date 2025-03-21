#include "core/EventHandler.h"

#include "ImeWnd.hpp"
#include "configs/AppConfig.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"
#include "ime/ImeManager.h"
#include "imgui.h"

#include <RE/B/BSInputDeviceManager.h>
#include <RE/B/BSKeyboardDevice.h>
#include <RE/B/BSTEvent.h>
#include <RE/B/BSWin32MouseDevice.h>
#include <RE/B/ButtonEvent.h>
#include <RE/C/Console.h>
#include <RE/C/CursorMenu.h>
#include <RE/I/InputDevices.h>
#include <RE/I/InputEvent.h>
#include <RE/M/MainMenu.h>
#include <RE/U/UI.h>
#include <WinUser.h>
#include <common/config.h>
#include <common/log.h>
#include <cstdint>
#include <dinput.h>
#include <memory>

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

        void EventHandler::InstallEventSink(ImeWnd *imeWnd)
        {
            static auto g_InputEventSink          = std::make_unique<InputEventSink>(imeWnd);
            static auto g_pMenuOpenCloseEventSink = std::make_unique<MenuOpenCloseEventSink>();
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent *>(g_InputEventSink.get());
            RE::UI::GetSingleton()->AddEventSink(g_pMenuOpenCloseEventSink.get());
        }

        auto EventHandler::UpdateMessageFilter(RE::InputEvent **a_events) -> void
        {
            if (a_events == nullptr)
            {
                return;
            }
            auto head = *a_events;
            if (head == nullptr)
            {
                return;
            }
            RE::ButtonEvent *buttonEvent = nullptr;
            if (head->GetEventType() == RE::INPUT_EVENT_TYPE::kButton)
            {
                buttonEvent = head->AsButtonEvent();
            }
            if (buttonEvent == nullptr)
            {
                return;
            }
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

        auto EventHandler::IsDiscardKeyboardEvent(const RE::ButtonEvent *buttonEvent) -> bool
        {
            bool discard = false;
            if (Hooks::UiHooks::IsEnableMessageFilter() && buttonEvent->GetIDCode() == DIK_E)
            {
                discard = true;
            }
            return discard;
        }

        auto EventHandler::PostHandleKeyboardEvent() -> void
        {
        }

        //////////////////////////////////////////////////////////////////////////
        // InputEventSink
        //////////////////////////////////////////////////////////////////////////

        RE::BSEventNotifyControl InputEventSink::ProcessEvent(Event *const *events,
                                                              RE::BSTEventSource<Event *> * /*eventSource*/)
        {

            for (auto *event = *events; event != nullptr; event = event->next)
            {
                if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton)
                {
                    if (auto *buttonEvent = event->AsButtonEvent(); buttonEvent != nullptr)
                    {
                        switch (buttonEvent->GetDevice())
                        {
                            case RE::INPUT_DEVICE::kKeyboard:
                                ProcessKeyboardEvent(buttonEvent);
                                break;
                            case RE::INPUT_DEVICE ::kMouse:
                                ProcessMouseButtonEvent(buttonEvent);
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }

        void InputEventSink::ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent)
        {
            auto value = buttonEvent->Value();
            switch (auto mouseKey = buttonEvent->GetIDCode())
            {
                case Keys::kWheelUp:
                    ImGui::GetIO().AddMouseWheelEvent(0, value);
                    break;
                case Keys::kWheelDown:
                    ImGui::GetIO().AddMouseWheelEvent(0, value * -1);
                    break;
                default: {
                    if (!ImGui::GetIO().WantCaptureMouse)
                    {
                        m_imeWnd->AbortIme();
                    }
                    else
                    {
                        ImGui::GetIO().AddMouseButtonEvent(static_cast<int>(mouseKey), buttonEvent->IsPressed());
                    }
                    break;
                }
            }
        }

        void InputEventSink::ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent)
        {
            auto keyCode = btnEvent->GetIDCode();
            if (keyCode == AppConfig::GetConfig().GetToolWindowShortcutKey() && btnEvent->IsDown())
            {
                m_imeWnd->ShowToolWindow();
            }
        }

        //////////////////////////////////////////////////////////////////////////
        // MenuOpenCloseEventSink
        //////////////////////////////////////////////////////////////////////////

        RE::BSEventNotifyControl MenuOpenCloseEventSink::ProcessEvent(const Event *event,
                                                                      RE::BSTEventSource<Event> * /*eventSource*/)
        {
            log_trace("Menu {} open {}", event->menuName.c_str(), event->opening);
            static bool firstOpenMainMenu = true;
            if (event->menuName == RE::Console::MENU_NAME)
            {
                Hooks::ScaleformAllowTextInput::OnTextEntryCountChanged();
            }
            else if (firstOpenMainMenu && event->menuName == RE::MainMenu::MENU_NAME && event->opening)
            {
                firstOpenMainMenu = false;
                ImeManagerComposer::GetInstance()->NotifyEnableMod(true);
            }
            else
            {
                FixInconsistentTextEntryCount(event);
            }
            return RE::BSEventNotifyControl::kContinue;
        }

        void MenuOpenCloseEventSink::FixInconsistentTextEntryCount(const Event *event)
        {
            if (event->menuName == RE::CursorMenu::MENU_NAME && !event->opening)
            {
                // fix: MapMenu will not call AllowTextInput(false) when closing
                uint8_t textEntryCount = Hooks::ScaleformAllowTextInput::TextEntryCount();
                while (textEntryCount > 0)
                {
                    Hooks::ScaleformAllowTextInput::AllowTextInput(false);
                    textEntryCount--;
                }
                // check var consistency
                textEntryCount = Hooks::ScaleformAllowTextInput::TextEntryCount();
                if (textEntryCount != 0)
                {
                    log_warn("Text entry count is incorrect and can't fix it! count: {}", textEntryCount);
                }
            }
        }
    }
}