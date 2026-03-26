//
// Created by jamie on 2026/1/2.
//
#include "menu/ToolWindowMenu.h"

#include "ImeApp.h"
#include "ime/ImeController.h"
#include "log.h"
#include "menu/MenuNames.h"
#include "utils/Utils.h"

namespace Ime
{
namespace
{

auto OnUserEvent(RE::BSUIMessageData *uiMessageData) -> RE::UI_MESSAGE_RESULTS
{
    const RE::UserEvents *userEvents = RE::UserEvents::GetSingleton();

    auto &runtimeData = ImeApp::GetInstance().GetSettings().runtimeData;
    if (uiMessageData->fixedStr == userEvents->cancel)
    {
        runtimeData.requestCloseTopWindow = true;
        return RE::UI_MESSAGE_RESULTS::kHandled;
    }
    return RE::UI_MESSAGE_RESULTS::kPassOn;
}

} // namespace

void ToolWindowMenu::RegisterMenu()
{
    logger::info("Registering ToolWindowMenu...");
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        ui->Register(ToolWindowMenuName, Creator);
    }
}

auto ToolWindowMenu::ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS
{
    if (!ImeApp::GetInstance().GetState().IsInitialized())
    {
        return RE::UI_MESSAGE_RESULTS::kPassOn;
    }
    RE::UI_MESSAGE_RESULTS results = RE::UI_MESSAGE_RESULTS::kPassOn;
    switch (a_message.type.get())
    {
        case RE::UI_MESSAGE_TYPE::kShow: {
            results = RE::UI_MESSAGE_RESULTS::kHandled;
            break;
        }
        case RE::UI_MESSAGE_TYPE::kHide: {
            ImeController::GetInstance()->SyncImeState(); // TODO: really need this?
            results = RE::UI_MESSAGE_RESULTS::kHandled;
            break;
        }
        case RE::UI_MESSAGE_TYPE::kUserEvent: {
            auto *uiMessageData = reinterpret_cast<RE::BSUIMessageData *>(a_message.data);
            results             = OnUserEvent(uiMessageData);
            break;
        }
        default:;
    }
    return results;
}

auto ToolWindowMenu::Creator() -> IMenu *
{
    auto *pMenu = new ToolWindowMenu();
    pMenu->menuFlags.set(Flag::kPausesGame, Flag::kUsesCursor);
    pMenu->depthPriority = 11;

    // pMenu->inputContext.set(RE::UserEvents::INPUT_CONTEXT_ID::kMenuMode);
    // Priority 7: no render but no events
    // Priority 8: render but no events
    // Priority 9: render and have events
    return pMenu;
}

} // namespace Ime
