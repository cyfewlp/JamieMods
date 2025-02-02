
#ifndef _SIMPLE_IME_CONFIGS_
#define _SIMPLE_IME_CONFIGS_

#pragma once

#include "SimpleIni.h"
#include <dinput.h>
#include <spdlog/common.h>
#include <string>

#define LIBC_NAMESPACE      __llvm_libc_SKSE_Plugin
#define LIBC_NAMESPACE_DECL [[gnu::visibility("hidden")]] LIBC_NAMESPACE

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        class FontConfig
        {
            // default value
        public:
            static constexpr float            DEFAULT_FONT_SIZE   = 14.0F;
            static constexpr int              DEFAULT_LOG_LEVEL   = SPDLOG_LEVEL_INFO;
            static constexpr int              DEFAULT_FLUSH_LEVEL = SPDLOG_LEVEL_OFF;
            static constexpr std::string_view DEFAULT_FONT_FILE{R"(C:\Windows\Fonts\simsun.ttc)"};
            static constexpr std::string_view DEFAULT_EMOJI_FONT_FILE{R"(C:\Windows\Fonts\seguiemj.ttf)"};
            static constexpr char             DEFAULT_TOOL_WINDOW_SHORTCUT_KEY = DIK_F2;

        private:
            float            fontSize              = DEFAULT_FONT_SIZE;
            uint32_t         toolWindowShortcutKey = DEFAULT_TOOL_WINDOW_SHORTCUT_KEY; // 0x3C
            int              logLevel              = DEFAULT_LOG_LEVEL;
            int              flushLevel            = DEFAULT_FLUSH_LEVEL;
            std::string_view eastAsiaFontFile;
            std::string_view emojiFontFile;

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

            [[nodiscard]] auto GetEmojiFontFile() const noexcept -> std::string_view
            {
                return emojiFontFile;
            }

            [[nodiscard]] auto GetEastAsiaFontFile() const noexcept -> std::string_view
            {
                return eastAsiaFontFile;
            }

            [[nodiscard]] auto GetToolWindowShortcutKey() const noexcept -> uint32_t
            {
                return toolWindowShortcutKey;
            }

            static auto intToLevel(int intLevel) -> spdlog::level::level_enum
            {
                if (intLevel < 0 || intLevel >= static_cast<int>(spdlog::level::n_levels))
                {
                    logv(err, "Invalid spdlog level {}, downgrade to default: info", intLevel);
                    return spdlog::level::info;
                }
                return static_cast<spdlog::level::level_enum>(intLevel);
            }

            void of(CSimpleIniA &ini)
            {
                eastAsiaFontFile = ini.GetValue("General", "EastAsia_Font_File", DEFAULT_FONT_FILE.data());
                emojiFontFile    = ini.GetValue("General", "Emoji_Font_File", DEFAULT_EMOJI_FONT_FILE.data());
                fontSize         = (float)ini.GetDoubleValue("General", "Font_Size", DEFAULT_FONT_SIZE);
                toolWindowShortcutKey =
                    ini.GetLongValue("General", "Tool_Window_Shortcut_Key", DEFAULT_TOOL_WINDOW_SHORTCUT_KEY);
                auto alogLevel   = ini.GetLongValue("General", "Log_Level", DEFAULT_LOG_LEVEL);
                auto aflushLevel = ini.GetLongValue("General", "Flush_Level", DEFAULT_FLUSH_LEVEL);
                logLevel         = intToLevel(alogLevel);
                flushLevel       = intToLevel(aflushLevel);
            }
        };
    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL

#endif