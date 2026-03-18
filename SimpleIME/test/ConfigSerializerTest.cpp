#include "configs/ConfigSerializer.h"

#include "RandomUtils.h"
#include "configs/configuration.h"
#include "ui/Settings.h"

#include <gtest/gtest.h>
#include <random>
#include <spdlog/common.h>

namespace fs = std::filesystem;

TEST(ConfigSerializerTest, should_return_default_configuration_if_file_not_exists)
{
    Ime::Configuration defaultConfiguration = Ime::GetDefaultConfiguration();
    const auto         configuration        = Ime::ConfigSerializer::LoadConfiguration("non_existent_file.toml");

    EXPECT_EQ(configuration.shortcut, defaultConfiguration.shortcut);
    EXPECT_EQ(configuration.enableMod, defaultConfiguration.enableMod);
    EXPECT_EQ(configuration.enableTsf, defaultConfiguration.enableTsf);
    EXPECT_EQ(configuration.fixInconsistentTextEntryCount, defaultConfiguration.fixInconsistentTextEntryCount);
    EXPECT_EQ(configuration.logging.level, defaultConfiguration.logging.level);
    EXPECT_EQ(configuration.logging.flushLevel, defaultConfiguration.logging.flushLevel);

    EXPECT_EQ(configuration.resources.translationDir, defaultConfiguration.resources.translationDir);
    EXPECT_EQ(configuration.resources.fontPathList, defaultConfiguration.resources.fontPathList);

    EXPECT_EQ(configuration.appearance.themeSourceColor, defaultConfiguration.appearance.themeSourceColor);
    EXPECT_EQ(configuration.appearance.themeContrastLevel, defaultConfiguration.appearance.themeContrastLevel);
    EXPECT_EQ(configuration.appearance.themeDarkMode, defaultConfiguration.appearance.themeDarkMode);
    EXPECT_EQ(configuration.appearance.language, defaultConfiguration.appearance.language);
    EXPECT_EQ(configuration.appearance.zoom, defaultConfiguration.appearance.zoom);
    EXPECT_EQ(configuration.appearance.errorDisplayDuration, defaultConfiguration.appearance.errorDisplayDuration);
    EXPECT_EQ(configuration.appearance.showSettings, defaultConfiguration.appearance.showSettings);
    EXPECT_EQ(configuration.appearance.verticalCandidateList, defaultConfiguration.appearance.verticalCandidateList);

    EXPECT_EQ(configuration.input.enableUnicodePaste, defaultConfiguration.input.enableUnicodePaste);
    EXPECT_EQ(configuration.input.keepImeOpen, defaultConfiguration.input.keepImeOpen);
    EXPECT_EQ(configuration.input.posUpdatePolicy, defaultConfiguration.input.posUpdatePolicy);
}

TEST(ConfigSerializerTest, should_save_load_configuration)
{
    const auto toSaveConfiguration = ImeTest::GetRandomConfiguation();

    std::filesystem::path filePath = "test_temp_configuration.toml";
    Ime::ConfigSerializer::SaveConfiguration(filePath, toSaveConfiguration);

    ASSERT_TRUE(std::filesystem::exists(filePath)) << "Save success but can't find the file!";

    const auto loadedConfig = Ime::ConfigSerializer::LoadConfiguration(filePath);

    EXPECT_EQ(loadedConfig.shortcut, toSaveConfiguration.shortcut);
    EXPECT_EQ(loadedConfig.enableMod, toSaveConfiguration.enableMod);
    EXPECT_EQ(loadedConfig.enableTsf, toSaveConfiguration.enableTsf);
    EXPECT_EQ(loadedConfig.fixInconsistentTextEntryCount, toSaveConfiguration.fixInconsistentTextEntryCount);
    EXPECT_EQ(loadedConfig.logging.level, toSaveConfiguration.logging.level);
    EXPECT_EQ(loadedConfig.logging.flushLevel, toSaveConfiguration.logging.flushLevel);

    EXPECT_EQ(loadedConfig.resources.translationDir, toSaveConfiguration.resources.translationDir);
    EXPECT_EQ(loadedConfig.resources.fontPathList, toSaveConfiguration.resources.fontPathList);

    EXPECT_EQ(loadedConfig.appearance.themeSourceColor, toSaveConfiguration.appearance.themeSourceColor);
    EXPECT_EQ(loadedConfig.appearance.themeContrastLevel, toSaveConfiguration.appearance.themeContrastLevel);
    EXPECT_EQ(loadedConfig.appearance.themeDarkMode, toSaveConfiguration.appearance.themeDarkMode);
    EXPECT_EQ(loadedConfig.appearance.language, toSaveConfiguration.appearance.language);
    EXPECT_EQ(loadedConfig.appearance.zoom, toSaveConfiguration.appearance.zoom);
    EXPECT_EQ(loadedConfig.appearance.errorDisplayDuration, toSaveConfiguration.appearance.errorDisplayDuration);
    EXPECT_EQ(loadedConfig.appearance.showSettings, toSaveConfiguration.appearance.showSettings);
    EXPECT_EQ(loadedConfig.appearance.verticalCandidateList, toSaveConfiguration.appearance.verticalCandidateList);

    EXPECT_EQ(loadedConfig.input.enableUnicodePaste, toSaveConfiguration.input.enableUnicodePaste);
    EXPECT_EQ(loadedConfig.input.keepImeOpen, toSaveConfiguration.input.keepImeOpen);
    EXPECT_EQ(loadedConfig.input.posUpdatePolicy, toSaveConfiguration.input.posUpdatePolicy);

    // file already writted. remove it after test.
    std::filesystem::remove(filePath);
}
