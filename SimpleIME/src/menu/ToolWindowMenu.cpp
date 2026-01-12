//
// Created by jamie on 2026/1/2.
//
#include "menu/ToolWindowMenu.h"

#include "common/log.h"
#include "menu/MenuNames.h"

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
void ToolWindowMenu::RegisterMenu()
{
    log_info("Registering ToolWindowMenu...");
    if (auto *ui = RE::UI::GetSingleton(); ui != nullptr)
    {
        ui->Register(ToolWindowMenuName, Creator);
    }
}

RE::UI_MESSAGE_RESULTS ToolWindowMenu::ProcessMessage(RE::UIMessage &a_message)
{
    switch (a_message.type.get())
    {
        case RE::UI_MESSAGE_TYPE::kShow:
            g_showing = true;
            return RE::UI_MESSAGE_RESULTS::kHandled;
        case RE::UI_MESSAGE_TYPE::kHide:
            g_showing = false;
            return RE::UI_MESSAGE_RESULTS::kHandled;
        case RE::UI_MESSAGE_TYPE::kUserEvent: {
            const auto &data = reinterpret_cast<RE::BSUIMessageData *>(a_message.data);
            if (data->fixedStr == "Cancel")
            {
                if (auto messageQueue = RE::UIMessageQueue::GetSingleton())
                {
                    messageQueue->AddMessage(ToolWindowMenuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                }
            }
        }
        break;
        default:;
    }
    return IMenu::ProcessMessage(a_message);
}

auto ToolWindowMenu::Creator() -> IMenu *
{
    using flags = RE::UI_MENU_FLAGS;
    auto *pMenu = new ToolWindowMenu();
    pMenu->menuFlags.set(flags::kPausesGame, flags::kUsesCursor);
    pMenu->depthPriority = 11;
    // Priority 7: no render but no events
    // Priority 8: render but no events
    // Priority 9: render and have events

    return pMenu;
}

}
}