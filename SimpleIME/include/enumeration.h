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

    constexpr Enumeration() noexcept                                        = default;
    constexpr Enumeration(const Enumeration &) noexcept                     = default;
    constexpr Enumeration(Enumeration &&) noexcept                          = default;
    constexpr auto operator=(const Enumeration &) noexcept -> Enumeration & = default;
    constexpr auto operator=(Enumeration &&) noexcept -> Enumeration &      = default;
    template <class U2>
    explicit constexpr Enumeration(Enumeration<enum_t, U2> a_rhs) noexcept = delete;
    template <class U2>
    constexpr auto operator=(Enumeration<enum_t, U2> a_rhs) noexcept -> Enumeration & = delete;

    constexpr auto operator==(enum_type a_value) const noexcept -> bool
    {
        return _impl == static_cast<underlying_type>(a_value);
    }

    template <class... Args>
    constexpr explicit Enumeration(Args... a_values) noexcept //
        requires(std::same_as<Args, enum_type> && ...)
        : _impl((static_cast<underlying_type>(a_values) | ...))
    {
    }

    ~Enumeration() noexcept = default;

    constexpr auto operator=(enum_type a_value) noexcept -> Enumeration &
    {
        _impl = static_cast<underlying_type>(a_value);
        return *this;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept
    {
        return _impl != static_cast<underlying_type>(0);
    }

    [[nodiscard]] constexpr auto operator*() const noexcept -> enum_type
    {
        return get();
    }

    [[nodiscard]] constexpr auto get() const noexcept -> enum_type
    {
        return static_cast<enum_type>(_impl);
    }

    [[nodiscard]] constexpr auto underlying() const noexcept -> underlying_type
    {
        return _impl;
    }

    template <class... Args>
    constexpr auto set(Args... a_args) noexcept -> Enumeration & //
        requires(std::same_as<Args, enum_type> &&...) {
            _impl |= (static_cast<underlying_type>(a_args) | ...);
            return *this;
        }

    template <class... Args>
    constexpr auto reset(Args... a_args) noexcept -> Enumeration & //
        requires(std::same_as<Args, enum_type> &&...) {
            _impl &= ~(static_cast<underlying_type>(a_args) | ...);
            return *this;
        }

    template <class... Args>
    [[nodiscard]] constexpr auto any(Args... a_args) const noexcept -> bool //
        requires(std::same_as<Args, enum_type> && ...)
    {
        return (_impl & (static_cast<underlying_type>(a_args) | ...)) != static_cast<underlying_type>(0);
    }

    template <class... Args>
    [[nodiscard]] constexpr auto all(Args... a_args) const noexcept -> bool //
        requires(std::same_as<Args, enum_type> && ...)
    {
        return (_impl & (static_cast<underlying_type>(a_args) | ...)) == (static_cast<underlying_type>(a_args) | ...);
    }

    template <class... Args>
    [[nodiscard]] constexpr auto none(Args... a_args) const noexcept -> bool //
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
    auto       value1 = static_cast<Underlying>(state);
    Underlying value2 = (static_cast<Underlying>(flags) | ...);
    (value1 & value2);
    return (value1 & value2) == value2;
}
} // namespace LIBC_NAMESPACE_DECL
#endif // ENUMERATION_H
