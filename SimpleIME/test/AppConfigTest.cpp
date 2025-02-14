#include "AppConfig.h"
#include "Configs.h"
#include "configs/converter.h"

#include <SimpleIni.h>
#include <gtest/gtest.h>
#include <string>

using namespace LIBC_NAMESPACE::SimpleIME;

// Demonstrate some basic assertions.
TEST(VarNameConvertTest, BasicAssertions)
{
    ASSERT_STREQ(ConvertCamelCaseToUnderscore("Upper").c_str(), "Upper");
    ASSERT_STREQ(ConvertCamelCaseToUnderscore("lower").c_str(), "Lower");
    ASSERT_STREQ(ConvertCamelCaseToUnderscore("all_lower").c_str(), "All_Lower");
    ASSERT_STREQ(ConvertCamelCaseToUnderscore("testVar").c_str(), "Test_Var");
    ASSERT_STREQ(ConvertCamelCaseToUnderscore("FirstUpper").c_str(), "First_Upper");
    ASSERT_STREQ(ConvertCamelCaseToUnderscore("HTTPVar").c_str(), "HTTP_Var");
}

TEST(PropertyTest, ConfigName)
{
    Property fontSize_{14.0F, "fontSize"};
    ASSERT_STREQ(fontSize_.ConfigName(), "Font_Size");
    ASSERT_EQ(fontSize_.Value(), 14.0F);
}

TEST(SimpleIniTest, GetValue)
{
    CSimpleIniA ini;

    ini.LoadData(R"(
[General]
Color_Abgr1 = 0xFF00FFFF
Color_Abgr2 = 100
Color_Abgr3 = 1212121212121212
1)");
    Property hexProperty1{0xFFFFFFFF, "colorAbgr1"};
    GetSimpleIniValue<uint32_t>(ini, "General", hexProperty1);
    ASSERT_EQ(hexProperty1.Value(), 0xFF00FFFF);

    Property hexProperty2{0xFFFFFFFF, "colorAbgr2"};
    GetSimpleIniValue<uint32_t>(ini, "General", hexProperty2);
    ASSERT_EQ(hexProperty2.Value(), 100);

    Property hexProperty3{0xFFFFFFFF, "colorAbgr3"};
    GetSimpleIniValue<uint32_t>(ini, "General", hexProperty3);
    ASSERT_EQ(hexProperty3.Value(), UINT_MAX);
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

    TEST(AppConfigTest, IniLoad)
    {
        AppConfig defaultConfig;
        ASSERT_NO_THROW(AppConfig::LoadIni("SimpleIME.ini"));

        AppConfig *loadedConfig = AppConfig::GetConfig();
        ASSERT_NE(loadedConfig, nullptr);
        ASSERT_NE(defaultConfig.GetLogLevel(), loadedConfig->GetLogLevel());
        ASSERT_NE(defaultConfig.GetFlushLevel(), loadedConfig->GetFlushLevel());

        ASSERT_EQ(loadedConfig->GetLogLevel(), spdlog::level::warn);
        ASSERT_EQ(loadedConfig->GetFlushLevel(), spdlog::level::warn);

        const auto &uiConfig        = loadedConfig->GetAppUiConfig();
        const auto &defaultUiConfig = defaultConfig.GetAppUiConfig();

        ASSERT_NE(uiConfig.TextColor(), defaultUiConfig.TextColor());
        ASSERT_EQ(uiConfig.TextColor(), 0xFF0000F0);

        ASSERT_NE(uiConfig.HighlightTextColor(), defaultUiConfig.HighlightTextColor());
        ASSERT_EQ(uiConfig.HighlightTextColor(), 0xFF0000F1);

        ASSERT_NE(uiConfig.WindowBgColor(), defaultUiConfig.WindowBgColor());
        ASSERT_EQ(uiConfig.WindowBgColor(), 0xFF0000F3);

        ASSERT_NE(uiConfig.WindowBorderColor(), defaultUiConfig.WindowBorderColor());
        ASSERT_EQ(uiConfig.WindowBorderColor(), 0xFF0000F2);

        ASSERT_NE(uiConfig.BtnColor(), defaultUiConfig.BtnColor());
        ASSERT_EQ(uiConfig.BtnColor(), 0xFF0000F4);

        ASSERT_STRNE(uiConfig.EastAsiaFontFile().c_str(), defaultUiConfig.EastAsiaFontFile().c_str());
        ASSERT_STREQ(uiConfig.EastAsiaFontFile().c_str(), "C:/path/to/east-asia/font");

        ASSERT_STRNE(uiConfig.EmojiFontFile().c_str(), defaultUiConfig.EmojiFontFile().c_str());
        ASSERT_STREQ(uiConfig.EmojiFontFile().c_str(), R"(C:\path\to\emoji-font)");
    }
}
