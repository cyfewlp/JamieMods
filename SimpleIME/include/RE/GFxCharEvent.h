#pragma once

#include <cstdint>

namespace RE
{
class GFxCharEvent : public RE::GFxEvent
{
public:
    GFxCharEvent() = default;

    explicit GFxCharEvent(std::uint32_t a_wcharCode, std::uint8_t a_keyboardIndex = 0)
        : GFxEvent(EventType::kCharEvent), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
    {
    }

    // @members
    std::uint32_t wcharCode{};     // 04
    std::uint8_t  keyboardIndex{}; // 08
};

static_assert(sizeof(GFxCharEvent) == 0x0C);
}
