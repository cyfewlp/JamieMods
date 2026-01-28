#include "configs/ConfigSerializer.h"

#include "RandomUtils.h"
#include "ui/Settings.h"

#include <gtest/gtest.h>
#include <random>
#include <spdlog/common.h>

using namespace LIBC_NAMESPACE::Ime;

namespace fs = std::filesystem;

TEST(ConfigSerializerTest, DeserializeAll)
{
    Settings settings;

    ConfigSerializer::Deserialize("SimpleIME.toml", settings);

    ASSERT_EQ(settings.shortcutKey, 0x9999);
    ASSERT_EQ(settings.enableMod, false);
    ASSERT_EQ(settings.enableTsf, false);
    ASSERT_EQ(settings.logging.level, spdlog::level::off);
    ASSERT_EQ(settings.logging.flushLevel, spdlog::level::off);
    ASSERT_EQ(settings.resources.translationDir, "path/to/translation");
    auto exceptFonts = std::vector<std::string>{"path/to/font1.ttf", "path/to/font2.ttf"};
    ASSERT_EQ(settings.resources.fontPathList, exceptFonts);
    ASSERT_EQ(settings.appearance.language, "a language");
    ASSERT_EQ(settings.appearance.themeSeedArgb, 0x33333333);
    ASSERT_EQ(settings.appearance.themeDarkMode, false);
    ASSERT_EQ(settings.appearance.errorDisplayDuration, 9999);
    ASSERT_EQ(settings.appearance.showSettings, true);

    // raw value: "xxx"
    ASSERT_EQ(settings.input.posUpdatePolicy, Settings::WindowPosUpdatePolicy::NONE);
    ASSERT_EQ(settings.input.enableUnicodePaste, false);
    ASSERT_EQ(settings.input.keepImeOpen, true);
}

TEST(ConfigSerializerTest, ShouldNoException)
{
    const auto    emptyPath = fs::path("empty.toml");
    std::ofstream emptyFile(emptyPath);
    emptyFile.close();

    Settings settings;
    ASSERT_NO_THROW(ConfigSerializer::Deserialize("empty.toml", settings));
    ASSERT_NO_THROW(ConfigSerializer::Serialize("empty.toml", settings));

    std::filesystem::remove(emptyPath);
}

TEST(ConfigSerializerTest, ShouldSerializeCorrectly)
{
    ImeTest::RandomUtils random;

    Settings settings;
    settings.shortcutKey = random.NextInt(0x1, 0xffffff);
    settings.enableMod   = random.NextBool();
    settings.enableTsf   = random.NextBool();

    settings.logging.level      = static_cast<spdlog::level::level_enum>(random.NextInt(0, spdlog::level::off));
    settings.logging.flushLevel = static_cast<spdlog::level::level_enum>(random.NextInt(0, spdlog::level::off));

    settings.resources.translationDir = random.NextStrinng(10);
    settings.resources.fontPathList   = std::vector{random.NextStrinng(10), random.NextStrinng(10)};

    settings.appearance.fontSizeScale        = std::round(random.NextFloat(1.f, 9999.f) * 100.f) / 100.f;
    settings.appearance.themeSeedArgb        = random.NextInt(0, 0xffffffff);
    settings.appearance.themeDarkMode        = random.NextBool();
    settings.appearance.language             = random.NextStrinng(10);
    settings.appearance.errorDisplayDuration = random.NextInt(0, 0xffff);
    settings.appearance.showSettings         = random.NextBool();

    settings.input.enableUnicodePaste = random.NextBool();
    settings.input.keepImeOpen        = random.NextBool();
    settings.input.posUpdatePolicy    = static_cast<Settings::WindowPosUpdatePolicy>(random.NextInt(0, 2));

    const auto testFilePath = fs::path("SerializeTest.toml");
    ConfigSerializer::Serialize(testFilePath.string(), settings);

    Settings deserialized;
    ASSERT_FALSE(settings == deserialized);
    ConfigSerializer::Deserialize(testFilePath.string(), deserialized);

    ASSERT_EQ(deserialized.enableMod, settings.enableMod);
    ASSERT_EQ(deserialized.enableTsf, settings.enableTsf);
    ASSERT_EQ(deserialized.shortcutKey, settings.shortcutKey);
    ASSERT_EQ(deserialized.logging, settings.logging);
    ASSERT_EQ(deserialized.resources, settings.resources);
    ASSERT_EQ(deserialized.appearance, settings.appearance);
    ASSERT_EQ(deserialized.input, settings.input);

    std::filesystem::remove(testFilePath);
}