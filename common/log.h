//
// Created by jamie on 2025/2/22.
//

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#pragma once
#include <stacktrace>

#ifdef LIBC_NAMESPACE_DECL

    #include <cstdint>
    #include <format>
    #include <source_location>
    #include <spdlog/common.h>
    #include <spdlog/spdlog.h>

namespace LIBC_NAMESPACE_DECL
{
    struct format_string_loc
    {
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr format_string_loc(const char                 *aFmt,
                                    const std::source_location &location = std::source_location::current()) noexcept
            : m_value{aFmt}, m_loc(spdlog::source_loc{location.file_name(), static_cast<std::int32_t>(location.line()),
                                                      location.function_name()})
        {
        }

        [[nodiscard]] constexpr auto GetValue() const noexcept -> const std::string_view &
        {
            return m_value;
        }

        [[nodiscard]] constexpr auto GetLoc() const noexcept -> const spdlog::source_loc &
        {
            return m_loc;
        }

    private:
        std::string_view   m_value;
        spdlog::source_loc m_loc;
    } __attribute__((aligned(64)));

    template <typename EnumType, typename... Args>
    static constexpr auto logd(EnumType level, const format_string_loc &fsl, Args &&...args) noexcept
        requires(std::same_as<EnumType, spdlog::level::level_enum>)
    {
        if (spdlog::should_log(level))
        {
            auto fmt = std::vformat(fsl.GetValue(), std::make_format_args(args...));
            spdlog::log(fsl.GetLoc(), level, fmt.c_str());
        }
    }

    template <typename... Args>
    static constexpr auto log_critical(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::critical, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_error(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::err, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_warn(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::warn, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_info(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::info, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_debug(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::debug, fsl, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static constexpr auto log_trace(const format_string_loc &&fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::trace, fsl, std::forward<Args>(args)...);
    }

    inline void LogStacktrace()
    {
        for (const auto &stacktraceEntry : std::stacktrace::current())
        {
            log_error("  at: {}", to_string(stacktraceEntry));
        }
    }
} // namespace LIBC_NAMESPACE_DECL

#endif // LIBC_NAMESPACE_DECL
#endif // COMMON_LOG_H
