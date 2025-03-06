#include "common/config.h"

#include "configs/converter.h"

#include "ImGuiThemeLoader.h"

#include "SimpleIni.h"
#include <gtest/gtest.h>

namespace LIBC_NAMESPACE_DECL
{
    using namespace Ime;

    TEST(ImGuiThemeLoaderTest, ImGuiDir)
    {
        ASSERT_EQ(converter<ImGuiDir>::convert("invalid str", ImGuiDir_None), ImGuiDir_None);
        ASSERT_EQ(converter<ImGuiDir>::convert("Left", ImGuiDir_None), ImGuiDir_Left);
        ASSERT_EQ(converter<ImGuiDir>::convert("Right", ImGuiDir_None), ImGuiDir_Right);
        ASSERT_EQ(converter<ImGuiDir>::convert("Up", ImGuiDir_None), ImGuiDir_Up);
        ASSERT_EQ(converter<ImGuiDir>::convert("Down", ImGuiDir_None), ImGuiDir_Down);
    }

    TEST(ImGuiThemeLoaderTest, ImGuiDirFromIni)
    {
        CSimpleIniA ini;
        ini.LoadData(R"(
[General]
imgui_dir1 = "Left"
imgui_dir2 = "Right"
1)");
        ASSERT_EQ(converter<ImGuiDir>::convert(ini.GetValue("General", "imgui_dir1"), ImGuiDir_None), ImGuiDir_Left);
        ASSERT_EQ(converter<ImGuiDir>::convert(ini.GetValue("General", "imgui_dir2"), ImGuiDir_None), ImGuiDir_Right);
    }

    TEST(ImGuiThemeLoaderTest, Theme)
    {
        spdlog::set_level(spdlog::level::trace);
        ImGuiThemeLoader      loader;
        std::vector<std::string> themes;
        loader.GetAllThemeNames(R"(D:\repo\JamieMods\SimpleIME\contrib\themes)", themes);
        ASSERT_EQ(themes.size(), 3);

        ImGuiStyle style{};
        ASSERT_EQ(loader.LoadTheme("visual_studio", style), true);
    }
}