//
// Created by jamie on 2025/2/7.
//
#include "configs/AppConfig.h"

#include "common/log.h"

#include <SimpleIni.h>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
AppConfig AppConfig::g_appConfig{};

SettingsConfig::SettingsConfig() : BaseConfig("Settings")
{
    Register(fontSizeScale);
    Register(showSettings);
    Register(enableMod);
    Register(language);
    Register(focusType);
    Register(windowPosUpdatePolicy);
    Register(enableUnicodePaste);
    Register(keepImeOpen);
    Register(theme);
}

template <typename Type>
void IniSetValue(CSimpleIniA &ini, const std::string &sectionStr, const Property<Type> &property)
{
    const char *section = sectionStr.c_str();
    if constexpr (std::is_same_v<Type, bool>)
    {
        ini.SetBoolValue(section, property.ConfigName(), property.Value());
    }
    else if constexpr (std::is_same_v<Type, float>)
    {
        ini.SetDoubleValue(section, property.ConfigName(), property.Value());
    }
    else if constexpr (std::is_same_v<Type, std::string>)
    {
        ini.SetValue(section, property.ConfigName(), property.Value().c_str());
    }
    else if constexpr (std::is_same_v<Type, Settings::FocusType>)
    {
        switch (property.Value())
        {
            case Settings::FocusType::Permanent:
                ini.SetValue(section, property.ConfigName(), "Permanent");
                break;
            case Settings::FocusType::Temporary:
                ini.SetValue(section, property.ConfigName(), "Temporary");
                break;
            default:
                break;
        }
    }
    else if constexpr (std::is_same_v<Type, Settings::WindowPosUpdatePolicy>)
    {
        switch (property.Value())
        {
            case Settings::WindowPosUpdatePolicy::NONE:
                ini.SetValue(section, property.ConfigName(), "None");
                break;
            case Settings::WindowPosUpdatePolicy::BASED_ON_CARET:
                ini.SetValue(section, property.ConfigName(), "BASED_ON_CARET");
                break;
            case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
                ini.SetValue(section, property.ConfigName(), "BASED_ON_CURSOR");
                break;
            default:
                break;
        }
    }
    else
    {
        assert(false && "Unknown Property type");
    }
}

template <typename Type>
void IniSetValueIfDiff(
    CSimpleIniA &ini, const std::string &section, const Property<Type> &property, const Property<Type> &diskProperty
)
{
    if (property == diskProperty)
    {
        return;
    }
    IniSetValue(ini, section, property);
}

void AppUiConfig::Set(const Settings &settings)
{
    m_fontSize.SetValue(settings.fontSize);
}

void AppUiConfig::Save(CSimpleIniA &ini, const AppUiConfig &diskConfig) const
{
    IniSetValueIfDiff(ini, SectionName(), m_fontSize, diskConfig.m_fontSize);
}

void SettingsConfig::Save(CSimpleIniA &ini, const SettingsConfig &diskConfig) const
{
    IniSetValueIfDiff(ini, SectionName(), fontSizeScale, diskConfig.fontSizeScale);
    IniSetValueIfDiff(ini, SectionName(), showSettings, diskConfig.showSettings);
    IniSetValueIfDiff(ini, SectionName(), enableMod, diskConfig.enableMod);
    IniSetValueIfDiff(ini, SectionName(), language, diskConfig.language);
    IniSetValueIfDiff(ini, SectionName(), focusType, diskConfig.focusType);
    IniSetValueIfDiff(ini, SectionName(), windowPosUpdatePolicy, diskConfig.windowPosUpdatePolicy);
    IniSetValueIfDiff(ini, SectionName(), enableUnicodePaste, diskConfig.enableUnicodePaste);
    IniSetValueIfDiff(ini, SectionName(), keepImeOpen, diskConfig.keepImeOpen);
    IniSetValueIfDiff(ini, SectionName(), theme, diskConfig.theme);
}

void SettingsConfig::Set(const Settings &settings)
{
    enableUnicodePaste.SetValue(settings.enableUnicodePaste);
    keepImeOpen.SetValue(settings.keepImeOpen);
    theme.SetValue(settings.theme);
    fontSizeScale.SetValue(settings.fontSizeScale);
    showSettings.SetValue(settings.showSettings);
    enableMod.SetValue(settings.enableMod);
    windowPosUpdatePolicy.SetValue(settings.windowPosUpdatePolicy);
    focusType.SetValue(settings.focusType);
    language.SetValue(settings.language);
}

void AppConfig::Set(const Settings &settings)
{
    m_appUiConfig.Set(settings);
    m_settingsConfig.Set(settings);
}

void AppConfig::Save(CSimpleIniA &ini, const AppConfig &diskConfig) const
{
    m_appUiConfig.Save(ini, diskConfig.m_appUiConfig);
    m_settingsConfig.Save(ini, diskConfig.m_settingsConfig);
}

void AppConfig::LoadIni(const char *configFilePath)
{
    if (configFilePath == nullptr)
    {
        log_error("LoadIni: parameter configFilePath is nullptr");
    }
    else
    {
        LoadIniConfig(configFilePath, g_appConfig);
    }
}

void AppConfig::SaveIni(const char *configFilePath)
{
    if (configFilePath == nullptr)
    {
        log_error("LoadIni: parameter configFilePath is nullptr");
    }
    else
    {
        SaveIniConfig(configFilePath, g_appConfig);
    }
}

auto AppConfig::GetConfig() -> AppConfig &
{
    return g_appConfig;
}

void AppConfig::LoadIniConfig(const char *configFilePath, AppConfig &destAppConfig)
{
    CSimpleIniA ini(false, false, true);
    if (const SI_Error error = ini.LoadFile(configFilePath); error != SI_OK)
    {
        log_error("Load config file failed. May config file {} missing", configFilePath);
        return;
    }
    destAppConfig.Load(ini);
    destAppConfig.m_appUiConfig.Load(ini);
    destAppConfig.m_settingsConfig.Load(ini);

    // validate
    float fontSizeScale = destAppConfig.m_settingsConfig.fontSizeScale.Value();
    if (fontSizeScale < SettingsConfig::MIN_FONT_SIZE_SCALE || fontSizeScale > SettingsConfig::MAX_FONT_SIZE_SCALE)
    {
        destAppConfig.m_settingsConfig.fontSizeScale.SetValue(SettingsConfig::DEFAULT_FONT_SIZE_SCALE);
    }
}

void AppConfig::SaveIniConfig(const char *configFilePath, const AppConfig &destAppConfig)
{
    CSimpleIniA ini(false, false, true);
    if (const SI_Error error = ini.LoadFile(configFilePath); error != SI_OK)
    {
        log_error("Load config file failed. May config file {} missing", configFilePath);
        return;
    }
    AppConfig diskConfig;
    diskConfig.Load(ini);
    diskConfig.m_appUiConfig.Load(ini);
    diskConfig.m_settingsConfig.Load(ini);

    destAppConfig.Save(ini, diskConfig);
    if (auto error = ini.SaveFile(configFilePath); error != SI_OK)
    {
        log_error("Can't save config.");
    }
}

constexpr auto ConvertCamelCaseToUnderscore(const std::string_view &input) -> std::string
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

constexpr auto converter<Settings::FocusType>::convert(const char *value, Settings::FocusType aDefault)
    -> Settings::FocusType
{
    if (value == nullptr) return aDefault;
    std::string strValue{value};
    std::ranges::transform(strValue, strValue.begin(), ::tolower);
    if (strValue == "permanent") return Settings::FocusType::Permanent;
    if (strValue == "temporary") return Settings::FocusType::Temporary;
    return aDefault;
}

constexpr auto converter<Settings::WindowPosUpdatePolicy>::convert(const char *value, const Policy aDefault) -> Policy
{
    if (value == nullptr) return aDefault;
    std::string strValue{value};
    std::ranges::transform(strValue, strValue.begin(), ::tolower);
    if (strValue == "none") return Policy::NONE;
    if (strValue == "based_on_caret") return Policy::BASED_ON_CARET;
    if (strValue == "based_on_cursor") return Policy::BASED_ON_CURSOR;
    return aDefault;
}
}
}
