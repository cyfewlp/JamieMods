#include "core/EventHandler.h"

#include "ImeWnd.hpp"
#include "Utils.h"
#include "configs/AppConfig.h"
#include "core/State.h"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"
#include "ime/ImeManagerComposer.h"
#include "imgui.h"
#include "utils/FocusGFxCharacterInfo.h"

#include <RE/B/BSInputDeviceManager.h>
#include <RE/B/BSTEvent.h>
#include <RE/B/BSWin32MouseDevice.h>
#include <RE/B/ButtonEvent.h>
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

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime::Core
    {
        constexpr auto EventHandler::IsPasteShortcutPressed(auto &code)
        {
            return code == ENUM_DIK_V && (::GetKeyState(ENUM_VK_CONTROL) & 0x8000) != 0;
        }

        void EventHandler::InstallEventSink(ImeWnd *imeWnd)
        {
            static InputEventSink         g_InputEventSink(imeWnd);
            static MenuOpenCloseEventSink g_pMenuOpenCloseEventSink;
            RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent *>(&g_InputEventSink);
            RE::UI::GetSingleton()->AddEventSink(&g_pMenuOpenCloseEventSink);
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
            if (Utils::IsImeNotActivateOrGameLoading())
            {
                Hooks::UiHooks::EnableMessageFilter(false);
            }
            else
            {
                if (Utils::IsImeInputting() || (!Utils::IsCapsLockOn() && Utils::IsKeyWillTriggerIme(code)))
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
            log_debug("Menu {} open {}", event->menuName.c_str(), event->opening);
            static bool firstOpenMainMenu = true;
            if (event->menuName != RE::CursorMenu::MENU_NAME && event->menuName != RE::HUDMenu::MENU_NAME)
            {
                auto movieView = RE::UI::GetSingleton()->GetMovieView(event->menuName);
                FocusGFxCharacterInfo::GetInstance().Update(movieView.get());
            }
            if (firstOpenMainMenu && event->menuName == RE::MainMenu::MENU_NAME && event->opening)
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
                uint8_t textEntryCount = Hooks::SKSE_ScaleformAllowTextInput::TextEntryCount();
                while (textEntryCount > 0)
                {
                    Hooks::SKSE_ScaleformAllowTextInput::AllowTextInput(false);
                    textEntryCount--;
                }
                // check var consistency
                textEntryCount = Hooks::SKSE_ScaleformAllowTextInput::TextEntryCount();
                if (textEntryCount != 0)
                {
                    log_warn("Text entry count is incorrect and can't fix it! count: {}", textEntryCount);
                }
            }
        }
    }
}