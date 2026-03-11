//
// Created by jamie on 2026/3/11.
//
#include "Utils.h"

#include "RE/B/BSTDerivedCreator.h"
#include "RE/B/BSUIScaleformData.h"
#include "RE/G/GFxEvent.h"
#include "RE/GFxCharEvent.h"
#include "RE/I/InterfaceStrings.h"
#include "RE/U/UIMessageQueue.h"
#include "menu/MenuNames.h"

namespace Ime::Skyrim
{
namespace
{
constexpr auto ASCII_GRAVE_ACCENT = 0x60U; // `
constexpr auto ASCII_MIDDLE_DOT   = 0xB7U; // ·

using ScaleformMessageCreator = RE::BSTDerivedCreator<RE::BSUIScaleformData, RE::IUIMessageData>;

auto Send(RE::BSUIScaleformData *scaleformData, const uint32_t code, RE::UIMessageQueue *messageQueue, const RE::BSFixedString &menuName) -> bool
{
    auto *charEvent               = new GFxCharEvent(code);
    scaleformData->scaleformEvent = charEvent;

    logger::debug("send code {:#x} to Skyrim", code);
    messageQueue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kScaleformEvent, scaleformData);
    return true;
}
} // namespace

void SendUiString(std::wstring_view wstringView)
{
    if (wstringView.empty()) return;

    auto             *messageQueue  = RE::UIMessageQueue::GetSingleton();
    const auto *const strings       = RE::InterfaceStrings::GetSingleton();
    auto             *msgFactoryMgr = RE::MessageDataFactoryManager::GetSingleton();
    if (strings == nullptr || messageQueue == nullptr || msgFactoryMgr == nullptr)
    {
        logger::warn("Can't send string to Skyrim. May game already closed?");
        return;
    }
    const auto *scaleformDataCreator = msgFactoryMgr->GetCreator<RE::BSUIScaleformData>(strings->bsUIScaleformData);
    if (scaleformDataCreator == nullptr)
    {
        logger::warn("Unexpected error. Can't create scaleform message data creator!");
        return;
    }

    for (const wchar_t c : wstringView)
    {
        const auto wcharCode = static_cast<uint32_t>(c);
        if (wcharCode == ASCII_GRAVE_ACCENT || wcharCode == ASCII_MIDDLE_DOT)
        {
            continue;
        }
        auto *scaleformData = scaleformDataCreator->Create();
        if (scaleformData != nullptr)
        {
            Send(scaleformData, wcharCode, messageQueue, ImeMenuName);
        }
        else
        {
            logger::error("Unexpected error. Can't create scaleform message data!");
            break;
        }
    }
}

} // namespace Ime::Skyrim
