#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma once

#include "common/config.h"
#include "configs/converter.h"

#include <SimpleIni.h>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        constexpr auto ConvertCamelCaseToUnderscore(const std::string &input) -> std::string
        {
            if (input.empty())
            {
                return {};
            }

            std::string output;
            output.reserve(input.size() * 2);

            char first = input[0];
            if (std::isupper(static_cast<unsigned char>(first)) == 0)
            {
                first = static_cast<char>(std::toupper(static_cast<unsigned char>(first)));
            }
            output.push_back(first);
            bool       toUpper   = false;
            const auto lastIndex = input.size() - 1;
            for (size_t i = 1; i < input.size(); ++i)
            {
                char      c     = input[i];
                const int iChar = static_cast<unsigned char>(c);
                if (toUpper)
                {
                    c       = static_cast<char>(std::toupper(iChar));
                    toUpper = false;
                }
                else if (std::isupper(iChar) && (i < lastIndex && std::islower(input[i + 1])))
                {
                    output.push_back('_');
                }
                if (c == '_')
                {
                    toUpper = true;
                }
                output.push_back(c);
            }

            return output;
        }

        template <typename Type>
        struct Property
        {
            constexpr Property(Type value, const std::string &varName) : m_value(std::move(value))
            {
                m_configName = ConvertCamelCaseToUnderscore(varName);
            }

            [[nodiscard]] constexpr const Type &Value() const
            {
                return m_value;
            }

            [[nodiscard]] constexpr const char *ConfigName() const
            {
                return m_configName.c_str();
            }

            constexpr void SetValue(const Type &value)
            {
                m_value = value;
            }

        private:
            Type        m_value;
            std::string m_configName{};
        };

        template <typename Type>
        void GetSimpleIniValue(CSimpleIniA &ini, const char *section, Property<Type> &property)
        {
            auto strV = ini.GetValue(section, property.ConfigName());
#ifdef SIMPLE_IME_DEBUG
            assert(strV != nullptr);
#endif
            if (strV != nullptr)
            {
                auto result = converter<Type>::convert(strV, property.Value());
                property.SetValue(result);
            }
        }

#define PROPERTY_VAR(varName, value)                                                                                   \
    varName##_                                                                                                         \
    {                                                                                                                  \
        value, #varName                                                                                                \
    }

        class AppUiConfig
        {
        public:
            AppUiConfig()                                      = default;
            AppUiConfig(const AppUiConfig &other)              = default;
            AppUiConfig(const AppUiConfig &&rvalue)            = delete;
            AppUiConfig &operator=(const AppUiConfig &other)   = default;
            AppUiConfig &operator=(const AppUiConfig &&rvalue) = delete;
            ~AppUiConfig()                                     = default;

            [[nodiscard]] constexpr auto EastAsiaFontFile() const -> const std::string &
            {
                return eastAsiaFontFile_.Value();
            }

            [[nodiscard]] constexpr auto EmojiFontFile() const -> const std::string &
            {
                return emojiFontFile_.Value();
            }

            [[nodiscard]] constexpr const float &FontSize() const
            {
                return fontSize_.Value();
            }

            [[nodiscard]] auto UseClassicTheme() const -> bool
            {
                return useClassicTheme_.Value();
            }

            [[nodiscard]] auto ThemeDirectory() const -> const std::string &
            {
                return m_themeDirectory.Value();
            }

            [[nodiscard]] auto DefaultTheme() const -> const std::string &
            {
                return m_defaultTheme.Value();
            }

            [[nodiscard]] auto HighlightTextColor() const -> uint32_t
            {
                return m_highlightTextColor.Value();
            }

        private:
            friend class AppConfig;
            Property<float>       PROPERTY_VAR(fontSize, 14.0F);
            Property<bool>        PROPERTY_VAR(useClassicTheme, false);
            Property<std::string> m_themeDirectory{"Theme", "themesDirectory"};
            Property<std::string> m_defaultTheme{"darcula", "defaultTheme"};
            Property<uint32_t>    m_highlightTextColor{0x4296FAFF, "highlightTextColor"};
            Property<std::string> PROPERTY_VAR(eastAsiaFontFile, R"(C:\Windows\Fonts\simsun.ttc)");
            Property<std::string> PROPERTY_VAR(emojiFontFile, R"(C:\Windows\Fonts\seguiemj.ttf)");
        };

        class AppConfig
        {
            // default value
        public:
            static constexpr auto DEFAULT_LOG_LEVEL                = spdlog::level::info;
            static constexpr auto DEFAULT_FLUSH_LEVEL              = spdlog::level::trace;
            static constexpr char DEFAULT_TOOL_WINDOW_SHORTCUT_KEY = 0x3C; // DIK_F2

        private:
            Property<uint32_t>                  m_toolWindowShortcutKey{DEFAULT_TOOL_WINDOW_SHORTCUT_KEY,
                                                       "toolWindowShortcutKey"}; // 0x3C
            Property<spdlog::level::level_enum> m_logLevel{DEFAULT_LOG_LEVEL, "logLevel"};
            Property<spdlog::level::level_enum> m_flushLevel{DEFAULT_FLUSH_LEVEL, "flushLevel"};
            Property<bool>                      PROPERTY_VAR(enableTsf, true);
            Property<bool>                      PROPERTY_VAR(alwaysActiveIme, false);
            Property<bool>                      PROPERTY_VAR(enableUnicodePaste, true);
            AppUiConfig                         m_appUiConfig;
            static AppConfig                    g_appConfig;

        public:
            AppConfig()                              = default;
            ~AppConfig()                             = default;
            AppConfig(const AppConfig &)             = delete;
            AppConfig &operator=(const AppConfig &)  = delete;
            AppConfig(const AppConfig &&)            = delete;
            AppConfig &operator=(const AppConfig &&) = delete;

            // Getters
            [[nodiscard]] auto GetLogLevel() const noexcept -> const spdlog::level::level_enum &
            {
                return m_logLevel.Value();
            }

            [[nodiscard]] auto GetFlushLevel() const noexcept -> const spdlog::level::level_enum &
            {
                return m_flushLevel.Value();
            }

            [[nodiscard]] auto GetToolWindowShortcutKey() const noexcept -> uint32_t
            {
                return m_toolWindowShortcutKey.Value();
            }

            [[nodiscard]] constexpr auto GetAppUiConfig() const noexcept -> const AppUiConfig &
            {
                return m_appUiConfig;
            }

            [[nodiscard]] auto EnableTsf() const -> bool
            {
                return enableTsf_.Value();
            }

            [[nodiscard]] auto AlwaysActiveIme() const -> bool
            {
                return alwaysActiveIme_.Value();
            }

            [[nodiscard]] auto EnableUnicodePaste() const -> bool
            {
                return enableUnicodePaste_.Value();
            }

            /**
             * load a ini config file from special path to AppConfig
             * @param configFilePath ini config file relative path
             */
            static void LoadIni(const char *configFilePath);

            static auto GetConfig() -> const AppConfig &;

        private:
            static void LoadIniConfig(const char *configFilePath, AppConfig &destAppConfig);
        };

    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif