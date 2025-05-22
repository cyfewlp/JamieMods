#include "core/EventHandler.h"

#include "ImeWnd.hpp"
#include "Utils.h"
#include "configs/AppConfig.h"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"
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

auto EventHandler::UpdateMessageFilter(const Settings &settings, RE::InputEvent **a_events) -> void
{
    auto *uiHooks = Hooks::UiHooks::GetInstance();
    if (!uiHooks || a_events == nullptr)
    {
        return;
    }
    const auto head = *a_events;
    if (head == nullptr)
    {
        return;
    }
    const RE::ButtonEvent *buttonEvent = nullptr;
    if (head->GetEventType() == RE::INPUT_EVENT_TYPE::kButton)
    {
        buttonEvent = head->AsButtonEvent();
    }
    if (buttonEvent == nullptr)
    {
        return;
    }
    const auto code = buttonEvent->GetIDCode();
    if (!settings.enableMod || Utils::IsImeNotActivateOrGameLoading())
    {
        uiHooks->EnableMessageFilter(false);
    }
    else
    {
        if (Utils::IsImeInputting() || (!Utils::IsCapsLockOn() && Utils::IsKeyWillTriggerIme(code)))
        {
            uiHooks->EnableMessageFilter(true);
        }
        else
        {
            uiHooks->EnableMessageFilter(false);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// InputEventSink
//////////////////////////////////////////////////////////////////////////

RE::BSEventNotifyControl InputEventSink::
    ProcessEvent(Event *const *events, RE::BSTEventSource<Event *> * /*eventSource*/)
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

void InputEventSink::ProcessMouseButtonEvent(const RE::ButtonEvent *buttonEvent) const
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

void InputEventSink::ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent) const
{
    if (const auto keyCode = btnEvent->GetIDCode();
        keyCode == AppConfig::GetConfig().GetToolWindowShortcutKey() && btnEvent->IsDown())
    {
        m_imeWnd->ShowToolWindow();
    }
}

//////////////////////////////////////////////////////////////////////////
// MenuOpenCloseEventSink
//////////////////////////////////////////////////////////////////////////

RE::BSEventNotifyControl MenuOpenCloseEventSink::
    ProcessEvent(const Event *event, RE::BSTEventSource<Event> * /*eventSource*/)
{
    log_debug("Menu {} open {}", event->menuName.c_str(), event->opening);
    static bool firstOpenMainMenu = true;
    if (event->menuName != RE::CursorMenu::MENU_NAME && event->menuName != RE::HUDMenu::MENU_NAME)
    {
        FocusGFxCharacterInfo::GetInstance().Update(event->menuName.c_str(), event->opening);
    }
    if (firstOpenMainMenu && event->menuName == RE::MainMenu::MENU_NAME && event->opening)
    {
        firstOpenMainMenu = false;
    }
    else if (event->menuName == RE::ConsoleNativeUIMenu::MENU_NAME) // Steam Overlay
    {
        static bool prevModEnabled = false;

        auto *manager = ImeManagerComposer::GetInstance();
        if (event->opening)
        {
            prevModEnabled = manager->IsModEnabled();
            manager->EnableMod(false);
        }
        else if (prevModEnabled)
        {
            manager->EnableMod(true);
        }
    }
    else
    {
        FixInconsistentTextEntryCount(event);
    }
    return RE::BSEventNotifyControl::kContinue;
}

void MenuOpenCloseEventSink::FixInconsistentTextEntryCount(const Event *event)
{
    // fix: if CursorMenu hide but text-entry count > 0, try to disable ime;
    // avoid modifying the textEntryCount field
    const uint8_t &textEntryCount = Hooks::ControlMap::GetSingleton()->GetTextEntryCount();
    if (event->menuName == RE::CursorMenu::MENU_NAME && textEntryCount > 0 && !event->opening)
    {
        const ImeManagerComposer *manager = ImeManagerComposer::GetInstance();
        manager->EnableIme(false);
    }
}
}
}