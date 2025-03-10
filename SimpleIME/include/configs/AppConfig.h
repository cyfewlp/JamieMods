#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma once

#include "common/config.h"
#include "common/log.h"
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
            explicit constexpr Property(Type value, const std::string &varName) : m_value(std::move(value))
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

        class IniSection
        {
        public:
            explicit IniSection(const char *sectionName) : m_sectionName(sectionName)
            {
            }

            template <typename T>
            void Register(Property<T> &prop)
            {
                m_binders.emplace_back([&prop, this](const CSimpleIniA &ini) {
                    const char *val = ini.GetValue(m_sectionName.c_str(), prop.ConfigName());
                    if (val != nullptr)
                    {
                        prop.SetValue(converter<T>::convert(val, prop.Value()));
                    }
#ifdef SIMPLE_IME_DEBUG
                    else
                    {
                        throw std::runtime_error(std::format("Can't load config: {}", prop.ConfigName()));
                    }
#endif
                });
            }

            void Load(const CSimpleIniA &ini)
            {
                for (auto &binder : m_binders)
                {
                    binder(ini);
                }
            }

        private:
            std::vector<std::function<void(const CSimpleIniA &)>> m_binders;
            std::string                                           m_sectionName;
        };

        class AppUiConfig
        {
            IniSection uiSelection{"UI"};

        public:
            AppUiConfig()
            {
                uiSelection.Register(m_fontSize);
                uiSelection.Register(m_useClassicTheme);
                uiSelection.Register(m_themeDirectory);
                uiSelection.Register(m_defaultTheme);
                uiSelection.Register(m_highlightTextColor);
                uiSelection.Register(m_eastAsiaFontFile);
                uiSelection.Register(m_emojiFontFile);
            }

            AppUiConfig(const AppUiConfig &other)              = default;
            AppUiConfig(const AppUiConfig &&rvalue)            = delete;
            AppUiConfig &operator=(const AppUiConfig &other)   = default;
            AppUiConfig &operator=(const AppUiConfig &&rvalue) = delete;
            ~AppUiConfig()                                     = default;

            [[nodiscard]] constexpr auto EastAsiaFontFile() const -> const std::string &
            {
                return m_eastAsiaFontFile.Value();
            }

            [[nodiscard]] constexpr auto EmojiFontFile() const -> const std::string &
            {
                return m_emojiFontFile.Value();
            }

            [[nodiscard]] constexpr const float &FontSize() const
            {
                return m_fontSize.Value();
            }

            [[nodiscard]] auto UseClassicTheme() const -> bool
            {
                return m_useClassicTheme.Value();
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
            void Load(const CSimpleIniA &ini)
            {
                uiSelection.Load(ini);
            }

            friend class AppConfig;
            Property<float>       m_fontSize{14.0F, "fontSize"};
            Property<bool>        m_useClassicTheme{false, "useClassicTheme"};
            Property<std::string> m_themeDirectory{"Theme", "themesDirectory"};
            Property<std::string> m_defaultTheme{"darcula", "defaultTheme"};
            Property<uint32_t>    m_highlightTextColor{0x4296FAFF, "highlightTextColor"};
            Property<std::string> m_eastAsiaFontFile{R"(C:\Windows\Fonts\simsun.ttc)", "eastAsiaFontFile"};
            Property<std::string> m_emojiFontFile{R"(C:\Windows\Fonts\seguiemj.ttf)", "emojiFontFile"};
        };

        class AppConfig
        {
            // default value
            IniSection generalSelection{"General"};

        public:
            static constexpr auto DEFAULT_LOG_LEVEL   = spdlog::level::info;
            static constexpr auto DEFAULT_FLUSH_LEVEL = spdlog::level::trace;
            static constexpr char ENUM_DIK_F2         = 0x3C; // DIK_F2

        private:
            Property<uint32_t>                  m_toolWindowShortcutKey{ENUM_DIK_F2, "toolWindowShortcutKey"};
            Property<spdlog::level::level_enum> m_logLevel{DEFAULT_LOG_LEVEL, "logLevel"};
            Property<spdlog::level::level_enum> m_flushLevel{DEFAULT_FLUSH_LEVEL, "flushLevel"};
            Property<bool>                      m_enableTsf{true, "enableTsf"};
            Property<bool>                      m_enableUnicodePaste{false, "enableUnicodePaste"};
            AppUiConfig                         m_appUiConfig;
            static AppConfig                    g_appConfig;

        public:
            AppConfig()
            {
                generalSelection.Register(m_toolWindowShortcutKey);
                generalSelection.Register(m_logLevel);
                generalSelection.Register(m_flushLevel);
                generalSelection.Register(m_enableTsf);
                // generalSelection.Register(m_enableUnicodePaste);
            }

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
                return m_enableTsf.Value();
            }

            [[nodiscard]] auto EnableUnicodePaste() const -> bool
            {
                return m_enableUnicodePaste.Value();
            }

            /**
             * load a ini config file from special path to AppConfig
             * @param configFilePath ini config file relative path
             */
            static void LoadIni(const char *configFilePath);

            static auto GetConfig() -> const AppConfig &;

        private:
            static void LoadIniConfig(const char *configFilePath, AppConfig &destAppConfig);

            void Load(const CSimpleIniA &ini)
            {
                generalSelection.Load(ini);
            }
        };

    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif