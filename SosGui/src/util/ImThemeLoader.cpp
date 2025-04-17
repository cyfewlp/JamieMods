//
// Created by jamie on 2025/4/16.
//

#include "util/ImThemeLoader.h"
#include "common/log.h"

namespace LIBC_NAMESPACE_DECL
{
    void ImThemeLoader::Loader::loadThemes()
    {
        auto config = toml::parse_file(IM_THEME_FILE_PATH);
        g_availableThemes.clear();

        if (!config.contains("themes") || !config["themes"].is_array())
        {
            log_error("Invalid themes file");
            return;
        }
        const auto &themesArray = config["themes"].as_array();

        for (const toml::node &themeNode : *themesArray)
        {
            if (!themeNode.is_table()) { continue; }

            auto theme = *themeNode.as_table();
            if (std::string name = theme["name"].value_or(""); !name.empty())
            {
                g_availableThemes.emplace_back(std::move(name));
            }
        }
    }

    void ImThemeLoader::Loader::UseTheme(const size_t themeIndex)
    {
        if (themeIndex >= g_availableThemes.size()) { return; }

        auto        config      = toml::parse_file(IM_THEME_FILE_PATH);
        const auto &themesArray = config["themes"].as_array();
        if (themesArray == nullptr) { return; }

        try
        {
            toml::node &theme_node = themesArray->at(themeIndex);
            if (!theme_node.is_table())
            {
                throw std::runtime_error(std::format("Can't find theme with index: {}", themeIndex));
            }
            toml::table theme_table = *theme_node.as_table();
            DoUseTheme(theme_table);
        }
        catch (std::exception &e)
        {
            log_error("Theme loading error: {}", e.what());
        }
    }

    void ImThemeLoader::Loader::DoUseTheme(toml::table &themeTable)
    {
        const auto colors_node = themeTable["style"]["colors"];
        const auto style_node  = themeTable["style"];
        auto      &style       = ImGui::GetStyle();
        ConfigImGuiStyle(style_node, style);
        ConfigImGuiColor(colors_node, style);
    }

#define CONFIG_STYLE(type, styleName) converter<type>::convert(style_node[#styleName].value_or(""), style.styleName)

    void ImThemeLoader::Loader::ConfigImGuiStyle(const toml::node_view<toml::node> &style_node, ImGuiStyle &style)
    {
        if (!style_node.is_table()) { return; }
        CONFIG_STYLE(float, Alpha);
        CONFIG_STYLE(float, DisabledAlpha);
        CONFIG_STYLE(ImVec2, WindowPadding);
        CONFIG_STYLE(float, WindowRounding);
        CONFIG_STYLE(float, WindowBorderSize);
        CONFIG_STYLE(float, WindowBorderHoverPadding);
        CONFIG_STYLE(ImVec2, WindowMinSize);
        CONFIG_STYLE(ImVec2, WindowTitleAlign);
        CONFIG_STYLE(float, ChildRounding);
        CONFIG_STYLE(float, ChildBorderSize);
        CONFIG_STYLE(float, PopupRounding);
        CONFIG_STYLE(float, PopupBorderSize);
        CONFIG_STYLE(ImVec2, FramePadding);
        CONFIG_STYLE(float, FrameRounding);
        CONFIG_STYLE(float, FrameBorderSize);
        CONFIG_STYLE(ImVec2, ItemSpacing);
        CONFIG_STYLE(ImVec2, ItemInnerSpacing);
        CONFIG_STYLE(ImVec2, CellPadding);
        CONFIG_STYLE(ImVec2, TouchExtraPadding);
        CONFIG_STYLE(float, IndentSpacing);
        CONFIG_STYLE(float, ColumnsMinSpacing);
        CONFIG_STYLE(float, ScrollbarSize);
        CONFIG_STYLE(float, ScrollbarRounding);
        CONFIG_STYLE(float, GrabMinSize);
        CONFIG_STYLE(float, GrabRounding);
        CONFIG_STYLE(float, LogSliderDeadzone);
        CONFIG_STYLE(float, ImageBorderSize);
        CONFIG_STYLE(float, TabRounding);
        CONFIG_STYLE(float, TabBorderSize);
        CONFIG_STYLE(float, TabCloseButtonMinWidthSelected);
        CONFIG_STYLE(float, TabCloseButtonMinWidthUnselected);
        CONFIG_STYLE(float, TabBarBorderSize);
        CONFIG_STYLE(float, TabBarOverlineSize);
        CONFIG_STYLE(float, TableAngledHeadersAngle);
        CONFIG_STYLE(ImGuiDir, WindowMenuButtonPosition);
        CONFIG_STYLE(ImVec2, TableAngledHeadersTextAlign);
        CONFIG_STYLE(ImGuiDir, ColorButtonPosition);
        CONFIG_STYLE(ImVec2, ButtonTextAlign);
        CONFIG_STYLE(ImVec2, SelectableTextAlign);
        CONFIG_STYLE(float, SeparatorTextBorderSize);
        CONFIG_STYLE(ImVec2, SeparatorTextAlign);
        CONFIG_STYLE(ImVec2, SeparatorTextPadding);
        CONFIG_STYLE(ImVec2, DisplayWindowPadding);
        CONFIG_STYLE(ImVec2, DisplaySafeAreaPadding);
        CONFIG_STYLE(float, DockingSeparatorSize);
        CONFIG_STYLE(float, MouseCursorScale);
        CONFIG_STYLE(float, CurveTessellationTol);
        CONFIG_STYLE(float, CircleTessellationMaxError);
    }

#define CONFIG_COLOR(type, styleName) converter<type>::convert(style_node[#styleName].value_or(""), style.styleName)
#define CONFIG_COLOR(colorName)       ColorConvert(colors_node[#colorName].value_or(""), style.Colors[ImGuiCol_##colorName])

    void ImThemeLoader::Loader::ConfigImGuiColor(const toml::node_view<toml::node> &colors_node, ImGuiStyle &style)
    {
        if (!colors_node.is_table()) { return; }
        CONFIG_COLOR(Text);
        CONFIG_COLOR(TextDisabled);
        CONFIG_COLOR(WindowBg);
        CONFIG_COLOR(ChildBg);
        CONFIG_COLOR(PopupBg);
        CONFIG_COLOR(Border);
        CONFIG_COLOR(BorderShadow);
        CONFIG_COLOR(FrameBg);
        CONFIG_COLOR(FrameBgHovered);
        CONFIG_COLOR(FrameBgActive);
        CONFIG_COLOR(TitleBg);
        CONFIG_COLOR(TitleBgActive);
        CONFIG_COLOR(TitleBgCollapsed);
        CONFIG_COLOR(MenuBarBg);
        CONFIG_COLOR(ScrollbarBg);
        CONFIG_COLOR(ScrollbarGrab);
        CONFIG_COLOR(ScrollbarGrabHovered);
        CONFIG_COLOR(ScrollbarGrabActive);
        CONFIG_COLOR(CheckMark);
        CONFIG_COLOR(SliderGrab);
        CONFIG_COLOR(SliderGrabActive);
        CONFIG_COLOR(Button);
        CONFIG_COLOR(ButtonHovered);
        CONFIG_COLOR(ButtonActive);
        CONFIG_COLOR(Header);
        CONFIG_COLOR(HeaderHovered);
        CONFIG_COLOR(HeaderActive);
        CONFIG_COLOR(Separator);
        CONFIG_COLOR(SeparatorHovered);
        CONFIG_COLOR(SeparatorActive);
        CONFIG_COLOR(ResizeGrip);
        CONFIG_COLOR(ResizeGripHovered);
        CONFIG_COLOR(ResizeGripActive);
        CONFIG_COLOR(InputTextCursor);
        CONFIG_COLOR(TabHovered);
        CONFIG_COLOR(Tab);
        CONFIG_COLOR(TabSelected);
        CONFIG_COLOR(TabSelectedOverline);
        CONFIG_COLOR(TabDimmed);
        CONFIG_COLOR(TabDimmedSelected);
        CONFIG_COLOR(TabDimmedSelectedOverline);
        CONFIG_COLOR(DockingPreview);
        CONFIG_COLOR(DockingEmptyBg);
        CONFIG_COLOR(PlotLines);
        CONFIG_COLOR(PlotLinesHovered);
        CONFIG_COLOR(PlotHistogram);
        CONFIG_COLOR(PlotHistogramHovered);
        CONFIG_COLOR(TableHeaderBg);
        CONFIG_COLOR(TableBorderStrong);
        CONFIG_COLOR(TableBorderLight);
        CONFIG_COLOR(TableRowBg);
        CONFIG_COLOR(TableRowBgAlt);
        CONFIG_COLOR(TextLink);
        CONFIG_COLOR(TextSelectedBg);
        CONFIG_COLOR(DragDropTarget);
        CONFIG_COLOR(NavCursor);
        CONFIG_COLOR(NavWindowingHighlight);
        CONFIG_COLOR(NavWindowingDimBg);
        CONFIG_COLOR(ModalWindowDimBg);
    }

    void ImThemeLoader::Loader::ColorConvert(const char *colorString, ImVec4 &color)
    {
        std::string strValue(colorString);
        if (strValue.empty()) { return; }
        const std::regex base_regex("rgba\\((\\d+),\\s?(\\d+),\\s?(\\d+),\\s?([0,1]\\.\\d+)\\)");
        std::smatch      base_match;
        if (std::regex_match(strValue, base_match, base_regex))
        {
            if (base_match.size() == 5)
            {
                uint8_t r = 0, g = 0, b = 0;
                converter<uint8_t>::convert(base_match[1].str().data(), r);
                converter<uint8_t>::convert(base_match[2].str().data(), g);
                converter<uint8_t>::convert(base_match[3].str().data(), b);
                color.x = r / 255.0F;
                color.y = g / 255.0F;
                color.z = b / 255.0F;
                converter<float>::convert(base_match[4].str().data(), color.w);
            }
        }
    }
}