//
// Created by jamie on 2026/1/14.
//
#include "configs/ConfigSerializer.h"

#include "common/log.h"
#include "common/toml/toml.hpp"
#include "ui/Settings.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <ios>
#include <iosfwd>
#include <spdlog/common.h>
#include <string>
#include <vector>

namespace toml
{
template <>
struct from<Ime::Settings::WindowPosUpdatePolicy>
{
    using Policy = Ime::Settings::WindowPosUpdatePolicy;

    static Policy from_toml(const toml::value &v)
    {
        auto str = toml::get_or(v, "NONE");
        std::ranges::transform(str, str.begin(), ::tolower);
        if (str == "none")
        {
            return Policy::NONE;
        }
        if (str == "based_on_caret")
        {
            return Policy::BASED_ON_CARET;
        }
        if (str == "based_on_cursor")
        {
            return Policy::BASED_ON_CURSOR;
        }
        return Policy::NONE;
    }
};

template <>
struct from<spdlog::level::level_enum>
{
    static spdlog::level::level_enum from_toml(const toml::value &v)
    {
        auto str = toml::get_or(v, "info");
        std::ranges::transform(str, str.begin(), ::tolower);
        if (str == "trace") return spdlog::level::trace;
        if (str == "debug") return spdlog::level::debug;
        if (str == "info") return spdlog::level::info;
        if (str == "warn") return spdlog::level::warn;
        if (str == "err") return spdlog::level::err;
        if (str == "critical") return spdlog::level::critical;
        if (str == "off") return spdlog::level::off;
        return spdlog::level::info;
    }
};

} // toml

namespace Ime::ConfigSerializer
{
namespace
{
auto toString(const Settings::WindowPosUpdatePolicy policy) -> std::string
{
    switch (policy)
    {
        case Settings::WindowPosUpdatePolicy::BASED_ON_CARET:
            return "BASED_ON_CARET";
        case Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR:
            return "BASED_ON_CURSOR";
        default:
            return "NONE";
    }
}

auto toString(const spdlog::level::level_enum &level) -> std::string
{
    switch (level)
    {
        case spdlog::level::trace:
            return "trace";
        case spdlog::level::debug:
            return "debug";
        case spdlog::level::info:
            return "info";
        case spdlog::level::warn:
            return "warn";
        case spdlog::level::err:
            return "err";
        case spdlog::level::critical:
            return "critical";
        case spdlog::level::off:
            return "off";
        default:
            return "info";
    }
}

toml::table toTomlTable(Settings &settings)
{
    using Comments = std::vector<std::string>;

    static Comments const shortcutKeyComment = {" 快捷键配置 (使用标准的 Hex 格式)", " DIK_F1 = 0x3B, DIK_F2 = 0x3C..."};
    static Comments enableTsfComment = {" 现代输入法建议开启，旧版输入法可能不兼容"};
    static Comments enableModComment = {" 是否启用 Mod 功能"};
    static Comments logLevelComment = {R"( "trace", "debug", "info", "warn", "error", "critical", "off")"};

    toml::value shortKey{settings.shortcutKey, shortcutKeyComment};
    shortKey.as_integer_fmt().uppercase = true;
    shortKey.as_integer_fmt().fmt = toml::integer_format::hex;

    toml::table logging{
        {"level", {toString(settings.logging.level), logLevelComment}},
        {"flush_level", toString(settings.logging.flushLevel)},
    };
    return {
        {"shortcut_key", shortKey},
        {"enable_tsf", {settings.enableTsf, enableTsfComment}},
        {"enable_mod", {settings.enableMod, enableModComment}},
        {"logging", logging}
    };
}

toml::table toTomlTable(Settings::Appearance &appearance)
{
    return {
        {"zoom", {appearance.zoom, {" UI缩放倍率(0.5 - 2.0), 请使用 0.25的整数倍"}}},
        {"language", appearance.language},
        {"theme_source_color", appearance.themeSourceColor},
        {"theme_dark_mode", appearance.themeDarkMode},
        {"error_display_duration", {appearance.errorDisplayDuration, {" 错误信息显示持续时间 (秒)，-1 为不自动关闭"}}},
        {"show_settings", appearance.showSettings},
    };
}

toml::table toTomlTable(Settings::Resources &resources)
{
    return {
        {"translation_dir", {resources.translationDir, {" 翻译文件目录"}}},
        {"fonts", resources.fontPathList},
    };
}

toml::table toTomlTable(Settings::Input &input)
{
    return {
        {"enable_unicode_paste", input.enableUnicodePaste},
        {"keep_ime_open", input.keepImeOpen},
        {"pos_update_policy", toString(input.posUpdatePolicy)},
    };
}
}

void DoDeserialize(toml::value &config, Settings &settings);

auto DoSerialize(Settings &settings) -> toml::value;

void Deserialize(const std::filesystem::path &filePath, Settings &settings)
{
    try
    {
        auto config = toml::parse(filePath);
        DoDeserialize(config, settings);
    }
    catch (toml::exception &exception)
    {
        logger::error("Parse configuration failed! Fallback to default configuration: {}", exception.what());
    }
}

void Serialize(const std::filesystem::path &filePath, Settings &settings)
{
    try
    {
        const auto settingsValue = DoSerialize(settings);

        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);
        file.open(filePath, std::ios::out | std::ios::trunc);
        file << toml::format(settingsValue);
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

void DoDeserialize(toml::value &config, Settings &settings)
{
    auto findAndSet = [](auto &tomlTable, const char *key, auto &value) {
        value = toml::find_or(tomlTable, key, value);
    };
    if (config.contains("core"))
    {
        auto &core = config["core"];
        findAndSet(core, "shortcut_key", settings.shortcutKey);
        findAndSet(core, "enable_tsf", settings.enableTsf);
        findAndSet(core, "enable_mod", settings.enableMod);
        settings.logging.level = toml::find_or(core, "logging", "level", settings.logging.level);
        settings.logging.flushLevel = toml::find_or(core, "logging", "flush_level", settings.logging.flushLevel);
    }

    if (config.contains("resources"))
    {
        auto &resources = config["resources"];
        findAndSet(resources, "translation_dir", settings.resources.translationDir);
        findAndSet(resources, "fonts", settings.resources.fontPathList);
    }

    if (config.contains("appearance"))
    {
        auto &appearance = config["appearance"];
        findAndSet(appearance, "zoom", settings.appearance.zoom);
        findAndSet(appearance, "language", settings.appearance.language);
        findAndSet(appearance, "theme_source_color", settings.appearance.themeSourceColor);
        findAndSet(appearance, "theme_dark_mode", settings.appearance.themeDarkMode);
        findAndSet(appearance, "error_display_duration", settings.appearance.errorDisplayDuration);
        findAndSet(appearance, "show_settings", settings.appearance.showSettings);
    }

    if (config.contains("input"))
    {
        auto &input = config["input"];
        findAndSet(input, "enable_unicode_paste", settings.input.enableUnicodePaste);
        findAndSet(input, "keep_ime_open", settings.input.keepImeOpen);
        findAndSet(input, "pos_update_policy", settings.input.posUpdatePolicy);
    }
}

auto DoSerialize(Settings &settings) -> toml::value
{
    toml::value core{toTomlTable(settings)};
    toml::value resources{toTomlTable(settings.resources)};
    toml::value appearance{toTomlTable(settings.appearance)};
    toml::value input{toTomlTable(settings.input)};
    return {
        toml::table{
            {"core", core},
            {"resources", resources},
            {"appearance", appearance},
            {"input", input},
        }
    };
}
}