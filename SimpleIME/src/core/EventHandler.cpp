#include "core/EventHandler.h"

#include "ImeWnd.hpp"
#include "Utils.h"
#include "configs/AppConfig.h"
#include "hooks/ScaleformHook.h"
#include "hooks/UiHooks.h"
#include "menu/MenuNames.h"
#include "ui/Settings.h"
#include "utils/FocusGFxCharacterInfo.h"

#include <RE/B/BSInputDeviceManager.h>
#include <RE/B/BSTEvent.h>
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
    if (uiHooks == nullptr || a_events == nullptr)
    {
        return;
    }
    auto *const head = *a_events;
    if (head == nullptr)
    {
        return;
    }
    const auto &state = State::GetInstance();
    if (!settings.enableMod || !state.IsKeyboardOpen() || state.IsImeNotActivateOrGameLoading())
    {
        uiHooks->EnableMessageFilter(false);
        return;
    }
    bool enableFilter = false;
    for (auto *event = head; event; event = event->next)
    {
        const RE::ButtonEvent *buttonEvent = nullptr;
        if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton)
        {
            buttonEvent = event->AsButtonEvent();
        }
        if (buttonEvent == nullptr)
        {
            continue;
        }
        const auto code = buttonEvent->GetIDCode();
        if (state.IsImeInputting() || (!Utils::IsCapsLockOn() && Utils::IsKeyWillTriggerIme(code)))
        {
            enableFilter = true;
            break;
        }
    }
    uiHooks->EnableMessageFilter(enableFilter);
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
            if (const auto *buttonEvent = event->AsButtonEvent(); buttonEvent != nullptr)
            {
                switch (buttonEvent->GetDevice())
                {
                    case RE::INPUT_DEVICE::kKeyboard:
                        ProcessKeyboardEvent(buttonEvent);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return RE::BSEventNotifyControl::kContinue;
}

void InputEventSink::ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent) const
{
    const auto keyCode = btnEvent->GetIDCode();
    if (keyCode == AppConfig::GetConfig().GetToolWindowShortcutKey() && btnEvent->IsDown())
    {
        m_imeWnd->ShowToolWindow();
        if (const auto messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            if (!m_imeWnd->IsPinedToolWindow() && m_imeWnd->IsShowingToolWindow())
            {
                messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
            }
            else
            {
                messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            }
        }
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
    // before game load, all menu will be closed;
    if (event->menuName == RE::LoadingMenu::MENU_NAME && !event->opening)
    {
        if (const auto messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            messageQueue->AddMessage(ImeMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }
    }
    if (firstOpenMainMenu && event->menuName == RE::MainMenu::MENU_NAME && event->opening)
    {
        if (const auto messageQueue = RE::UIMessageQueue::GetSingleton())
        {
            messageQueue->AddMessage(ImeMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }
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