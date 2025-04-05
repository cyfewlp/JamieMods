//
// Created by jamie on 2025/4/5.
//

#ifndef PAPYRUSFUNCTIONS_H
#define PAPYRUSFUNCTIONS_H

#pragma once

#include "PapyrusEvent.h"
#include "SosGuiMenu.h"

#include "RE/I/IVirtualMachine.h"
#include "RE/U/UIMessageQueue.h"

namespace LIBC_NAMESPACE_DECL
{
    class PapyrusFunctions
    {
    public:
        static bool Register(RE::BSScript::IVirtualMachine *vm)
        {
            if (!PapyrusEvent::Bind(vm))
            {
                return false;
            }
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
}

#endif // PAPYRUSFUNCTIONS_H
