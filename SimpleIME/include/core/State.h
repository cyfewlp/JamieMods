#pragma once

#include "enumeration.h"

#include <cstdint>
#include <type_traits>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime::Core
{
class State
{
    State(const State &other)           = delete;
    State(State &&other)                = delete;
    State operator=(const State &other) = delete;
    State operator=(State &&other)      = delete;

public:
    enum StateKey : std::uint32_t
    {
        NONE                   = 0,
        IN_COMPOSING           = 0x1,
        IN_CAND_CHOOSING       = 0x2,
        IN_ALPHANUMERIC        = 0x4, // set when input method on english mode
        IME_OPEN               = 0x8,
        LANG_PROFILE_ACTIVATED = 0x10, // set when activate any non-english language profile
        IME_DISABLED           = 0x20, // if set, ignore any TextService change
        TSF_FOCUS              = 0x40,
        GAME_LOADING           = 0x80,
        KEYBOARD_OPEN          = 0x100, // if not set this, we should pass game event;
        ALL                    = 0xFFFF
    };

    State() = default;

    auto Set(StateKey state, bool set = true) -> void
    {
        if (set)
        {
            m_state.set(state);
        }
        else
        {
            m_state.reset(state);
        }
    }

    auto Clear(StateKey state) -> void
    {
        m_state.reset(state);
    }

    auto Has(StateKey state) const -> bool
    {
        return m_state.all(state);
    }

    template <typename... Args>
    auto HasAll(Args &&...state) const -> bool
        requires((std::is_same_v<Args, StateKey> && ...))
    {
        return m_state.all(std::forward<Args>(state)...);
    }

    template <typename... Args>
    auto HasAny(Args &&...state) const -> bool
        requires((std::is_same_v<Args, StateKey> && ...))
    {
        return m_state.any(std::forward<Args>(state)...);
    }

    template <typename... Args>
    auto NotHas(Args &&...state) const -> bool
        requires((std::is_same_v<Args, StateKey> && ...))
    {
        return m_state.none(std::forward<Args>(state)...);
    }

    [[nodiscard]] constexpr auto TsfFocus() const -> bool
    {
        return Has(TSF_FOCUS);
    }

    [[nodiscard]] auto ImeDisabled() const -> bool
    {
        return Has(IME_DISABLED);
    }

    static auto GetInstance() -> State &
    {
        static State g_instance;
        return g_instance;
    }

    constexpr auto IsKeyboardOpen() const
    {
        return Has(KEYBOARD_OPEN);
    }

    constexpr auto IsImeInputting() const
    {
        return HasAny(IN_CAND_CHOOSING, IN_COMPOSING);
    }

    constexpr auto IsImeNotActivateOrGameLoading() const
    {
        return HasAny(IME_DISABLED, IN_ALPHANUMERIC, GAME_LOADING) || NotHas(LANG_PROFILE_ACTIVATED);
    }

private:
    Enumeration<StateKey> m_state;
};
}
}