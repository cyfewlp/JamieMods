#include "core/EventHandler.h"

#include "ImeWnd.hpp"
#include "RE/ControlMap.h"
#include "common/log.h"
#include "ime/ImeController.h"
#include "menu/MenuNames.h"

#include <RE/B/BSInputDeviceManager.h>
#include <RE/B/BSTEvent.h>
#include <RE/B/ButtonEvent.h>
#include <RE/C/CursorMenu.h>
#include <RE/I/InputDevices.h>
#include <RE/I/InputEvent.h>
#include <RE/M/MainMenu.h>
#include <RE/U/UI.h>

namespace Ime::Events
{

class InputEventSink : public RE::BSTEventSink<RE::InputEvent *>
{
    using Event = RE::InputEvent;
    using Keys  = RE::BSWin32MouseDevice::Keys;

public:
    explicit InputEventSink(ImeWnd *m_imeWnd, uint32_t shortcutKey) : m_imeWnd(m_imeWnd), m_shortcutKey(shortcutKey) {}

    RE::BSEventNotifyControl ProcessEvent(Event *const *event, RE::BSTEventSource<Event *> * /*eventSource*/) override;

private:
    ImeWnd  *m_imeWnd;
    uint32_t m_shortcutKey;

    void ProcessKeyboardEvent(const RE::ButtonEvent *btnEvent) const;
};

class MenuOpenCloseEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    using Event = RE::MenuOpenCloseEvent;

public:
    RE::BSEventNotifyControl ProcessEvent(const Event *a_event, RE::BSTEventSource<Event> *) override;

private:
    static void FixInconsistentTextEntryCount(const Event *event);
};

namespace
{
auto &GetInputEventSink()
{
    static std::unique_ptr<InputEventSink> instance = nullptr;
    return instance;
}

auto &GetMenuOpenCloseEventSink()
{
    static std::unique_ptr<MenuOpenCloseEventSink> instance = nullptr;
    return instance;
}

void OnFirstOpenMainMenu()
{
    if (auto *messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue)
    {
        messageQueue->AddMessage(ImeMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }
    ImeController::GetInstance()->ApplySettings();
}

void OnLoadingMenuClose()
{
    if (const auto messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue)
    {
        messageQueue->AddMessage(ImeMenuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }
}

void OnSteamOverlayMenu(bool open)
{
    static bool prevModEnabled = false;

    auto *manager = ImeController::GetInstance();
    if (open)
    {
        prevModEnabled = manager->IsModEnabled();
        manager->EnableMod(false);
    }
    else if (prevModEnabled)
    {
        manager->EnableMod(true);
    }
}

} // namespace

void InstallEventSinks(ImeWnd *imeWnd, const uint32_t shortcutKey)
{
    if (auto &inputSink = GetInputEventSink(); inputSink == nullptr)
    {
        if (const auto &deviceManager = RE::BSInputDeviceManager::GetSingleton(); deviceManager)
        {
            inputSink = std::make_unique<InputEventSink>(imeWnd, shortcutKey);
            deviceManager->AddEventSink(inputSink.get());
        }
    }
    if (auto &menuSink = GetMenuOpenCloseEventSink(); menuSink == nullptr)
    {
        if (auto *ui = RE::UI::GetSingleton(); ui)
        {
            menuSink = std::make_unique<MenuOpenCloseEventSink>();
            ui->AddEventSink(menuSink.get());
        }
    }
}

void UnInstallEventSinks()
{
    if (auto &inputSink = GetInputEventSink(); inputSink != nullptr)
    {
        if (const auto &deviceManager = RE::BSInputDeviceManager::GetSingleton(); deviceManager)
        {
            deviceManager->RemoveEventSink(inputSink.get());
        }
        inputSink.reset();
    }
    if (auto &menuSink = GetMenuOpenCloseEventSink(); menuSink != nullptr)
    {
        if (const auto &ui = RE::UI::GetSingleton(); ui)
        {
            ui->RemoveEventSink(menuSink.get());
        }
        menuSink.reset();
    }
}

//////////////////////////////////////////////////////////////////////////
// InputEventSink
//////////////////////////////////////////////////////////////////////////

RE::BSEventNotifyControl InputEventSink::ProcessEvent(Event *const *events, RE::BSTEventSource<Event *> *)
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
    if (const auto keyCode = btnEvent->GetIDCode(); /**/
        keyCode == m_shortcutKey && btnEvent->IsDown())
    {
        m_imeWnd->ToggleToolWindow();
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
    logger::debug("Menu {} open {}", event->menuName.c_str(), event->opening);
    static bool firstOpenMainMenu = true;
    if (event->menuName != RE::CursorMenu::MENU_NAME && event->menuName != RE::HUDMenu::MENU_NAME)
    {
        // FocusGFxCharacterInfo::GetInstance().Update(event->menuName.c_str(), event->opening);
    }
    // before game load, all menus will be closed;
    if (event->menuName == RE::LoadingMenu::MENU_NAME)
    {
        if (!event->opening)
        {
            OnLoadingMenuClose();
        }
    }
    else if (event->menuName == RE::MainMenu::MENU_NAME)
    {
        if (firstOpenMainMenu && event->opening)
        {
            OnFirstOpenMainMenu();
            firstOpenMainMenu = false;
        }
    }
    else if (event->menuName == RE::ConsoleNativeUIMenu::MENU_NAME) // Steam Overlay
    {
        OnSteamOverlayMenu(event->opening);
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
    if (event->menuName == RE::CursorMenu::MENU_NAME && /**/
        ControlMap::GetSingleton()->HasTextEntry() && !event->opening)
    {
        const ImeController *manager = ImeController::GetInstance();
        manager->EnableIme(false);
    }
}
} // namespace Ime::Events
