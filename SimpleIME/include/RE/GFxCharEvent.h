#pragma once

#include "log.h"

#include <cstdint>

namespace Ime
{

/**
 * @brief Custom GFx event type for intercepting character input from SimpleIME.
 * * **Conflict Avoidance:**
 * These events use the @c 0x80000000 range to prevent ID collisions with native
 * GFx event types.
 * * **Dispatcher Logic:**
 * This custom ID allows us to intercept events for specialized handling:
 * - **Resource Tracking:** Collecting events to ensure proper cleanup and prevent memory leaks.
 * - **Input Redirection:** Directly routing events to ImGui when the @c ToolWindowMenu is active.
 * * @note **Why use Custom IDs instead of Menu Names?**
 * Skyrim's event dispatcher typically overwrites the origin menu name with "TopMenu"
 * during transmission. This makes menu-based filtering unreliable. By using
 * unique event IDs, we remain **menu-agnostic** and avoid the performance
 * overhead of registering entirely new message types.
 *
 * @warning While slightly unconventional, this approach is safe provided these IDs
 * are reserved exclusively for SimpleIME's internal signaling.
 */
enum class GFxEventTypeEx : std::uint32_t
{
    kImeCharEvent = 0x80000001, ///< event type for char event sent by SimpleIME.
    kImeKeyUp,                  ///< event type for key event sent by SimpleIME.
};

class GFxCharEvent : public RE::GFxEvent
{
public:
    GFxCharEvent() = default;

    explicit GFxCharEvent(std::uint32_t a_wcharCode, std::uint8_t a_keyboardIndex = 0)
        : GFxEvent(static_cast<RE::GFxEvent::EventType>(Ime::GFxEventTypeEx::kImeCharEvent)), wcharCode(a_wcharCode), keyboardIndex(a_keyboardIndex)
    {
    }

    // @members
    std::uint32_t wcharCode{};     // 04
    std::uint8_t  keyboardIndex{}; // 08
};

static_assert(sizeof(GFxCharEvent) == 0x0C);

} // namespace Ime
