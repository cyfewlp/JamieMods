#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma once

#include "AppConfig.h"
#include "common/config.h"
#include "common/log.h"
#include "configs/converter.h"
#include "ime/ImeManagerComposer.h"

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

            bool operator==(const Property &other) const
            {
                return m_value == other.m_value;
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

            [[nodiscard]] auto SectionName() const -> const std::string &
            {
                return m_sectionName;
            }

            void Load(const CSimpleIniA &ini) const
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

        template <typename Type>
        class BaseConfig
        {
            IniSection section;

        public:
            explicit BaseConfig(const char *sectionName) : section(sectionName)
            {
            }

            virtual ~BaseConfig() = default;

        protected:
            template <typename ProT>
            void Register(Property<ProT> &property) noexcept
            {
                section.Register(property);
            }

            auto SectionName() const -> const std::string &
            {
                return section.SectionName();
            }

        public:
            void Load(const CSimpleIniA &ini) const
            {
                section.Load(ini);
            }

            virtual void Save(CSimpleIniA &ini, const Type &diskConfig) const = 0;
        };

        class AppUiConfig : public BaseConfig<AppUiConfig>
        {
        public:
            AppUiConfig() : BaseConfig("UI")
            {
                Register(m_fontSize);
                Register(m_useClassicTheme);
                Register(m_translationDir);
                Register(m_themeDirectory);
                Register(m_defaultTheme);
                Register(m_highlightTextColor);
                Register(m_eastAsiaFontFile);
                Register(m_emojiFontFile);
            }

            AppUiConfig(const AppUiConfig &other)              = default;
            AppUiConfig(const AppUiConfig &&rvalue)            = delete;
            AppUiConfig &operator=(const AppUiConfig &other)   = default;
            AppUiConfig &operator=(const AppUiConfig &&rvalue) = delete;

            void Save(CSimpleIniA &ini, const AppUiConfig &diskConfig) const override;

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

            [[nodiscard]] auto TranslationDir() const -> const std::string &
            {
                return m_translationDir.Value();
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
            Property<float>       m_fontSize{14.0F, "fontSize"};
            Property<bool>        m_useClassicTheme{false, "useClassicTheme"};
            Property<std::string> m_themeDirectory{R"(Data\interface\SimpleIME)", "themesDirectory"};
            Property<std::string> m_defaultTheme{"darcula", "defaultTheme"};
            Property<uint32_t>    m_highlightTextColor{0x4296FAFF, "highlightTextColor"};
            Property<std::string> m_eastAsiaFontFile{R"(C:\Windows\Fonts\simsun.ttc)", "eastAsiaFontFile"};
            Property<std::string> m_emojiFontFile{R"(C:\Windows\Fonts\seguiemj.ttf)", "emojiFontFile"};
            Property<std::string> m_translationDir{R"(Data\interface\SimpleIME)", "translationDir"};
        };

        using WindowPosPolicy = ImeManagerComposer::ImeWindowPosUpdatePolicy;

        template <>
        struct converter<FocusType>
        {
            static constexpr auto convert(const char *value, FocusType aDefault) -> FocusType;
        };

        template <>
        struct converter<WindowPosPolicy>
        {
            static constexpr auto convert(const char *value, WindowPosPolicy aDefault) -> WindowPosPolicy;
        };

        struct SettingsConfig final : BaseConfig<SettingsConfig>
        {

            SettingsConfig();

            void Save(CSimpleIniA &ini, const SettingsConfig &diskConfig) const override;

            Property<bool>            showSettings{false, "showSettings"};
            Property<bool>            enableMod{true, "enableMod"};
            Property<std::string>     language{"chinese", "language"};
            Property<FocusType>       focusType{FocusType::Permanent, "focusType"};
            Property<WindowPosPolicy> windowPosUpdatePolicy{WindowPosPolicy::BASED_ON_CARET, "windowPosUpdatePolicy"};
            Property<bool>            enableUnicodePaste{true, "enableUnicodePaste"};
            Property<bool>            keepImeOpen{true, "keepImeOpen"};
        };

        class AppConfig : public BaseConfig<AppConfig>
        {
        public:
            static constexpr auto DEFAULT_LOG_LEVEL   = spdlog::level::info;
            static constexpr auto DEFAULT_FLUSH_LEVEL = spdlog::level::trace;
            static constexpr char ENUM_DIK_F2         = 0x3C; // DIK_F2

        private:
            Property<uint32_t>                  m_toolWindowShortcutKey{ENUM_DIK_F2, "toolWindowShortcutKey"};
            Property<spdlog::level::level_enum> m_logLevel{DEFAULT_LOG_LEVEL, "logLevel"};
            Property<spdlog::level::level_enum> m_flushLevel{DEFAULT_FLUSH_LEVEL, "flushLevel"};
            Property<bool>                      m_enableTsf{true, "enableTsf"};

            AppUiConfig      m_appUiConfig;
            SettingsConfig   m_settingsConfig;
            static AppConfig g_appConfig;

        public:
            AppConfig() : BaseConfig("General")
            {
                Register(m_toolWindowShortcutKey);
                Register(m_logLevel);
                Register(m_flushLevel);
                Register(m_enableTsf);
            }

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

            [[nodiscard]] auto GetSettingsConfig() -> SettingsConfig &
            {
                return m_settingsConfig;
            }

            [[nodiscard]] auto EnableTsf() const -> bool
            {
                return m_enableTsf.Value();
            }

            void Save(CSimpleIniA &ini, const AppConfig &diskConfig) const override;

            /**
             * load an ini config file from a special path to AppConfig
             * @param configFilePath ini config file relative path
             */
            static void LoadIni(const char *configFilePath);
            static void SaveIni(const char *configFilePath);

            static auto GetConfig() -> AppConfig &;

        private:
            static void LoadIniConfig(const char *configFilePath, AppConfig &destAppConfig);
            static void SaveIniConfig(const char *configFilePath, AppConfig &destAppConfig);
        };

    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif