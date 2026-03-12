//
// Created by jamie on 2026/1/14.
//
#include "configs/ConfigSerializer.h"

#include "configs/configuration.h"
#include "log.h"
#include "toml/toml.hpp"
#include "ui/Settings.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <ios>
#include <iosfwd>
#include <spdlog/common.h>
#include <string>
#include <vector>

namespace Ime::ConfigSerializer
{
namespace
{

static constexpr auto KEY_SHORTCUT = "shortcut";

// Section keys
static constexpr auto KEY_SECTION_CORE       = "core";
static constexpr auto KEY_SECTION_RESOURCES  = "resources";
static constexpr auto KEY_SECTION_APPEARANCE = "appearance";
static constexpr auto KEY_SECTION_INPUT      = "input";
static constexpr auto KEY_SECTION_LOGGING    = "logging";

// Core keys
static constexpr auto KEY_ENABLE_TSF = "enable_tsf";
static constexpr auto KEY_ENABLE_MOD = "enable_mod";

// Logging keys
static constexpr auto KEY_LOG_LEVEL       = "level";
static constexpr auto KEY_LOG_FLUSH_LEVEL = "flush_level";

// Resources keys
static constexpr auto KEY_TRANSLATION_DIR = "translation_dir";
static constexpr auto KEY_FONTS           = "fonts";

// Appearance keys
static constexpr auto KEY_ZOOM                   = "zoom";
static constexpr auto KEY_LANGUAGE               = "language";
static constexpr auto KEY_THEME_SOURCE_COLOR     = "theme_source_color";
static constexpr auto KEY_THEME_DARK_MODE        = "theme_dark_mode";
static constexpr auto KEY_THEME_CONTRAST_LEVEL   = "theme_contrast_level";
static constexpr auto KEY_ERROR_DISPLAY_DURATION = "error_display_duration";
static constexpr auto KEY_SHOW_SETTINGS          = "show_settings";

// Input keys
static constexpr auto KEY_ENABLE_UNICODE_PASTE = "enable_unicode_paste";
static constexpr auto KEY_KEEP_IME_OPEN        = "keep_ime_open";
static constexpr auto KEY_POS_UPDATE_POLICY    = "pos_update_policy";

//! @brief Format the configuration to a TOML string with comments for better readability.
//! May throw `toml::exception` if the configuration contains unsupported types or values.
auto FormatConfigurationToToml(const Configuration &configuration) -> std::string
{
    using Comments = std::vector<std::string>;

    const Comments shortcutComment = {
        " 快捷键配置，格式: <key> 或 <modifier(s)> + <key>，多个部分用 '+' 分隔，忽略空格",
        " <key>      : F1-F12, NumPad0-9, A-Z, 0-9 等 ImGui 支持的按键名",
        " <modifier> : ctrl, shift, alt, super (可组合)",
        R"( 示例: "F2", "ctrl + A", "ctrl + shift + alt + A")",
        " 注意: 不支持多个普通键组合 (如 A + B)，末尾不能为 '+', F1-F12 不能和任何 <modifier> 组合(游戏限制), 非法配置将使用默认值",
        "默认值为 F2"
    };
    const Comments enableTsfComment = {
        " 强烈建议开启，只需要保证您的系统版本大于 Window 8.1(除非您手动将 TSF 关闭了，或者您是某些极简系统). 关闭将使用 Imm32, 支持不如 TSF"
    };
    const Comments enableModComment = {" 是否启用 Mod 功能"};
    const Comments logLevelComment  = {R"( "trace", "debug", "info", "warn", "error", "critical", "off")"};
    const Comments zoomComment      = {" UI缩放倍率，有效值为 0.5 - 2.0, 请使用 0.25 的整数倍", " 可以指定为负数，表示应用当前显示器的缩放倍率"};

    toml::table logging{
        {KEY_LOG_LEVEL,       {configuration.logging.level, logLevelComment}},
        {KEY_LOG_FLUSH_LEVEL, configuration.logging.flushLevel              },
    };

    toml::table core{
        {{KEY_SHORTCUT, {configuration.shortcut, shortcutComment}},
         {KEY_ENABLE_TSF, {configuration.enableTsf, enableTsfComment}},
         {KEY_ENABLE_MOD, {configuration.enableMod, enableModComment}},
         {KEY_SECTION_LOGGING, logging}}
    };
    toml::table resources{
        {KEY_TRANSLATION_DIR, {configuration.resources.translationDir, {" 翻译文件目录"}}},
        {KEY_FONTS,           configuration.resources.fontPathList                       },
    };
    toml::table appearance{
        {KEY_ZOOM,                   {configuration.appearance.zoom, zoomComment}                                                    },
        {KEY_LANGUAGE,               configuration.appearance.language                                                               },
        {KEY_THEME_SOURCE_COLOR,     configuration.appearance.themeSourceColor                                                       },
        {KEY_THEME_DARK_MODE,        configuration.appearance.themeDarkMode                                                          },
        {KEY_THEME_CONTRAST_LEVEL,   configuration.appearance.themeContrastLevel                                                     },
        {KEY_ERROR_DISPLAY_DURATION, {configuration.appearance.errorDisplayDuration, {" 错误信息显示持续时间 (秒)，-1 为不自动关闭"}}},
        {KEY_SHOW_SETTINGS,          configuration.appearance.showSettings                                                           },
    };
    toml::table input{
        {KEY_ENABLE_UNICODE_PASTE, configuration.input.enableUnicodePaste},
        {KEY_KEEP_IME_OPEN,        configuration.input.keepImeOpen       },
        {KEY_POS_UPDATE_POLICY,    configuration.input.posUpdatePolicy   },
    };
    toml::value tomlTable = {
        toml::table{
                    {KEY_SECTION_CORE, core},
                    {KEY_SECTION_RESOURCES, resources},
                    {KEY_SECTION_APPEARANCE, appearance},
                    {KEY_SECTION_INPUT, input},
                    }
    };

    return toml::format(tomlTable);
}

auto ParseConfigurationFromToml(toml::value &rawToml) -> Configuration
{
    auto findAndSet = [](auto &tomlTable, const char *key, auto &value) {
        value = toml::find_or(tomlTable, key, value);
    };
    Configuration config{};
    if (rawToml.contains(KEY_SECTION_CORE))
    {
        auto &coreToml = rawToml[KEY_SECTION_CORE];
        findAndSet(coreToml, KEY_SHORTCUT, config.shortcut);
        findAndSet(coreToml, KEY_ENABLE_TSF, config.enableTsf);
        findAndSet(coreToml, KEY_ENABLE_MOD, config.enableMod);
        if (coreToml.contains(KEY_SECTION_LOGGING))
        {
            auto &loggingToml         = coreToml[KEY_SECTION_LOGGING];
            config.logging.level      = toml::find_or(loggingToml, KEY_LOG_LEVEL, config.logging.level);
            config.logging.flushLevel = toml::find_or(loggingToml, KEY_LOG_FLUSH_LEVEL, config.logging.flushLevel);
        }
    }

    if (rawToml.contains(KEY_SECTION_RESOURCES))
    {
        auto &resourcesToml = rawToml[KEY_SECTION_RESOURCES];
        findAndSet(resourcesToml, KEY_TRANSLATION_DIR, config.resources.translationDir);
        findAndSet(resourcesToml, KEY_FONTS, config.resources.fontPathList);
    }

    if (rawToml.contains(KEY_SECTION_APPEARANCE))
    {
        auto &appearanceToml = rawToml[KEY_SECTION_APPEARANCE];
        findAndSet(appearanceToml, KEY_ZOOM, config.appearance.zoom);
        findAndSet(appearanceToml, KEY_LANGUAGE, config.appearance.language);
        findAndSet(appearanceToml, KEY_THEME_SOURCE_COLOR, config.appearance.themeSourceColor);
        findAndSet(appearanceToml, KEY_THEME_DARK_MODE, config.appearance.themeDarkMode);
        findAndSet(appearanceToml, KEY_THEME_CONTRAST_LEVEL, config.appearance.themeContrastLevel);
        findAndSet(appearanceToml, KEY_ERROR_DISPLAY_DURATION, config.appearance.errorDisplayDuration);
        findAndSet(appearanceToml, KEY_SHOW_SETTINGS, config.appearance.showSettings);
    }

    if (rawToml.contains(KEY_SECTION_INPUT))
    {
        auto &input = rawToml[KEY_SECTION_INPUT];
        findAndSet(input, KEY_ENABLE_UNICODE_PASTE, config.input.enableUnicodePaste);
        findAndSet(input, KEY_KEEP_IME_OPEN, config.input.keepImeOpen);
        findAndSet(input, KEY_POS_UPDATE_POLICY, config.input.posUpdatePolicy);
    }
    return config;
}
} // namespace

void SaveConfiguration(const std::filesystem::path &filePath, const Configuration &configuration)
{
    try
    {
        const auto tomlString = FormatConfigurationToToml(configuration);

        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);
        file.open(filePath, std::ios::out | std::ios::trunc);
        file << tomlString;
        file.close();
        logger::info("Configuration saved successfully to {}", filePath.generic_string());
    }
    catch (const std::ios_base::failure &e)
    {
        logger::error("File IO error while saving configuration: {}", e.what());
    }
    catch (toml::exception &exception)
    {
        logger::error("Parse configuration failed! Can't save configuration: {}", exception.what());
    }
    catch (const std::exception &e)
    {
        logger::error("Unknown error during serialization: {}", e.what());
    }
}

auto LoadConfiguration(const std::filesystem::path &filePath) -> Configuration
{
    try
    {
        auto tomlValue = toml::parse(filePath);
        return ParseConfigurationFromToml(tomlValue);
    }
    catch (toml::exception &exception)
    {
        logger::error("Parse configuration failed! Fallback to default configuration: {}", exception.what());
    }
    return {};
}

} // namespace Ime::ConfigSerializer
