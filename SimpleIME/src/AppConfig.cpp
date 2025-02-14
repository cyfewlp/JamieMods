//
// Created by jamie on 2025/2/7.
//
#include "configs/AppConfig.h"
#include <SimpleIni.h>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace PLUGIN_NAMESPACE
{
    std::unique_ptr<AppConfig> AppConfig::appConfig_ = nullptr;

    template <typename Type>
    static constexpr void GetValue(const YAML::Node &node, Property<Type> &property)
    {
        if (node[property.ConfigName()])
        {
            property.SetValue(node[property.ConfigName()].template as<Type>());
        }
    }
}

void PLUGIN_NAMESPACE::AppConfig::LoadIni(const char *configFilePath)
{
    if (appConfig_ == nullptr)
    {
        appConfig_ = std::make_unique<AppConfig>();
        LoadIniConfig(configFilePath);
    }
}

PLUGIN_NAMESPACE::AppConfig *PLUGIN_NAMESPACE::AppConfig::GetConfig()
{
    return appConfig_.get();
}

void PLUGIN_NAMESPACE::AppConfig::LoadIniConfig(const char *configFilePath)
{
    CSimpleIniA ini;
    if (const SI_Error error = ini.LoadFile(configFilePath); error != SI_OK)
    {
        log_error("Load config file failed. May config file {} missing", configFilePath);
        return;
    }
    GetSimpleIniValue(ini, "General", appConfig_->logLevel_);
    GetSimpleIniValue(ini, "General", appConfig_->flushLevel_);
    GetSimpleIniValue(ini, "General", appConfig_->toolWindowShortcutKey_);

    // load ui configs
    AppUiConfig appUiConfig = {};
    GetSimpleIniValue(ini, "UI", appUiConfig.textColor_);
    GetSimpleIniValue(ini, "UI", appUiConfig.highlightTextColor_);
    GetSimpleIniValue(ini, "UI", appUiConfig.windowBgColor_);
    GetSimpleIniValue(ini, "UI", appUiConfig.windowBorderColor_);
    GetSimpleIniValue(ini, "UI", appUiConfig.btnColor_);
    GetSimpleIniValue(ini, "UI", appUiConfig.eastAsiaFontFile_);
    GetSimpleIniValue(ini, "UI", appUiConfig.emojiFontFile_);
    GetSimpleIniValue(ini, "UI", appUiConfig.fontSize_);
    appConfig_->appUiConfig_ = appUiConfig;
}
