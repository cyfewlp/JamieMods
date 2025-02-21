//
// Created by jamie on 2025/2/3.
//

#ifndef ENUMERATION_H
#define ENUMERATION_H

#pragma once

#include <concepts>

namespace LIBC_NAMESPACE_DECL
{
    template <typename enum_t, typename Underlying = std::underlying_type_t<enum_t>>
    class Enumeration
    {
    public:
        using enum_type       = enum_t;
        using underlying_type = Underlying;

        static_assert(std::is_enum_v<enum_type>, "enum_type must be an enum");
        static_assert(std::is_integral_v<underlying_type>, "underlying_type must be an integral");

        constexpr Enumeration() noexcept                               = default;
        constexpr Enumeration(const Enumeration &) noexcept            = default;
        constexpr Enumeration(Enumeration &&) noexcept                 = default;
        constexpr Enumeration &operator=(const Enumeration &) noexcept = default;
        constexpr Enumeration &operator=(Enumeration &&) noexcept      = default;
        template <class U2>
        explicit constexpr Enumeration(Enumeration<enum_t, U2> a_rhs) noexcept = delete;
        template <class U2>
        constexpr Enumeration &operator=(Enumeration<enum_t, U2> a_rhs) noexcept = delete;

        constexpr bool operator==(enum_type a_value) const noexcept
        {
            return _impl == static_cast<underlying_type>(a_value);
        }

        template <class... Args>
        constexpr Enumeration(Args... a_values) noexcept //
            requires(std::same_as<Args, enum_type> && ...)
            : _impl((static_cast<underlying_type>(a_values) | ...))
        {
        }

        ~Enumeration() noexcept = default;

        constexpr Enumeration &operator=(enum_type a_value) noexcept
        {
            _impl = static_cast<underlying_type>(a_value);
            return *this;
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept
        {
            return _impl != static_cast<underlying_type>(0);
        }

        [[nodiscard]] constexpr enum_type operator*() const noexcept
        {
            return get();
        }

        [[nodiscard]] constexpr enum_type get() const noexcept
        {
            return static_cast<enum_type>(_impl);
        }

        [[nodiscard]] constexpr underlying_type underlying() const noexcept
        {
            return _impl;
        }

        template <class... Args>
        constexpr Enumeration &set(Args... a_args) noexcept //
            requires(std::same_as<Args, enum_type> && ...)
        {
            _impl |= (static_cast<underlying_type>(a_args) | ...);
            return *this;
        }

        template <class... Args>
        constexpr Enumeration &reset(Args... a_args) noexcept //
            requires(std::same_as<Args, enum_type> && ...)
        {
            _impl &= ~(static_cast<underlying_type>(a_args) | ...);
            return *this;
        }

        template <class... Args>
        [[nodiscard]] constexpr bool any(Args... a_args) const noexcept //
            requires(std::same_as<Args, enum_type> && ...)
        {
            return (_impl & (static_cast<underlying_type>(a_args) | ...)) != static_cast<underlying_type>(0);
        }

        template <class... Args>
        [[nodiscard]] constexpr bool all(Args... a_args) const noexcept //
            requires(std::same_as<Args, enum_type> && ...)
        {
            return (_impl & (static_cast<underlying_type>(a_args) | ...)) ==
                   (static_cast<underlying_type>(a_args) | ...);
        }

        template <class... Args>
        [[nodiscard]] constexpr bool none(Args... a_args) const noexcept //
            requires(std::same_as<Args, enum_type> && ...)
        {
            return (_impl & (static_cast<underlying_type>(a_args) | ...)) == static_cast<underlying_type>(0);
        }

    private:
        underlying_type _impl{0};
    };

    template <typename enum_t, typename... Flags, typename Underlying = std::underlying_type_t<enum_t>>
    auto all(enum_t &state, Flags... flags) -> bool
        requires(std::same_as<Flags, enum_t> && ...)
    {
        Underlying value1 = static_cast<Underlying>(state);
        Underlying value2 = (static_cast<Underlying>(flags) | ...);
        (value1 & value2);
        return (value1 & value2) == value2;
    }
} // namespace LIBC_NAMESPACE_DECL
#endif // HELLOWORLD_ENUMERATION_H
