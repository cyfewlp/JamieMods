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

template <typename Type>
struct Property
{
    explicit constexpr Property(Type value, const std::string_view configName)
        : m_value(std::move(value)), m_configName(configName)
    {
    }

    [[nodiscard]] constexpr const Type &Value() const
    {
        return m_value;
    }

    [[nodiscard]] constexpr const char *ConfigName() const
    {
        return m_configName.data();
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
    Type             m_value;
    std::string_view m_configName{};
};

class IniSection
{
public:
    explicit IniSection(const char *sectionName) : m_sectionName(sectionName) {}

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
    explicit BaseConfig(const char *sectionName) : section(sectionName) {}

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
        Register(m_translationDir);
        Register(m_themeDirectory);
        Register(m_highlightTextColor);
        Register(m_eastAsiaFontFile);
        Register(m_emojiFontFile);
        Register(m_errorMessageDuration);
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

    [[nodiscard]] constexpr auto FontSize() const -> int
    {
        return m_fontSize.Value();
    }

    [[nodiscard]] auto TranslationDir() const -> const std::string &
    {
        return m_translationDir.Value();
    }

    [[nodiscard]] auto ThemeDirectory() const -> const std::string &
    {
        return m_themeDirectory.Value();
    }

    [[nodiscard]] auto HighlightTextColor() const -> uint32_t
    {
        return m_highlightTextColor.Value();
    }

    [[nodiscard]] auto ErrorMessageDuration() const -> int
    {
        return m_errorMessageDuration.Value();
    }

private:
    friend class AppConfig;
    Property<int>         m_fontSize{14, "Font_Size"};
    Property<std::string> m_themeDirectory{R"(Data\interface\SimpleIME)", "Themes_Directory"};
    Property<uint32_t>    m_highlightTextColor{0x4296FAFF, "Highlight_Text_Color"};
    Property<std::string> m_eastAsiaFontFile{R"(C:\Windows\Fonts\simsun.ttc)", "East_Asia_Font_File"};
    Property<std::string> m_emojiFontFile{R"(C:\Windows\Fonts\seguiemj.ttf)", "Emoji_Font_File"};
    Property<std::string> m_translationDir{R"(Data\interface\SimpleIME)", "Translation_Dir"};
    Property<int>         m_errorMessageDuration{10, "Error_Message_Duration"};
};

template <>
struct converter<Settings::FocusType>
{
    static constexpr auto convert(const char *value, Settings::FocusType aDefault) -> Settings::FocusType;
};

template <>
struct converter<Settings::WindowPosUpdatePolicy>
{
    using Policy = Settings::WindowPosUpdatePolicy;
    static constexpr auto convert(const char *value, Policy aDefault) -> Policy;
};

struct SettingsConfig final : BaseConfig<SettingsConfig>
{
    using Policy = Settings::WindowPosUpdatePolicy;

    static constexpr float DEFAULT_FONT_SIZE_SCALE = 1.0F;
    static constexpr float MIN_FONT_SIZE_SCALE     = 0.1F;
    static constexpr float MAX_FONT_SIZE_SCALE     = 5.0F;

    SettingsConfig();

    void Save(CSimpleIniA &ini, const SettingsConfig &diskConfig) const override;

    void Set(const Settings &settings);

    [[nodiscard]] auto GetFontSizeScale() const -> float
    {
        return fontSizeScale.Value();
    }

    [[nodiscard]] auto GetShowSettings() const -> bool
    {
        return showSettings.Value();
    }

    [[nodiscard]] auto GetEnableMod() const -> bool
    {
        return enableMod.Value();
    }

    [[nodiscard]] auto GetLanguage() const -> const std::string &
    {
        return language.Value();
    }

    [[nodiscard]] auto GetFocusType() const -> Settings::FocusType
    {
        return focusType.Value();
    }

    [[nodiscard]] auto GetWindowPosUpdatePolicy() const -> Policy
    {
        return windowPosUpdatePolicy.Value();
    }

    [[nodiscard]] auto GetEnableUnicodePaste() const -> bool
    {
        return enableUnicodePaste.Value();
    }

    [[nodiscard]] auto GetKeepImeOpen() const -> bool
    {
        return keepImeOpen.Value();
    }

    [[nodiscard]] auto GetTheme() const -> const std::string &
    {
        return theme.Value();
    }

private:
    friend class AppConfig;

    Property<float>               fontSizeScale{1.0, "Font_Size_Scale"};
    Property<bool>                showSettings{false, "Show_Settings"};
    Property<bool>                enableMod{true, "Enable_Mod"};
    Property<std::string>         language{"chinese", "Language"};
    Property<Settings::FocusType> focusType{Settings::FocusType::Permanent, "Focus_Type"};
    Property<Policy>              windowPosUpdatePolicy{Policy::BASED_ON_CARET, "Window_Pos_Update_Policy"};
    Property<bool>                enableUnicodePaste{true, "Enable_Unicode_Paste"};
    Property<bool>                keepImeOpen{true, "Keep_Ime_Open"};
    Property<std::string>         theme{"darcula", "Theme"};
};

class AppConfig final : public BaseConfig<AppConfig>
{
public:
    static constexpr auto DEFAULT_LOG_LEVEL   = spdlog::level::info;
    static constexpr auto DEFAULT_FLUSH_LEVEL = spdlog::level::trace;
    static constexpr char ENUM_DIK_F2         = 0x3C; // DIK_F2

private:
    Property<uint32_t>                  m_toolWindowShortcutKey{ENUM_DIK_F2, "Tool_Window_Shortcut_Key"};
    Property<spdlog::level::level_enum> m_logLevel{DEFAULT_LOG_LEVEL, "Log_Level"};
    Property<spdlog::level::level_enum> m_flushLevel{DEFAULT_FLUSH_LEVEL, "Flush_Level"};
    Property<bool>                      m_enableTsf{true, "Enable_Tsf"};

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
    static void SaveIniConfig(const char *configFilePath, const AppConfig &destAppConfig);
};

} // namespace SimpleIME
} // namespace LIBC_NAMESPACE_DECL
#endif