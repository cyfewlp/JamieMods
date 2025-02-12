//
// Created by jamie on 2025/2/7.
//
#include "AppConfig.h"
#include <memory>
#include <yaml-cpp/yaml.h>

constexpr std::string PLUGIN_NAMESPACE::ConvertCamelCaseToUnderscore(const std::string &input)
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

    for (size_t i = 1; i < input.size(); ++i)
    {
        char c = input[i];
        if (std::isupper(static_cast<unsigned char>(c)))
        {
            output.push_back('_');
        }
        output.push_back(c);
    }

    return output;
}

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

    template <typename Type>
    constexpr Property<Type>::Property(Type value, const std::string &varName)
        : value_(std::move(value)), varName_(varName)
    {
        configName_ = ConvertCamelCaseToUnderscore(varName);
    }
}

PLUGIN_NAMESPACE::AppConfig *PLUGIN_NAMESPACE::AppConfig::Load()
{
    if (appConfig_ == nullptr)
    {
        appConfig_ = std::make_unique<AppConfig>();
        LoadConfig();
    }
    return appConfig_.get();
}

void PLUGIN_NAMESPACE::AppConfig::LoadConfig()
{
    try
    {
        YAML::Node config = YAML::LoadFile(R"(Data\SKSE\Plugins\SimpleIME.yml)");
        DoLoadConfig(config);
    }
    catch (YAML::Exception &exception)
    {
        const auto errorMsg = std::format("Load config failed: {}", exception.what());
        SKSE::stl::report_and_fail(errorMsg);
    }
}

void PLUGIN_NAMESPACE::AppConfig::DoLoadConfig(YAML::Node &config) noexcept(false)
{
    GetValue(config, appConfig_->logLevel_);
    GetValue(config, appConfig_->flushLevel_);
    GetValue(config, appConfig_->toolWindowShortcutKey);

    AppUiConfig appUiConfig  = {};
    appConfig_->appUiConfig_ = appUiConfig;
    const auto &uiNode       = config["UI"];
    if (!uiNode)
    {
        return;
    }
    GetValue(uiNode, appUiConfig.eastAsiaFontFile_);
    GetValue(uiNode, appUiConfig.emojiFontFile_);
    GetValue(uiNode, appUiConfig.textColor_);
    GetValue(uiNode, appUiConfig.highlightTextColor_);
    GetValue(uiNode, appUiConfig.windowBorderColor_);
    GetValue(uiNode, appUiConfig.windowBgColor_);
    GetValue(uiNode, appUiConfig.btnColor_);
    GetValue(uiNode, appUiConfig.fontSize_);
}

uint32_t LIBC_NAMESPACE::SimpleIME::AppConfig::HexStringToUInt32(const std::string &hexStr, uint32_t aDefault)
{
    try
    {
        size_t start = (hexStr.find("0x") == 0 || hexStr.find("0X") == 0) ? 2 : 0;
        return static_cast<uint32_t>(std::stoul(hexStr.substr(start), nullptr, 16));
    }
    catch (const std::exception &e)
    {
        log_error("Error: {},not a hex string: {}", e.what(), hexStr.c_str());
    }
    return aDefault;
}

auto LIBC_NAMESPACE::SimpleIME::AppConfig::parseLogLevel(int32_t intLevel) -> spdlog::level::level_enum
{
    if (intLevel < 0 || intLevel >= static_cast<int>(spdlog::level::n_levels))
    {
        log_error("Invalid spdlog level {}, downgrade to default: info", intLevel);
        return spdlog::level::info;
    }
    return static_cast<spdlog::level::level_enum>(intLevel);
}