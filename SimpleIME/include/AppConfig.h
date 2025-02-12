
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma once

#include "Configs.h"
#include <yaml-cpp/yaml.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {

        constexpr std::string ConvertCamelCaseToUnderscore(const std::string &input);

        template <typename Type>
        struct Property
        {
            explicit constexpr Property(Type value, const std::string &varName);

            [[nodiscard]] constexpr Type Value() const
            {
                return value_;
            }

            [[nodiscard]] constexpr const char *VarName() const
            {
                return configName_.c_str();
            }

            [[nodiscard]] constexpr const char *ConfigName() const
            {
                return configName_.c_str();
            }

            constexpr void SetValue(const Type &value)
            {
                value_ = value;
            }

        private:
            Type        value_;
            std::string varName_;
            std::string configName_;
        };

        class AppUiConfig
        {
        public:
            [[nodiscard]] constexpr uint32_t TextColor() const
            {
                return textColor_.Value();
            }

            [[nodiscard]] constexpr uint32_t HighlightTextColor() const
            {
                return highlightTextColor_.Value();
            }

            [[nodiscard]] constexpr uint32_t WindowBorderColor() const
            {
                return windowBorderColor_.Value();
            }

            [[nodiscard]] constexpr uint32_t WindowBgColor() const
            {
                return windowBgColor_.Value();
            }

            [[nodiscard]] constexpr uint32_t BtnColor() const
            {
                return btnColor_.Value();
            }

            [[nodiscard]] constexpr std::string EastAsiaFontFile() const
            {
                return eastAsiaFontFile_.Value();
            }

            [[nodiscard]] constexpr std::string EmojiFontFile() const
            {
                return emojiFontFile_.Value();
            }

            [[nodiscard]] constexpr float FontSize() const
            {
                return fontSize_.Value();
            }

        private:
            friend class AppConfig;
            Property<float>       fontSize_{14.0F, "fontSize"};
            Property<uint32_t>    textColor_{0xFFCCCCCC, "textColor"};
            Property<uint32_t>    highlightTextColor_{0xFF00D7FF, "highlightTextColor"};
            Property<uint32_t>    windowBorderColor_{0xFF3A3A3A, "windowBorderColor"};
            Property<uint32_t>    windowBgColor_{0x801E1E1E, "windowBackgroundColor"};
            Property<uint32_t>    btnColor_{0xFF444444, "buttonColor"};
            Property<std::string> eastAsiaFontFile_{R"(C:\Windows\Fonts\simsun.ttc)", "eastAsiaFontFile"};
            Property<std::string> emojiFontFile_{R"(C:\Windows\Fonts\seguiemj.ttf)", "emojiFontFile_"};
        };

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
            static constexpr auto  DEFAULT_BUTTON_COL               = std::span("0xFF444444");

        private:
            Property<uint32_t> toolWindowShortcutKey{DEFAULT_TOOL_WINDOW_SHORTCUT_KEY, "toolWindowShortcutKey"}; // 0x3C
            Property<int>      logLevel_{DEFAULT_LOG_LEVEL, "logLevel"};
            Property<int>      flushLevel_{DEFAULT_FLUSH_LEVEL, "flushLevel"};

            AppUiConfig        appUiConfig_;

        public:
            AppConfig() = default;

            [[nodiscard]] auto GetLogLevel() const noexcept -> spdlog::level::level_enum
            {
                if (logLevel_.Value() < 0 || logLevel_.Value() >= spdlog::level::n_levels)
                {
                    return spdlog::level::info;
                }
                return static_cast<spdlog::level::level_enum>(logLevel_.Value());
            }

            [[nodiscard]] auto GetFlushLevel() const noexcept -> spdlog::level::level_enum
            {
                if (flushLevel_.Value() < 0 || flushLevel_.Value() >= spdlog::level::n_levels)
                {
                    return spdlog::level::info;
                }
                return static_cast<spdlog::level::level_enum>(flushLevel_.Value());
            }

            [[nodiscard]] auto GetToolWindowShortcutKey() const noexcept -> uint32_t
            {
                return toolWindowShortcutKey.Value();
            }

            [[nodiscard]] constexpr AppUiConfig &GetAppUiConfig()
            {
                return appUiConfig_;
            }

            static AppConfig *Load();

        private:
            static void                       LoadConfig();
            static void                       DoLoadConfig(YAML::Node &config) noexcept(false);
            static std::unique_ptr<AppConfig> appConfig_;
            static auto                       parseLogLevel(int32_t intLevel) -> spdlog::level::level_enum;
            uint32_t                          HexStringToUInt32(const std::string &hexStr, uint32_t aDefault = 0);
        };

    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif