#pragma once

#include <cstdint>
#include <type_traits>

namespace Ime::Core
{
// \todo may replace this singleton with an explicit context object passed through the call stack.
class State
{
public:
    enum StateKey : std::uint32_t
    {
        NONE                   = 0,
        IN_COMPOSING           = 0x1, ///< set when composition is active. UI should not rely on this state.
        IN_CAND_CHOOSING       = 0x2, ///< set when candidate list is active. UI should not rely on this state.
        IN_ALPHANUMERIC        = 0x4, ///< set when input method on english mode
        IME_OPEN               = 0x8,
        LANG_PROFILE_ACTIVATED = 0x10, ///< set when activate any non-english language profile
        IME_DISABLED           = 0x20, ///< if set, ignore any TextService change
        TEXT_SERVICE_FOCUS     = 0x40,
        GAME_LOADING           = 0x80,
        KEYBOARD_OPEN          = 0x100, ///< if not set this, we should pass game event
        ALL                    = 0xFFFF
    };

    using UnderlyingType = std::underlying_type_t<StateKey>;

    auto Set(StateKey state, bool set = true) -> void
    {
        if (set)
        {
            m_state |= static_cast<UnderlyingType>(state);
        }
        else
        {
            m_state &= ~static_cast<UnderlyingType>(state);
        }
    }

    auto Clear(StateKey state) -> void { m_state &= ~static_cast<UnderlyingType>(state); }

    [[nodiscard]] auto Has(StateKey state) const -> bool
    {
        const auto mask = static_cast<UnderlyingType>(state);
        return (m_state & mask) == mask;
    }

    template <typename... Args>
    [[nodiscard]] auto HasAll(Args... states) const -> bool
        requires((std::is_same_v<Args, StateKey> && ...))
    {
        const auto mask = (static_cast<UnderlyingType>(states) | ...);
        return (m_state & mask) == mask;
    }

    template <typename... Args>
    [[nodiscard]] auto HasAny(Args... states) const -> bool
        requires((std::is_same_v<Args, StateKey> && ...))
    {
        const auto mask = (static_cast<UnderlyingType>(states) | ...);
        return (m_state & mask) != 0U;
    }

    template <typename... Args>
    [[nodiscard]] auto NotHas(Args... states) const -> bool
        requires((std::is_same_v<Args, StateKey> && ...))
    {
        const auto mask = (static_cast<UnderlyingType>(states) | ...);
        return (m_state & mask) == 0U;
    }

    [[nodiscard]] constexpr auto TsfFocus() const -> bool { return Has(TEXT_SERVICE_FOCUS); }

    [[nodiscard]] auto ImeDisabled() const -> bool { return Has(IME_DISABLED); }

    [[nodiscard]] constexpr auto IsKeyboardOpen() const { return Has(KEYBOARD_OPEN); }

    [[nodiscard]] constexpr auto IsImeInputting() const { return HasAny(IN_CAND_CHOOSING, IN_COMPOSING); }

    [[nodiscard]] constexpr auto IsImeWaitingInput() const { return NotHas(IME_DISABLED, IN_ALPHANUMERIC); }

    [[nodiscard]] constexpr auto IsImeNotActivateOrGameLoading() const
    {
        return HasAny(IME_DISABLED, IN_ALPHANUMERIC, GAME_LOADING) || NotHas(LANG_PROFILE_ACTIVATED);
    }

    static auto GetInstance() -> State &
    {
        static State g_instance;
        return g_instance;
    }

private:
    UnderlyingType m_state{0};
};
} // namespace Ime::Core
