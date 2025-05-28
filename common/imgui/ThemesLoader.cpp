//
// Created by jamie on 2025/5/26.
//

#include "ThemesLoader.h"

#include "common/log.h"
#include "common/toml++/toml.hpp"
#include "imgui.h"

#include <expected>
#include <ranges>

namespace LIBC_NAMESPACE_DECL
{
void ImGuiUtil::ThemesLoader::LoadThemes()
{
    if (m_availableThemes.size() > 0)
    {
        return;
    }
    m_availableThemes.emplace_back(DefaultThemeIndex_DARK, "Dark");
    m_availableThemes.emplace_back(DefaultThemeIndex_LIGHT, "Light");
    m_availableThemes.emplace_back(DefaultThemeIndex_CLASSIC, "Classic");
    try
    {
        toml::table themesValue = toml::parse_file(m_filePath.get());
        if (!themesValue.at("themes").is_array_of_tables())
        {
            throw std::runtime_error("Invalid themes format");
        }
        const auto themesArray = themesValue["themes"].as_array();
        for (size_t index = 0; index < themesArray->size(); ++index)
        {
            if (auto name = themesValue["themes"][index]["name"]; name.is_string())
            {
                auto nameStr = name.value_or(""s);
                if (nameStr == "Classic" || nameStr == "Dark" || nameStr == "Light")
                {
                    continue;
                }
                m_availableThemes.emplace_back(index, nameStr);
            }
        }
        std::sort(m_availableThemes.begin() + DefaultThemeIndex_Count, m_availableThemes.end());
    }
    catch (std::exception &e)
    {
        log_error("Can't load themes from {}: {}", m_filePath.get(), e.what());
    }
}

template <typename Type>
struct converter
{
};

template <>
struct converter<ImGuiDir>
{
    static void convert(const char *value, ImGuiDir &target)
    {
        if (strcmp("Left", value) == 0)
        {
            target = ImGuiDir_Left;
        }
        else if (strcmp("Right", value) == 0)
        {
            target = ImGuiDir_Right;
        }
        else if (strcmp("Up", value) == 0)
        {
            target = ImGuiDir_Up;
        }
        else if (strcmp("Down", value) == 0)
        {
            target = ImGuiDir_Down;
        }
    }
};

template <>
struct converter<float>
{
    static void convert(const char *value, float &target)
    {
        if (const std::string strValue(value); !strValue.empty())
        {
            char       *pEnd{};
            const float result = std::strtof(value, &pEnd);
            if (*pEnd == 0)
            {
                target = result;
            }
        }
    }
};

template <>
struct converter<uint8_t>
{
    static void convert(const char *value, uint8_t &target)
    {
        if (const std::string strValue(value); !strValue.empty())
        {
            char *pEnd{};
            if (const uint32_t result = std::strtoul(value, &pEnd, 10); *pEnd != 0 || result > 255)
            {
                SPDLOG_WARN("Can't get unsigned int value or value to large()");
            }
            else
            {
                target = static_cast<uint8_t>(result);
            }
        }
    }
};

template <>
struct converter<ImVec2>
{
    static void convert(const char *value, ImVec2 &target)
    {
        if (std::string strValue(value); strValue.starts_with("[") && strValue.ends_with("]"))
        {
            strValue.erase(0, 1);
            strValue.erase(strValue.end() - 1);
            std::ranges::split_view view = std::views::split(strValue, std::string_view(", "));

            std::vector<std::string> elements;
            for (const auto &part : view)
            {
                elements.emplace_back(part.begin(), part.end());
            }
            if (elements.size() == 2)
            {
                converter<float>::convert(elements[0].c_str(), target.x);
                converter<float>::convert(elements[1].c_str(), target.y);
            }
        }
    }
};

#define CONFIG_STYLE(type, styleName) converter<type>::convert(styleNode[#styleName].value_or(""), style.styleName)

void ConfigImGuiStyle(const toml::node_view<toml::node> &styleNode, ImGuiStyle &style)
{
    if (!styleNode.is_table())
    {
        return;
    }
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

#define CONFIG_COLOR(colorName) ColorConvert(colorsNode[#colorName].value_or(""), style.Colors[ImGuiCol_##colorName])

void ColorConvert(const char *colorString, ImVec4 &color)
{
    const std::string strValue(colorString);
    if (strValue.empty())
    {
        return;
    }
    const std::regex base_regex("rgba\\((\\d+),\\s?(\\d+),\\s?(\\d+),\\s?([0,1]\\.\\d+)\\)");
    if (std::smatch base_match; std::regex_match(strValue, base_match, base_regex))
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

void ConfigImGuiColor(const toml::node_view<toml::node> &colorsNode, ImGuiStyle &style)
{
    if (!colorsNode.is_table())
    {
        return;
    }
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

void UseTheme_(toml::table &table, ImGuiStyle & style)
{
    const auto styleTable  = table["style"];
    const auto styleColors = table["style"]["colors"];
    ConfigImGuiStyle(styleTable, style);
    ConfigImGuiColor(styleColors, style);
}

auto ImGuiUtil::ThemesLoader::UseTheme(const size_t themeIndex, ImGuiStyle & style) const noexcept -> std::expected<void, std::string>
{
    if (themeIndex >= m_availableThemes.size())
    {
        return std::unexpected(std::format("Index {} out of range {}", themeIndex, m_availableThemes.size()));
    }
    try
    {
        auto themesValue = toml::parse_file(m_filePath.get());
        if (!themesValue.at("themes").is_array_of_tables())
        {
            return std::unexpected("Invalid themes format");
        }

        const auto &theme = m_availableThemes.at(themeIndex);
        switch (theme.tomlArrayIndex)
        {
            case DefaultThemeIndex_DARK:
                ImGui::StyleColorsDark(&style);
                return {};
            case DefaultThemeIndex_LIGHT:
                ImGui::StyleColorsLight(&style);
                return {};
            case DefaultThemeIndex_CLASSIC:
                ImGui::StyleColorsClassic(&style);
                return {};
            default:
                break;
        }

        if (const auto themeTable = themesValue["themes"][theme.tomlArrayIndex].as_table())
        {
            UseTheme_(*themeTable, style);
            return {};
        }
        return std::unexpected(std::format("Can't find theme with index: {}", themeIndex));
    }
    catch (std::exception &e)
    {
        return std::unexpected(std::format("Theme loading error: {}", e.what()));
    }
}
}
