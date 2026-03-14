//
// Created by jamie on 2026/3/8.
//

#include "RandomUtils.h"
#include "configs/configuration.h"
#include "configs/settings_converter.h"
#include "imgui.h"
#include "imguiex/ErrorNotifier.h"
#include "ui/Settings.h"

#include <gtest/gtest.h>

void ErrorNotifier::addError(std::string_view msg, const ErrorMsg::Level level)
{
    if (level < m_currentLevel)
    {
        return;
    }
    if (errors.size() >= MaxMessages)
    {
        errors.pop_front();
    }
    errors.push_back({.text = std::string(msg), .level = level, .confirmed = false});
}

TEST(ConfigurattionToSettingsTest, should_convert_shortcut_string_to_ImGuiKeyChord)
{
    Ime::Configuration configuration;

    Ime::Settings defaultSettings = Ime::GetDefaultSettings();

    configuration.shortcut = "F2";
    auto settings          = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiKey_F2);

    configuration.shortcut = "Ctrl+F1";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiMod_Ctrl | ImGuiKey_F1);

    configuration.shortcut = "Alt+F2";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiMod_Alt | ImGuiKey_F2);

    configuration.shortcut = "SHiFt+F3";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiMod_Shift | ImGuiKey_F3);

    configuration.shortcut = "SHiFt    +alt+CTRL+F4";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiKey_F4);

    configuration.shortcut = " F5 +  ctrl +   alt + shift  ";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiKey_F5);

    configuration.shortcut = " F7 +  ctrl +   alt + shift +super ";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiMod_Alt | ImGuiMod_Super | ImGuiKey_F7);

    configuration.shortcut = " F6 +  ctrl +   alt + shift + ";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, defaultSettings.shortcut) << "Should not support shortcut when not end with modifier or key";

    configuration.shortcut = " F6  + A +  ctrl +   alt";
    settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.shortcut, defaultSettings.shortcut) << "Should not support shortcut when contains multiple normal keys";
}

TEST(ConfigurattionToSettingsTest, should_convert_spdlog_level_string_to_spdlog_level_enum)
{
    Ime::Configuration configuration;

    Ime::Settings defaultSettings = Ime::GetDefaultSettings();

    configuration.logging.level = "debug";
    auto settings               = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.logging.level, spdlog::level::debug) << "should correct convert log level string to spdlog level enum";

    configuration.logging.level = "  debug   ";
    settings                    = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.logging.level, spdlog::level::debug) << "should trim the log level string before convert";

    const std::vector<spdlog::level::level_enum> expectedLevels = {
        spdlog::level::trace,
        spdlog::level::debug,
        spdlog::level::info,
        spdlog::level::warn,
        spdlog::level::err,
        spdlog::level::critical,
        spdlog::level::off
    };
    for (size_t i{}; auto &testLevelString : {"traCe", "dEbug", "iNfo", "Warn", "ErR", "critiCal", "oFf"})
    {
        configuration.logging.level = testLevelString;
        settings                    = Ime::ConvertConfigurationToSettings(configuration);
        EXPECT_EQ(settings.logging.level, expectedLevels[i++]) << "should case insensitive";
    }

    configuration.logging.level = "unknown level";
    settings                    = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.logging.level, defaultSettings.logging.level) << "should not support unknown log level string, and keep the default value";
}

TEST(ConfigurattionToSettingsTest, should_convert_WindowPolicy_string_to_enum)
{
    Ime::Configuration configuration;

    Ime::Settings defaultSettings = Ime::GetDefaultSettings();

    configuration.input.posUpdatePolicy = "unknown policy";
    auto settings                       = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.input.posUpdatePolicy, defaultSettings.input.posUpdatePolicy) << "should not support unknown policy string";

    configuration.input.posUpdatePolicy = " based_on_caret ";
    settings                            = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.input.posUpdatePolicy, Ime::Settings::WindowPosUpdatePolicy::BASED_ON_CARET) << "should trim the policy string before convert";

    configuration.input.posUpdatePolicy = "based_On_cuRsor";
    settings                            = Ime::ConvertConfigurationToSettings(configuration);
    EXPECT_EQ(settings.input.posUpdatePolicy, Ime::Settings::WindowPosUpdatePolicy::BASED_ON_CURSOR) << "should case insensitive";
}

TEST(ConfigurattionToSettingsTest, should_set_base_type_member_value_from_configuration)
{
    const auto configuration = ImeTest::GetRandomConfiguation();

    auto settings = Ime::ConvertConfigurationToSettings(configuration);

    EXPECT_EQ(settings.enableMod, configuration.enableMod);
    EXPECT_EQ(settings.enableTsf, configuration.enableTsf);

    EXPECT_EQ(settings.resources.translationDir, configuration.resources.translationDir);
    EXPECT_EQ(settings.resources.fontPathList, configuration.resources.fontPathList);

    EXPECT_EQ(settings.appearance.schemeConfig.sourceColor, configuration.appearance.themeSourceColor);
    EXPECT_EQ(settings.appearance.schemeConfig.contrastLevel, configuration.appearance.themeContrastLevel);
    EXPECT_EQ(settings.appearance.schemeConfig.darkMode, configuration.appearance.themeDarkMode);
    EXPECT_EQ(settings.appearance.language, configuration.appearance.language);
    EXPECT_EQ(settings.appearance.zoom, configuration.appearance.zoom);
    EXPECT_EQ(settings.appearance.errorDisplayDuration, configuration.appearance.errorDisplayDuration);
    EXPECT_EQ(settings.appearance.showSettings, configuration.appearance.showSettings);

    EXPECT_EQ(settings.input.enableUnicodePaste, configuration.input.enableUnicodePaste);
    EXPECT_EQ(settings.input.keepImeOpen, configuration.input.keepImeOpen);
}

TEST(ConfigurattionToSettingsTest, should_convert_default_configuration_to_default_settings)
{
    Ime::Configuration configuration = Ime::GetDefaultConfiguration();

    Ime::Settings defaultSettings = Ime::GetDefaultSettings();
    auto          settings        = Ime::ConvertConfigurationToSettings(configuration);

    EXPECT_EQ(settings.enableMod, defaultSettings.enableMod);
    EXPECT_EQ(settings.enableTsf, defaultSettings.enableTsf);
    EXPECT_EQ(settings.fixInconsistentTextEntryCount, defaultSettings.fixInconsistentTextEntryCount);
    EXPECT_EQ(settings.shortcut, defaultSettings.shortcut);

    EXPECT_EQ(settings.logging.level, defaultSettings.logging.level);
    EXPECT_EQ(settings.logging.flushLevel, defaultSettings.logging.flushLevel);

    EXPECT_EQ(settings.resources.translationDir, defaultSettings.resources.translationDir);
    EXPECT_EQ(settings.resources.fontPathList, defaultSettings.resources.fontPathList);

    EXPECT_EQ(settings.appearance.schemeConfig.sourceColor, defaultSettings.appearance.schemeConfig.sourceColor);
    EXPECT_EQ(settings.appearance.schemeConfig.contrastLevel, defaultSettings.appearance.schemeConfig.contrastLevel);
    EXPECT_EQ(settings.appearance.schemeConfig.darkMode, defaultSettings.appearance.schemeConfig.darkMode);
    EXPECT_EQ(settings.appearance.language, defaultSettings.appearance.language);
    EXPECT_EQ(settings.appearance.zoom, defaultSettings.appearance.zoom);
    EXPECT_EQ(settings.appearance.errorDisplayDuration, defaultSettings.appearance.errorDisplayDuration);
    EXPECT_EQ(settings.appearance.showSettings, defaultSettings.appearance.showSettings);

    EXPECT_EQ(settings.input.posUpdatePolicy, defaultSettings.input.posUpdatePolicy);
    EXPECT_EQ(settings.input.enableUnicodePaste, defaultSettings.input.enableUnicodePaste);
    EXPECT_EQ(settings.input.keepImeOpen, defaultSettings.input.keepImeOpen);
}

TEST(SettingsToConfigurationTest, should_convert_shortcut_ImGuiKeyChord_to_string)
{
    Ime::Settings settings{};

    settings.shortcut = ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiKey_F1;
    auto config       = Ime::ConvertSettingsToConfiguration(settings);
    EXPECT_STREQ(config.shortcut.c_str(), "ctrl+alt+f1") << "should convert ImGuiKeyChord to string with modifier and key, and all in lower case";

    settings.shortcut = ImGuiMod_Ctrl | ImGuiMod_Super | ImGuiMod_Alt | ImGuiKey_F1;
    config            = Ime::ConvertSettingsToConfiguration(settings);
    EXPECT_STREQ(config.shortcut.c_str(), "ctrl+alt+super+f1") << "should support all modifiers.";

    settings.shortcut = ImGuiMod_Ctrl | ImGuiMod_Alt;
    config            = Ime::ConvertSettingsToConfiguration(settings);
    EXPECT_STREQ(config.shortcut.c_str(), "f2") << "should unsupport if the shortcut does not contain any named key, and fallback to default value";

    settings.shortcut = ImGuiKey_A;
    config            = Ime::ConvertSettingsToConfiguration(settings);
    EXPECT_STREQ(config.shortcut.c_str(), "a") << "should convert normal key without modifier to string, and keep the case";
}

TEST(SettingsToConfigurationTest, should_apply_rgb_mask_to_source_color)
{
    Ime::Settings settings{};
    settings.appearance.schemeConfig.sourceColor = 0x12345678; // the alpha channel should be ignored

    auto config = Ime::ConvertSettingsToConfiguration(settings);
    EXPECT_EQ(config.appearance.themeSourceColor, 0x00345678) << "should apply RGB mask to source color when convert settings to configuration";
}
