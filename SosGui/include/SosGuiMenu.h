//
// Created by jamie on 2025/4/4.
//

#ifndef SOSGUIMENU_H
#define SOSGUIMENU_H

#pragma once
#include "SimpleIME/include/ImGuiThemeLoader.h"

namespace LIBC_NAMESPACE_DECL
{

    class SosGuiMenu : public RE::IMenu
    {
    public:
        static constexpr std::string_view MENU_NAME = "SosGuiMenu";
        static void                       RegisterMenu();

        void PostDisplay() override;
        auto ProcessMessage(RE::UIMessage &a_message) -> RE::UI_MESSAGE_RESULTS override;

    private:
        static auto Creator() -> IMenu *;
    };

}

#endif // SOSGUIMENU_H
