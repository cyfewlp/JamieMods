#include "core/EventHandler.h"

#include "ImeWnd.hpp"
#include "RE/ControlMap.h"
#include "ime/ImeController.h"
#include "log.h"
#include "menu/MenuNames.h"
#include "utils/Utils.h"

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
class MenuOpenCloseEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    using Event = RE::MenuOpenCloseEvent;

public:
    auto ProcessEvent(const Event *a_event, RE::BSTEventSource<Event> * /*a_eventSource*/) -> RE::BSEventNotifyControl override;

private:
    static void FixInconsistentTextEntryCount(const Event *event);
};

namespace
{
auto GetMenuOpenCloseEventSink() -> std::unique_ptr<MenuOpenCloseEventSink> &
{
    static std::unique_ptr<MenuOpenCloseEventSink> instance = nullptr;
    return instance;
}

void OnFirstOpenMainMenu()
{
    Skyrim::ShowMenu(ImeMenuName);
    ImeController::GetInstance()->ApplySettings();
}

void OnLoadingMenuClose()
{
    Skyrim::ShowMenu(ImeMenuName);
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

void InstallEventSinks()
{
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
// MenuOpenCloseEventSink
//////////////////////////////////////////////////////////////////////////

auto MenuOpenCloseEventSink::ProcessEvent(const Event *event, RE::BSTEventSource<Event> * /*eventSource*/) -> RE::BSEventNotifyControl
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
