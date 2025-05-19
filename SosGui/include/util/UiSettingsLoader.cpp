//
// Created by jamie on 2025/5/19.
//

#include "UiSettingsLoader.h"

#define TOML_EXCEPTIONS 1

#include "common/log.h"
#include "common/toml++/toml.hpp"
#include "utils.h"

namespace LIBC_NAMESPACE_DECL
{
void Settings::UiSettingsLoader::Load(UiSettings &uiSetting)
{
    const auto filePath = util::GetInterfaceFile("sosgui.toml");
    log_debug("Loading settings from file {}", filePath);
    try
    {
        toml::table config = toml::parse_file(filePath);

        if (!config["UiSetting"].is_table())
        {
            log_debug("Missing UiSetting config, skip.");
            return;
        }
        if (const auto indexNode = config["UiSetting"]["selectedThemeIndex"].as_integer(); indexNode)
        {
            uiSetting.selectedThemeIndex = indexNode->get();
        }
        if (const auto globalFontScale = config["UiSetting"]["globalFontScale"].as_floating_point(); globalFontScale)
        {
            uiSetting.globalFontScale = globalFontScale->get();
        }
        const auto fontInfoTable = config["FontInfo"];
        if (!fontInfoTable.is_table())
        {
            return;
        }
        uiSetting.fontInfo.filePath    = fontInfoTable["filePath"].value<std::string>().value_or("");
        uiSetting.fontInfo.familyName  = fontInfoTable["familyName"].value<std::string>().value_or("");
        uiSetting.fontInfo.familyIndex = fontInfoTable["familyIndex"].value<int32_t>().value_or(-1);
        uiSetting.fontInfo.fontIndex   = fontInfoTable["fontIndex"].value<int32_t>().value_or(-1);
        uiSetting.fontInfo.faceIndex   = fontInfoTable["faceIndex"].value<uint32_t>().value_or(0);
        uiSetting.fontInfo.bold        = fontInfoTable["bold"].value<bool>().value_or(false);
        uiSetting.fontInfo.oblique     = fontInfoTable["oblique"].value<bool>().value_or(false);
    }
    catch (toml::parse_error &e)
    {
        log_error("Can't open config file {}: {}", filePath, e.what());
    }
}

void Settings::UiSettingsLoader::Save(UiSettings &uiSetting)
{
    const auto filePath = util::GetInterfaceFile("sosgui.toml");

    const auto tomlTable = toml::table{
        {"UiSetting",
         toml::table{
             {"selectedThemeIndex", uiSetting.selectedThemeIndex},
             {"globalFontScale", uiSetting.globalFontScale},
             {"FontInfo",
              toml::table{
                  {"filePath", uiSetting.fontInfo.filePath},
                  {"familyName", uiSetting.fontInfo.familyName},
                  {"familyIndex", uiSetting.fontInfo.familyIndex},
                  {"fontIndex", uiSetting.fontInfo.fontIndex},
                  {"faceIndex", uiSetting.fontInfo.faceIndex},
                  {"bold", uiSetting.fontInfo.bold},
                  {"oblique", uiSetting.fontInfo.oblique}
              }}
         }},
    };
    std::ofstream outFile;
    outFile.open(filePath, std::ios::out);

    if (outFile.is_open())
    {
        outFile << tomlTable << std::endl;
        outFile.close();
        log_info("UiSetting already saved to {}", filePath);
    }
    else
    {
        log_error("UiSetting save fail: Unable to open file {} for writing.", filePath);
    }
}
}