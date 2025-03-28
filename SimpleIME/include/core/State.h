#pragma once

#include "enumeration.h"
#include <atomic>
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
                ALL                    = 0xFFFF
            };

            State() = default;

            auto Set(StateKey state) -> void
            {
                m_state.set(state);
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
                return Has(StateKey::TSF_FOCUS);
            }

            [[nodiscard]] auto ImeDisabled() const -> bool
            {
                return Has(StateKey::IME_DISABLED);
            }

            auto SetSupportOtherMod(bool enable) -> void
            {
                m_fSupportOtherMod.store(enable);
            }

            [[nodiscard]] auto IsSupportOtherMod() const -> bool
            {
                return m_fSupportOtherMod.load();
            }

            auto SetEnableUnicodePaste(bool enable) -> void
            {
                m_fEnableUnicodePaste.store(enable);
            }

            [[nodiscard]] auto IsEnableUnicodePaste() const -> bool
            {
                return m_fEnableUnicodePaste.load();
            }

            static auto GetInstance() -> State &
            {
                static State g_instance;
                return g_instance;
            }

        private:
            Enumeration<StateKey> m_state;
            std::atomic_bool      m_fSupportOtherMod    = false;
            std::atomic_bool      m_fEnableUnicodePaste = false;
        };
    }
}