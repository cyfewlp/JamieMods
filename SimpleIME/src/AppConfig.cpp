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

        void AppUiConfig::Save(CSimpleIniA &ini, const AppUiConfig &diskConfig) const
        {
        }

        SettingsConfig::SettingsConfig() : BaseConfig("Settings")
        {
            Register(showSettings);
            Register(enableMod);
            Register(language);
            Register(focusType);
            Register(windowPosUpdatePolicy);
            Register(enableUnicodePaste);
            Register(keepImeOpen);
        }

        template <typename Type>
        void IniSetValue(CSimpleIniA &ini, const std::string &sectionStr, const Property<Type> &property)
        {
            const char *section = sectionStr.c_str();
            if constexpr (std::is_same_v<Type, bool>)
            {
                ini.SetBoolValue(section, property.ConfigName(), property.Value());
            }
            else if constexpr (std::is_same_v<Type, std::string>)
            {
                ini.SetValue(section, property.ConfigName(), property.Value().c_str());
            }
            else if constexpr (std::is_same_v<Type, FocusType>)
            {
                switch (property.Value())
                {
                    case FocusType::Permanent:
                        ini.SetValue(section, property.ConfigName(), "Permanent");
                        break;
                    case FocusType::Temporary:
                        ini.SetValue(section, property.ConfigName(), "Temporary");
                        break;
                    default:
                        break;
                }
            }
            else if constexpr (std::is_same_v<Type, WindowPosPolicy>)
            {
                switch (property.Value())
                {
                    case WindowPosPolicy::NONE:
                        ini.SetValue(section, property.ConfigName(), "None");
                        break;
                    case WindowPosPolicy::BASED_ON_CARET:
                        ini.SetValue(section, property.ConfigName(), "BASED_ON_CARET");
                        break;
                    case WindowPosPolicy::BASED_ON_CURSOR:
                        ini.SetValue(section, property.ConfigName(), "BASED_ON_CURSOR");
                        break;
                    default:
                        break;
                }
            }
        }

        template <typename Type>
        void IniSetValueIfDiff(CSimpleIniA &ini, const std::string &section, const Property<Type> &property,
                               const Property<Type> &diskProperty)
        {
            if (property == diskProperty)
            {
                return;
            }
            IniSetValue(ini, section, property);
        }

        void SettingsConfig::Save(CSimpleIniA &ini, const SettingsConfig &diskConfig) const
        {
            IniSetValueIfDiff(ini, SectionName(), showSettings, diskConfig.showSettings);
            IniSetValueIfDiff(ini, SectionName(), enableMod, diskConfig.enableMod);
            IniSetValueIfDiff(ini, SectionName(), language, diskConfig.language);
            IniSetValueIfDiff(ini, SectionName(), focusType, diskConfig.focusType);
            IniSetValueIfDiff(ini, SectionName(), windowPosUpdatePolicy, diskConfig.windowPosUpdatePolicy);
            IniSetValueIfDiff(ini, SectionName(), enableUnicodePaste, diskConfig.enableUnicodePaste);
            IniSetValueIfDiff(ini, SectionName(), keepImeOpen, diskConfig.keepImeOpen);
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
        }

        void AppConfig::SaveIniConfig(const char *configFilePath, AppConfig &destAppConfig)
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

        constexpr auto converter<FocusType>::convert(const char *value, FocusType aDefault) -> FocusType
        {
            if (value == nullptr) return aDefault;
            std::string strValue{value};
            std::ranges::transform(strValue, strValue.begin(), ::tolower);
            if (strValue == "permanent") return FocusType::Permanent;
            if (strValue == "temporary") return FocusType::Temporary;
            return aDefault;
        }

        constexpr auto converter<WindowPosPolicy>::convert(const char *value, WindowPosPolicy aDefault)
            -> WindowPosPolicy
        {
            if (value == nullptr) return aDefault;
            std::string strValue{value};
            std::ranges::transform(strValue, strValue.begin(), ::tolower);
            if (strValue == "none") return WindowPosPolicy::NONE;
            if (strValue == "based_on_caret") return WindowPosPolicy::BASED_ON_CARET;
            if (strValue == "based_on_cursor") return WindowPosPolicy::BASED_ON_CURSOR;
            return aDefault;
        }
    }
}
