//
// Created by jamie on 2026/2/5.
//

#pragma once

#include "RE/B/BSTEvent.h"
#include "RE/B/BSTSingleton.h"
#include "RE/C/ControlMap.h"
#include "RE/U/UserEventEnabled.h"

#include <cstdint>

namespace Ime
{
class ControlMap : public RE::BSTSingletonSDM<ControlMap>,         // 00
                   public RE::BSTEventSource<RE::UserEventEnabled> // 08
{
    using CM = RE::ControlMap;

public:
    // NOLINTBEGIN( misc-non-private-member-variables-in-classes)
    CM::InputContext                *controlMap[CM::InputContextID::kTotal]; // 060
    RE::BSTArray<CM::LinkedMapping>  linkedMappings;                         // 0E8
    RE::BSTArray<CM::InputContextID> contextPriorityStack;                   // 100
    uint32_t                         enabledControls;                        // 118
    uint32_t                         unk11C;                                 // 11C
    std::uint8_t                     textEntryCount;                         // 120
    bool                             ignoreKeyboardMouse;                    // 121
    bool                             ignoreActivateDisabledEvents;           // 122
    std::uint8_t                     pad123;                                 // 123
    uint32_t                         gamePadMapType;                         // 124
    uint8_t                          allowTextInput;                         // 128
    uint8_t                          unk129;                                 // 129
    uint8_t                          unk12A;                                 // 12A
    uint8_t                          pad12B;                                 // 12B
    uint32_t                         unk12C;                                 // 12C
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    // A SKSE copy: InputManager::AllowTextInput
    auto SKSE_AllowTextInput(bool allow) -> uint8_t;

    auto GetTextEntryCount() const -> uint8_t;

    auto HasTextEntry() const -> bool
    {
        return GetTextEntryCount() > 0;
    }

    static auto GetSingleton() -> ControlMap *;

private:
    static void DoAllowTextInput(bool allow, std::uint8_t &entryCount);
};
} // namespace Ime
