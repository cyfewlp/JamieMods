//
// Created by jamie on 2026/1/2.
//

#pragma once

#include "RE/I/IMenu.h"

namespace Ime
{
// only be used pause game when ToolWindow is open;
class ToolWindowMenu final : public RE::IMenu
{
public:
    static void RegisterMenu();

    auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

private:
    static auto Creator() -> IMenu *;
};
} // namespace Ime
