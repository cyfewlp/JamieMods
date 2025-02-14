
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma once

#include "Configs.h"
#include "SimpleIni.h"
#include "configs/converter.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace SimpleIME
    {
        constexpr std::string ConvertCamelCaseToUnderscore(const std::string &input)
        {
            if (input.empty())
            {
                return {};
            }

            std::string output;
            output.reserve(input.size() * 2);

            char first = input[0];
            if (!std::isupper(static_cast<unsigned char>(first)))
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

            constexpr Property(Type value, const std::string &varName) : value_(std::move(value))
            {
                configName_ = ConvertCamelCaseToUnderscore(varName);
            }

            [[nodiscard]] constexpr const Type &Value() const
            {
                return value_;
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
            std::string configName_;
        };

        template <typename Type>
        void GetSimpleIniValue(CSimpleIniA &ini, const char *section, Property<Type> &property)
        {
            auto strV = ini.GetValue(section, property.ConfigName());
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

            //
            [[nodiscard]] constexpr auto TextColor() const -> const uint32_t &
            {
                return textColor_.Value();
            }

            [[nodiscard]] constexpr auto HighlightTextColor() const -> const uint32_t &
            {
                return highlightTextColor_.Value();
            }

            [[nodiscard]] constexpr auto WindowBorderColor() const -> const uint32_t &
            {
                return windowBorderColor_.Value();
            }

            [[nodiscard]] constexpr auto WindowBgColor() const -> const uint32_t &
            {
                return windowBgColor_.Value();
            }

            [[nodiscard]] constexpr auto BtnColor() const -> const uint32_t &
            {
                return btnColor_.Value();
            }

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

        private:
            friend class AppConfig;
            Property<float>       PROPERTY_VAR(fontSize, 14.0F);
            Property<uint32_t>    PROPERTY_VAR(textColor, 0xFFCCCCCC);
            Property<uint32_t>    PROPERTY_VAR(highlightTextColor, 0xFFCCCCCC);
            Property<uint32_t>    PROPERTY_VAR(windowBorderColor, 0xFF3A3A3A);
            Property<uint32_t>    windowBgColor_{0x801E1E1E, "windowBackgroundColor"};
            Property<uint32_t>    btnColor_{0xFF444444, "buttonColor"};
            Property<std::string> PROPERTY_VAR(eastAsiaFontFile, R"(C:\Windows\Fonts\simsun.ttc)");
            Property<std::string> PROPERTY_VAR(emojiFontFile, R"(C:\Windows\Fonts\seguiemj.ttf)");
        };

        class AppConfig
        {
            // default value
        public:
            static constexpr auto DEFAULT_LOG_LEVEL                = spdlog::level::info;
            static constexpr auto DEFAULT_FLUSH_LEVEL              = spdlog::level::trace;
            static constexpr char DEFAULT_TOOL_WINDOW_SHORTCUT_KEY = DIK_F2;

        private:
            Property<uint32_t>                  toolWindowShortcutKey_{DEFAULT_TOOL_WINDOW_SHORTCUT_KEY,
                                                      "toolWindowShortcutKey"}; // 0x3C
            Property<spdlog::level::level_enum> logLevel_{DEFAULT_LOG_LEVEL, "logLevel"};
            Property<spdlog::level::level_enum> flushLevel_{DEFAULT_FLUSH_LEVEL, "flushLevel"};

            AppUiConfig                         appUiConfig_;

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
                return logLevel_.Value();
            }

            [[nodiscard]] auto GetFlushLevel() const noexcept -> const spdlog::level::level_enum &
            {
                return flushLevel_.Value();
            }

            [[nodiscard]] auto GetToolWindowShortcutKey() const noexcept -> uint32_t
            {
                return toolWindowShortcutKey_.Value();
            }

            [[nodiscard]] constexpr AppUiConfig &GetAppUiConfig()
            {
                return appUiConfig_;
            }

            /**
             * load a ini config file from special path to AppConfig
             * @param configFilePath ini config file relative path
             */
            static void       LoadIni(const char *configFilePath);

            static AppConfig *GetConfig();

        private:
            static void                       LoadIniConfig(const char *configFilePath);

            static std::unique_ptr<AppConfig> appConfig_;
        };

    } // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif