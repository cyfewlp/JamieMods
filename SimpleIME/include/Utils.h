//
// Created by jamie on 2025/3/1.
//

#pragma once

#include "RE/GFxCharEvent.h"
#include "core/State.h"
#include "log.h"

namespace Ime
{
inline int align_to(int value, int alignment)
{
    return ((value + alignment - 1) / alignment) * alignment;
}

class Utils
{
    static constexpr auto ASCII_GRAVE_ACCENT = 0x60; // `
    static constexpr auto ASCII_MIDDLE_DOT   = 0xB7; // ·
    using State                              = Core::State;

public:
    template <typename Ptr>
    static auto ToLongPtr(Ptr *ptr) -> LONG_PTR
    {
        return reinterpret_cast<LONG_PTR>(ptr);
    }

    static constexpr auto IsCapsLockOn() -> bool
    {
        SHORT capsState = GetKeyState(VK_CAPITAL);
        return (capsState & 0x0001) != 0;
    }

    template <typename String>
    static void SendStringToGame(String &&sourceString)
    {
        logger::debug("Ready result string to Skyrim...");
        auto *pInterfaceStrings = RE::InterfaceStrings::GetSingleton();
        auto *pFactoryManager   = RE::MessageDataFactoryManager::GetSingleton();
        if (pInterfaceStrings == nullptr || pFactoryManager == nullptr)
        {
            logger::warn("Can't send string to Skyrim may game already close?");
            return;
        }

        const auto *pFactory = pFactoryManager->GetCreator<RE::BSUIScaleformData>(pInterfaceStrings->bsUIScaleformData);
        if (pFactory == nullptr)
        {
            logger::warn("Can't send string to Skyrim may game already close?");
            return;
        }

        // Start send message
        for (uint32_t const code : sourceString)
        {
            if (!Send(pFactory, code))
            {
                return;
            }
        }
    }

private:
    static auto Send(auto *pFactory, const uint32_t code) -> bool
    {
        if (code == ASCII_GRAVE_ACCENT || code == ASCII_MIDDLE_DOT)
        {
            return true;
        }
        auto *pCharEvent            = new RE::GFxCharEvent(code);
        pCharEvent->type            = static_cast<RE::GFxEvent::EventType>(GFxEventTypeEx::kImeCharEvent);
        auto *pScaleFormMessageData = pFactory->Create();
        if (pScaleFormMessageData == nullptr)
        {
            logger::error("Unable create BSTDerivedCreator.");
            return false;
        }
        pScaleFormMessageData->scaleformEvent = pCharEvent;
        logger::debug("send code {:#x} to Skyrim", code);
        RE::UIMessageQueue::GetSingleton()->AddMessage(
            RE::InterfaceStrings::GetSingleton()->topMenu, RE::UI_MESSAGE_TYPE::kScaleformEvent, pScaleFormMessageData
        );
        return true;
    }
};
} // namespace Ime
