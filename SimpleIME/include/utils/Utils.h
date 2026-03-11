//
// Created by jamie on 2025/3/1.
//

#pragma once

#include "RE/GFxCharEvent.h"
#include "core/State.h"
#include "log.h"

namespace Ime
{
inline auto align_to(int value, int alignment) -> int
{
    return ((value + alignment - 1) / alignment) * alignment;
}

namespace Skyrim
{
inline void ToggleMenu(const RE::BSFixedString &a_menuName, bool show)
{
    if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue != nullptr)
    {
        messageQueue->AddMessage(a_menuName, show ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
}

inline void ShowMenu(const RE::BSFixedString &a_menuName)
{
    if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue != nullptr)
    {
        messageQueue->AddMessage(a_menuName, RE::UI_MESSAGE_TYPE::kShow, nullptr);
    }
}

inline void HideMenu(const RE::BSFixedString &a_menuName)
{
    if (auto *const messageQueue = RE::UIMessageQueue::GetSingleton(); messageQueue != nullptr)
    {
        messageQueue->AddMessage(a_menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
}

void SendUiString(std::wstring_view wstringView);

} // namespace Skyrim
} // namespace Ime
