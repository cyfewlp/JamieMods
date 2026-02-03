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
    inline static bool g_showing = false;

public:
    static void RegisterMenu();

    auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

    static constexpr bool IsShowing()
    {
        return g_showing;
    }

private:
    static auto Creator() -> IMenu *;
};
} // namespace Ime
