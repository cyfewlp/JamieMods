#include "configs/ConfigSerializer.h"

#include "RandomUtils.h"
#include "configs/configuration.h"
#include "ui/Settings.h"

#include <gtest/gtest.h>
#include <random>
#include <spdlog/common.h>

namespace fs = std::filesystem;

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

    EXPECT_EQ(loadedConfig.input.enableUnicodePaste, toSaveConfiguration.input.enableUnicodePaste);
    EXPECT_EQ(loadedConfig.input.keepImeOpen, toSaveConfiguration.input.keepImeOpen);
    EXPECT_EQ(loadedConfig.input.posUpdatePolicy, toSaveConfiguration.input.posUpdatePolicy);

    // file already writted. remove it after test.
    std::filesystem::remove(filePath);
}
