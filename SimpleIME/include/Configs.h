
#ifndef CONFIGS_H
#define CONFIGS_H

#pragma once

#include "SimpleIni.h"
#include <cstdint>
#include <dinput.h>
#include <span>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

#define LIBC_NAMESPACE      __llvm_libc_SKSE_Plugin
#define LIBC_NAMESPACE_DECL [[gnu::visibility("hidden")]] LIBC_NAMESPACE

namespace LIBC_NAMESPACE_DECL
{
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

    static constexpr auto throw_fail(HRESULT hresult, const char *msg) noexcept(false)
    {
        if (FAILED(hresult))
        {
            throw std::runtime_error(msg);
        }
    }

    namespace SimpleIME
    {
        class AppConfig
        {
            // default value
        public:
            static constexpr float DEFAULT_FONT_SIZE                = 14.0F;
            static constexpr int   DEFAULT_LOG_LEVEL                = SPDLOG_LEVEL_INFO;
            static constexpr int   DEFAULT_FLUSH_LEVEL              = SPDLOG_LEVEL_OFF;
            static constexpr auto  DEFAULT_FONT_FILE                = std::span(R"(C:\Windows\Fonts\simsun.ttc)");
            static constexpr auto  DEFAULT_EMOJI_FONT_FILE          = std::span(R"(C:\Windows\Fonts\seguiemj.ttf)");
            static constexpr char  DEFAULT_TOOL_WINDOW_SHORTCUT_KEY = DIK_F2;
            static constexpr auto  DEFAULT_TEXT_COL                 = std::span("0xFFCCCCCC");
            static constexpr auto  DEFAULT_HIGHLIGHT_TEXT_COL       = std::span("0xFFFFD700");
            static constexpr auto  DEFAULT_WINDOW_BORDER_COL        = std::span("0xFF3A3A3A");
            static constexpr auto  DEFAULT_WINDOW_BG_COL            = std::span("0x801E1E1E");
            static constexpr auto  DEFAULT_BUTTON_COL            = std::span("0xFF444444");
            static constexpr auto  DEFAULT_BUTTON_HOVERED_COL            = std::span("0xFF666666");

        private:
            float       fontSize              = DEFAULT_FONT_SIZE;
            uint32_t    toolWindowShortcutKey = DEFAULT_TOOL_WINDOW_SHORTCUT_KEY; // 0x3C
            int         logLevel;
            int         flushLevel;
            std::string eastAsiaFontFile;
            std::string emojiFontFile;
            // ABGR
            std::uint32_t textColor;
            std::uint32_t highlightTextColor;
            std::uint32_t windowBorderColor;
            std::uint32_t windowBgColor;
            std::uint32_t btnColor;

        public:
            [[nodiscard]] auto GetFontSize() const noexcept -> float
            {
                return fontSize;
            }

            [[nodiscard]] auto GetLogLevel() const noexcept -> spdlog::level::level_enum
            {
                return static_cast<spdlog::level::level_enum>(logLevel);
            }

            [[nodiscard]] auto GetFlushLevel() const noexcept -> spdlog::level::level_enum
            {
                return static_cast<spdlog::level::level_enum>(flushLevel);
            }

            [[nodiscard]] auto GetEmojiFontFile() const noexcept -> std::string
            {
                return emojiFontFile;
            }

            [[nodiscard]] auto GetEastAsiaFontFile() const noexcept -> std::string
            {
                return eastAsiaFontFile;
            }

            [[nodiscard]] auto GetToolWindowShortcutKey() const noexcept -> uint32_t
            {
                return toolWindowShortcutKey;
            }

            [[nodiscard]] auto GetTextColor() const noexcept -> std::uint32_t
            {
                return textColor;
            }

            [[nodiscard]] auto GetHightlightTextColor() const noexcept -> std::uint32_t
            {
                return highlightTextColor;
            }

            [[nodiscard]] auto GetWindowBordrColor() const noexcept -> std::uint32_t
            {
                return windowBorderColor;
            }

            [[nodiscard]] auto GetWindowBgColor() const noexcept -> std::uint32_t
            {
                return windowBgColor;
            }

            [[nodiscard]] auto GetBtnColor() const noexcept -> std::uint32_t
            {
                return btnColor;
            }

            void of(CSimpleIniA &ini);

        private:
            static auto parseLogLevel(int32_t intLevel) -> spdlog::level::level_enum;
            uint32_t HexStringToUInt32(const std::string &hexStr, uint32_t aDefault = 0);
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif