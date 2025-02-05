
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

struct Logger
{
    template <typename T>
    static void log(spdlog::source_loc loc, spdlog::level::level_enum level, const T &value)
    {
        spdlog::log(loc, level, value);
    }

    template <typename... Types>
    static void log(spdlog::source_loc loc, spdlog::level::level_enum level, const char *const message,
                    const Types &...params)
    {
        auto fmt = std::vformat(std::string(message), std::make_format_args(params...));
        spdlog::log(loc, level, fmt.c_str());
    }
};

namespace LIBC_NAMESPACE_DECL
{
    template <typename enum_t, typename... Args>
    static constexpr auto logd(enum_t level, Args... args)
        requires(std::same_as<enum_t, spdlog::level::level_enum>)
    {
        Logger::log(spdlog::source_loc(__FILE__, __LINE__, __FUNCTION__), level, args...);
    }

    template <typename... Args>
    static constexpr auto log_error(Args... args)
    {
        logd(spdlog::level::err, args...);
    }

    template <typename... Args>
    static constexpr auto log_warn(Args... args)
    {
        logd(spdlog::level::warn, args...);
    }

    template <typename... Args>
    static constexpr auto log_info(Args... args)
    {
        logd(spdlog::level::info, args...);
    }

    template <typename... Args>
    static constexpr auto log_debug(Args... args)
    {
        logd(spdlog::level::debug, args...);
    }

    template <typename... Args>
    static constexpr auto log_trace(Args... args)
    {
        logd(spdlog::level::trace, args...);
    }

    namespace SimpleIME
    {
        class FontConfig
        {
            // default value
        public:
            static constexpr float DEFAULT_FONT_SIZE                = 14.0F;
            static constexpr int   DEFAULT_LOG_LEVEL                = SPDLOG_LEVEL_INFO;
            static constexpr int   DEFAULT_FLUSH_LEVEL              = SPDLOG_LEVEL_OFF;
            static constexpr auto  DEFAULT_FONT_FILE                = std::span(R"(C:\Windows\Fonts\simsun.ttc)");
            static constexpr auto  DEFAULT_EMOJI_FONT_FILE          = std::span(R"(C:\Windows\Fonts\seguiemj.ttf)");
            static constexpr char  DEFAULT_TOOL_WINDOW_SHORTCUT_KEY = DIK_F2;

        private:
            float            fontSize              = DEFAULT_FONT_SIZE;
            uint32_t         toolWindowShortcutKey = DEFAULT_TOOL_WINDOW_SHORTCUT_KEY; // 0x3C
            int              logLevel;
            int              flushLevel;
            std::string eastAsiaFontFile;
            std::string emojiFontFile;

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

            static auto parseLogLevel(int32_t intLevel) -> spdlog::level::level_enum
            {
                if (intLevel < 0 || intLevel >= static_cast<int>(spdlog::level::n_levels))
                {
                    log_error("Invalid spdlog level {}, downgrade to default: info", intLevel);
                    return spdlog::level::info;
                }
                return static_cast<spdlog::level::level_enum>(intLevel);
            }

            void of(CSimpleIniA &ini)
            {
                eastAsiaFontFile = ini.GetValue("General", "EastAsia_Font_File", DEFAULT_FONT_FILE.data());
                emojiFontFile    = ini.GetValue("General", "Emoji_Font_File", DEFAULT_EMOJI_FONT_FILE.data());
                fontSize         = static_cast<float>(ini.GetDoubleValue("General", "Font_Size", DEFAULT_FONT_SIZE));
                toolWindowShortcutKey =
                    ini.GetLongValue("General", "Tool_Window_Shortcut_Key", DEFAULT_TOOL_WINDOW_SHORTCUT_KEY);
                auto alogLevel   = ini.GetLongValue("General", "Log_Level", DEFAULT_LOG_LEVEL);
                auto aflushLevel = ini.GetLongValue("General", "Flush_Level", DEFAULT_FLUSH_LEVEL);
                logLevel         = parseLogLevel(alogLevel);
                flushLevel       = parseLogLevel(aflushLevel);
            }
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif