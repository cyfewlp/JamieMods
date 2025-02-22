//
// Created by jamie on 2025/2/7.
//
#include "configs/AppConfig.h"
#include "common/log.h"
#include <SimpleIni.h>
#include <memory>

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        std::unique_ptr<AppConfig> AppConfig::m_appConfig = nullptr;

        void                       AppConfig::LoadIni(const char *configFilePath)
        {
            if (m_appConfig == nullptr)
            {
                m_appConfig = std::make_unique<AppConfig>();
                LoadIniConfig(configFilePath);
            }
        }

        AppConfig *AppConfig::GetConfig()
        {
            return m_appConfig.get();
        }

        void AppConfig::LoadIniConfig(const char *configFilePath)
        {
            CSimpleIniA ini;
            if (const SI_Error error = ini.LoadFile(configFilePath); error != SI_OK)
            {
                log_error("Load config file failed. May config file {} missing", configFilePath);
                return;
            }
            GetSimpleIniValue(ini, "General", m_appConfig->m_logLevel);
            GetSimpleIniValue(ini, "General", m_appConfig->m_flushLevel);
            GetSimpleIniValue(ini, "General", m_appConfig->m_toolWindowShortcutKey);
            GetSimpleIniValue(ini, "General", m_appConfig->enableTsf_);

            // load ui configs
            AppUiConfig appUiConfig = {};
            GetSimpleIniValue(ini, "UI", appUiConfig.textColor_);
            GetSimpleIniValue(ini, "UI", appUiConfig.highlightTextColor_);
            GetSimpleIniValue(ini, "UI", appUiConfig.m_windowBgColor);
            GetSimpleIniValue(ini, "UI", appUiConfig.windowBorderColor_);
            GetSimpleIniValue(ini, "UI", appUiConfig.m_btnColor);
            GetSimpleIniValue(ini, "UI", appUiConfig.eastAsiaFontFile_);
            GetSimpleIniValue(ini, "UI", appUiConfig.emojiFontFile_);
            GetSimpleIniValue(ini, "UI", appUiConfig.fontSize_);
            m_appConfig->m_appUiConfig = appUiConfig;
        }
    }
}
