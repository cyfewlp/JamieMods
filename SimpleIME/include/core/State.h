#pragma once

#include "enumeration.h"
#include <atomic>
#include <cstdint>

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

            ~State()
            {
                g_instance.release();
            }

            auto Set(const StateKey &&state) -> void
            {
                m_state.set(state);
            }

            auto Clear(const StateKey &&state) -> void
            {
                m_state.reset(state);
            }

            auto Has(const StateKey &&state) const -> bool
            {
                return m_state.all(state);
            }

            template <typename... Args>
            auto HasAny(Args &&...state) const -> bool
            {
                return m_state.any(std::forward<Args>(state)...);
            }

            template <typename... Args>
            auto NotHas(Args &&...state) const -> bool
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

            auto SetEnableMod(bool enable) -> void
            {
                m_fModEnabled.store(enable);
            }

            [[nodiscard]] auto IsModEnabled() const -> bool
            {
                return m_fModEnabled.load();
            }

            static auto GetInstance() -> State *
            {
                if (g_instance == nullptr)
                {
                    g_instance = std::make_unique<State>();
                }
                return g_instance.get();
            }

        private:
            Enumeration<StateKey>         m_state;
            std::atomic_bool              m_fModEnabled = false;
            static std::unique_ptr<State> g_instance;
        };
    }
}