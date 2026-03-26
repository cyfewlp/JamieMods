//
// Created by jamie on 2025/4/5.
//

#pragma once

#include "RE/I/IVirtualMachine.h"
#include "RE/U/UIMessageQueue.h"
#include "SosGuiMenu.h"

namespace SosGui
{
class PapyrusFunctions
{
public:
    static bool Register(RE::BSScript::IVirtualMachine *vm)
    {
        vm->RegisterFunction("ShowSosGuiMenu", "SosGuiNative", ShowSosGuiMenu);
        return true;
    }

    static constexpr void ShowSosGuiMenu(RE::StaticFunctionTag *)
    {
        if (auto *messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue != nullptr)
        {
            messageQueue->AddMessage(SosGuiMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
        }
    }
};
} // namespace SosGui
