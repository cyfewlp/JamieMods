
#ifndef CONFIGS_H
#define CONFIGS_H

#pragma once

#include <cstdint>
#include <dinput.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#define LIBC_NAMESPACE        __llvm_libc_SKSE_Plugin
#define LIBC_NAMESPACE_DECL   [[gnu::visibility("hidden")]] LIBC_NAMESPACE

#define PLUGIN_BASE_NAMESPACE SimpleIME
#define PLUGIN_NAMESPACE      LIBC_NAMESPACE::PLUGIN_BASE_NAMESPACE

#define WM_CUSTOM             0x7000

namespace LIBC_NAMESPACE_DECL
{
    enum CustomMessage
    {
        CM_CHAR = WM_CUSTOM + 1,
        CM_IME_CHAR,
        CM_IME_COMPOSITION,
        CM_ACTIVATE_PROFILE
    };

    struct format_string_loc
    {
    public:
        constexpr format_string_loc(const char                *a_fmt,
                                    const std::source_location location = std::source_location::current()) noexcept
            : value{a_fmt}, loc(spdlog::source_loc{location.file_name(), static_cast<std::int32_t>(location.line()),
                                                   location.function_name()})
        {
        }

        [[nodiscard]] constexpr auto GetValue() const noexcept -> const std::string_view &
        {
            return value;
        }

        [[nodiscard]] constexpr auto GetLoc() const noexcept -> const spdlog::source_loc &
        {
            return loc;
        }

    private:
        std::string_view   value;
        spdlog::source_loc loc;
    };

    template <typename enum_t, typename... Args>
    static constexpr auto logd(enum_t level, const format_string_loc &fsl, Args &&...args) noexcept
        requires(std::same_as<enum_t, spdlog::level::level_enum>)
    {
        auto fmt = std::vformat(fsl.GetValue(), std::make_format_args(args...));
        spdlog::log(fsl.GetLoc(), level, fmt.c_str());
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
    static constexpr auto log_trace(const format_string_loc fsl, Args &&...args) noexcept
    {
        logd(spdlog::level::trace, fsl, std::forward<Args>(args)...);
    }

    bool PluginInit();

    void InitializeMessaging();


} // namespace LIBC_NAMESPACE_DECL

#endif