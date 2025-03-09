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

        auto AppConfig::GetConfig() -> const AppConfig &
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
            GetSimpleIniValue(ini, "General", destAppConfig.m_logLevel);
            GetSimpleIniValue(ini, "General", destAppConfig.m_flushLevel);
            GetSimpleIniValue(ini, "General", destAppConfig.m_toolWindowShortcutKey);
            GetSimpleIniValue(ini, "General", destAppConfig.enableTsf_);
            GetSimpleIniValue(ini, "General", destAppConfig.alwaysActiveIme_);
            //GetSimpleIniValue(ini, "General", destAppConfig.enableUnicodePaste_);

            // load ui configs
            auto &appUiConfig = destAppConfig.m_appUiConfig;
            GetSimpleIniValue(ini, "UI", appUiConfig.useClassicTheme_);
            GetSimpleIniValue(ini, "UI", appUiConfig.eastAsiaFontFile_);
            GetSimpleIniValue(ini, "UI", appUiConfig.emojiFontFile_);
            GetSimpleIniValue(ini, "UI", appUiConfig.fontSize_);
            GetSimpleIniValue(ini, "UI", appUiConfig.m_themeDirectory);
            GetSimpleIniValue(ini, "UI", appUiConfig.m_defaultTheme);
            GetSimpleIniValue(ini, "UI", appUiConfig.m_highlightTextColor);
        }
    }
}
