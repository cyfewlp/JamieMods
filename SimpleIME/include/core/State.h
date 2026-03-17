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
        // IME_OPEN               = 0x8, ///< [Deprecated] Specific to Imm32. Equivalence to TSF `Keyboard Open close`
        LANG_PROFILE_ACTIVATED = 0x10, ///< set when activate any non-english language profile
        IME_DISABLED           = 0x20, ///< if set, ignore any TextService change
        TEXT_SERVICE_FOCUS     = 0x40,
        GAME_LOADING           = 0x80,
        /// For chinese: Keyboard open -> active chinese input, keyboard close -> active english input.
        ///              Equivalence to press [shift] to toggle chinese/english input.
        /// For Japanese: Keyboard open -> active japanese input, keyboard close -> active english input.
        ///              Equivalence to press [alt + `] to toggle japanese/english input.
        KEYBOARD_OPEN          = 0x100, ///< if not set this, we should pass game event
        ALL                    = 0xFFFF
    };

    struct ConversionMode
    {
        enum class Flags : std::uint32_t
        {
            ALPHANUMERIC = 0x0000, ///< TF_CONVERSIONMODE_ALPHANUMERIC
            NATIVE       = 0x0001, ///< TF_CONVERSIONMODE_NATIVE
            KATAKANA     = 0x0002, ///< TF_CONVERSIONMODE_KATAKANA
            FULLSHAPE    = 0x0008, ///< TF_CONVERSIONMODE_FULLSHAPE
            ROMAN        = 0x0010, ///< TF_CONVERSIONMODE_ROMAN
            CHARCODE     = 0x0020, ///< TF_CONVERSIONMODE_CHARCODE
            SOFTKEYBOARD = 0x0080, ///< TF_CONVERSIONMODE_SOFTKEYBOARD
            NOCONVERSION = 0x0100, ///< TF_CONVERSIONMODE_NOCONVERSION
            EUDC         = 0x0200, ///< TF_CONVERSIONMODE_EUDC
            SYMBOL       = 0x0400, ///< TF_CONVERSIONMODE_SYMBOL
            FIXED        = 0x0800, ///< TF_CONVERSIONMODE_FIXED
        };
        using UnderlyingType = std::underlying_type<Flags>::type;

        ConversionMode() = default;

        explicit ConversionMode(Flags flags) : m_mode(static_cast<UnderlyingType>(flags)) {}

        friend ConversionMode operator|(ConversionMode::Flags lhs, ConversionMode::Flags rhs)
        {
            return ConversionMode{static_cast<Flags>(static_cast<UnderlyingType>(lhs) | static_cast<UnderlyingType>(rhs))};
        }

        friend ConversionMode operator|(ConversionMode lhs, ConversionMode::Flags rhs)
        {
            return ConversionMode{static_cast<Flags>(lhs.m_mode | static_cast<UnderlyingType>(rhs))};
        }

        [[nodiscard]] auto Has(Flags flag) const -> bool
        {
            const auto mask = static_cast<UnderlyingType>(flag);
            return (m_mode & mask) == mask;
        }

        auto RawValue() const -> UnderlyingType { return m_mode; }

        auto Set(UnderlyingType flags) -> void { m_mode = flags; }

        auto Clear() -> void { m_mode = static_cast<UnderlyingType>(Flags::ALPHANUMERIC); }

        auto Clear(Flags flag) -> void { m_mode &= (~static_cast<UnderlyingType>(flag)); }

        auto IsAlphanumeric() const -> bool { return m_mode == 0; }

        auto IsNative() const -> bool { return Has(Flags::NATIVE); }

        auto IsKatakana() const -> bool { return Has(Flags::KATAKANA); }

        auto IsFullShape() const -> bool { return Has(Flags::FULLSHAPE); }

        auto IsRoman() const -> bool { return Has(Flags::ROMAN); }

        auto IsCharCode() const -> bool { return Has(Flags::CHARCODE); }

        auto IsSoftKeyboard() const -> bool { return Has(Flags::SOFTKEYBOARD); }

        auto IsNoConversion() const -> bool { return Has(Flags::NOCONVERSION); }

        auto IsEudc() const -> bool { return Has(Flags::EUDC); }

        auto IsSymbol() const -> bool { return Has(Flags::SYMBOL); }

        auto IsFixed() const -> bool { return Has(Flags::FIXED); }

    private:
        UnderlyingType m_mode{static_cast<UnderlyingType>(Flags::ALPHANUMERIC)};
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

    [[nodiscard]] constexpr auto GetConversionMode() -> ConversionMode & { return m_conversionMode; }

    [[nodiscard]] constexpr auto GetConversionMode() const -> const ConversionMode & { return m_conversionMode; }

    [[nodiscard]] constexpr auto TsfFocus() const -> bool { return Has(TEXT_SERVICE_FOCUS); }

    [[nodiscard]] auto ImeDisabled() const -> bool { return Has(IME_DISABLED); }

    [[nodiscard]] constexpr auto IsKeyboardOpen() const { return Has(KEYBOARD_OPEN); }

    //! Only check is 'IN_COMPOSING' flag is set.
    [[nodiscard]] constexpr auto IsImeInputting() const { return Has(IN_COMPOSING); }

    static auto GetInstance() -> State &
    {
        static State g_instance;
        return g_instance;
    }

private:
    // UnderlyingType m_state{0};
    std::atomic<UnderlyingType> m_state{0};
    //! Missing thread safety. But it is not a big problem since the conversion mode is only updated in TextService thread,
    //! and read in UI thread. The worst case is UI thread get a stale conversion mode and update in next composition.
    ConversionMode              m_conversionMode;
};
} // namespace Ime::Core
