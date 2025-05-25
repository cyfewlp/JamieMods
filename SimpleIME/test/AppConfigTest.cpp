#include "configs/AppConfig.h"

#include "configs/converter.h"

#include <SimpleIni.h>
#include <gtest/gtest.h>
#include <ranges>
#include <spdlog/spdlog.h>
#include <string>

using namespace LIBC_NAMESPACE::Ime;

TEST(PropertyTest, ConfigName)
{
    Property fontSize_{14.0F, "fontSize"};
    ASSERT_STREQ(fontSize_.ConfigName(), "Font_Size");
    ASSERT_EQ(fontSize_.Value(), 14.0F);
}

TEST(SimpleIniTest, SaveIni)
{
    CSimpleIniA ini(false, false, true);

    SI_Error error = ini.LoadFile("SimpleIME.ini");
    if (error == SI_OK)
    {
        ini.SetValue("General", "Tool_Window_Shortcut_Key", "0x2222");
        error = ini.SaveFile("SimpleIME-temp.ini");
        ASSERT_EQ(error, SI_OK);
    }
}

namespace PLUGIN_NAMESPACE
{
TEST(SimpleIniTest, HexString)
{
    CSimpleIniA ini;

    ini.LoadData(R"(
[General]
name = dave
color = 0xFFFFFF
colorAbgr = 0xFF00FFFF
1)");
    ASSERT_STREQ(ini.GetValue("General", "name"), "dave");
    ASSERT_EQ(ini.GetLongValue("General", "color"), 0xFFFFFF);
    char *p_end{};
    ASSERT_EQ(std::strtoul("FFFFFFFF", &p_end, 16), 0xFFFFFFFF);
    ASSERT_EQ(std::strtol("0xFF", &p_end, 16), 0xFF);
    ASSERT_STREQ(ini.GetValue("General", "colorAbgr"), "0xFF00FFFF");
}

TEST(PropertyTest, ConvertUint32)
{
    ASSERT_EQ(converter<uint32_t>::convert("0xFF"), 0xFF);
    ASSERT_EQ(converter<uint32_t>::convert("-200"), -200);
    ASSERT_EQ(converter<uint32_t>::convert("0xFF00FF00"), 0xFF00FF00);
    ASSERT_EQ(converter<uint32_t>::convert("ddddd"), 0);
}

TEST(PropertyTest, NoSection)
{
    CSimpleIniA ini;

    ini.LoadData(R"(
name = dave
utf8 = 你好
emoji = 😊
)");
    ASSERT_STREQ(ini.GetValue("", "name"), "dave");
    ASSERT_STREQ(ini.GetValue("", "utf8"), "你好");
    ASSERT_STREQ(ini.GetValue("", "emoji"), "😊");
}

TEST(PropertyTest, ConvertLogLevel)
{
    using namespace spdlog::level;
    ASSERT_EQ(converter<level_enum>::convert("trace"), 0);
    ASSERT_EQ(converter<level_enum>::convert("debug"), 1);
    ASSERT_EQ(converter<level_enum>::convert("info"), 2);
    ASSERT_EQ(converter<level_enum>::convert("warn"), 3);
    ASSERT_EQ(converter<level_enum>::convert("error"), 4);
    ASSERT_EQ(converter<level_enum>::convert("critical"), 5);
    ASSERT_EQ(converter<level_enum>::convert("off"), 6);
    ASSERT_EQ(converter<level_enum>::convert("invalid_level"), 6);
}

TEST(PropertyTest, ConvertBool)
{
    ASSERT_EQ(converter<bool>::convert("true"), true);
    ASSERT_EQ(converter<bool>::convert("1"), true);
    ASSERT_EQ(converter<bool>::convert("-1"), true);

    ASSERT_EQ(converter<bool>::convert("false"), false);
    ASSERT_EQ(converter<bool>::convert("invalid string"), false);
    ASSERT_EQ(converter<bool>::convert("0"), false);
}

TEST(PropertyTest, ConvertMultiLineTOVector)
{
    auto result = converter<std::unordered_set<std::string>>::convert("value1\nvalue2", {});
    ASSERT_EQ(result.size(), 2);
    ASSERT_TRUE(result.contains("value1"));
    ASSERT_TRUE(result.contains("value2"));
}

TEST(AppConfigTest, IniLoad)
{
    AppConfig defaultConfig;
    ASSERT_NO_THROW(AppConfig::LoadIni("SimpleIME.ini"));

    const AppConfig &loadedConfig = AppConfig::GetConfig();
    ASSERT_NE(defaultConfig.GetLogLevel(), loadedConfig.GetLogLevel());
    ASSERT_NE(defaultConfig.GetFlushLevel(), loadedConfig.GetFlushLevel());

    ASSERT_EQ(loadedConfig.GetLogLevel(), spdlog::level::warn);
    ASSERT_EQ(loadedConfig.GetFlushLevel(), spdlog::level::warn);

    const auto &uiConfig        = loadedConfig.GetAppUiConfig();
    const auto &defaultUiConfig = defaultConfig.GetAppUiConfig();

    ASSERT_NE(uiConfig.HighlightTextColor(), defaultUiConfig.HighlightTextColor());
    ASSERT_EQ(uiConfig.HighlightTextColor(), 0xFF0000F1);

    ASSERT_STRNE(uiConfig.EastAsiaFontFile().c_str(), defaultUiConfig.EastAsiaFontFile().c_str());
    ASSERT_STREQ(uiConfig.EastAsiaFontFile().c_str(), "C:/path/to/east-asia/font");

    ASSERT_STRNE(uiConfig.EmojiFontFile().c_str(), defaultUiConfig.EmojiFontFile().c_str());
    ASSERT_STREQ(uiConfig.EmojiFontFile().c_str(), R"(C:\path\to\emoji-font)");

    ASSERT_EQ(loadedConfig.GetToolWindowShortcutKey(), 0x1111);
    ASSERT_EQ(loadedConfig.EnableTsf(), false);
    ASSERT_EQ(uiConfig.FontSize(), 160.0F);
    ASSERT_EQ(uiConfig.UseClassicTheme(), true);
    ASSERT_EQ(uiConfig.ThemeDirectory(), R"(a/b/c)");
}
}
