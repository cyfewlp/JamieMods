//
// Created by jamie on 2025/5/26.
//

#include "ThemesLoader.h"

#include "common/config.h"
#include "common/log.h"
#include "common/toml++/toml.hpp"
#include "imgui.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <expected>
#include <format>
#include <regex>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>

namespace LIBC_NAMESPACE_DECL
{
void ImGuiUtil::ThemesLoader::LoadThemes()
{
    if (!m_availableThemes.empty())
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
        auto *const themesArray = themesValue["themes"].as_array();
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
    static auto convert(std::string_view value) -> std::optional<ImGuiDir>
    {
        if ("Left" == value)
        {
            return ImGuiDir_Left;
        }
        if ("Right" == value)
        {
            return ImGuiDir_Right;
        }
        if ("Up" == value)
        {
            return ImGuiDir_Up;
        }
        if ("Down" == value)
        {
            return ImGuiDir_Down;
        }
        return std::nullopt;
    }
};

template <>
struct converter<float>
{
    static auto convert(std::string_view svValue) -> std::optional<float>
    {
        if (!svValue.empty())
        {
            float result;
            auto [ptr, ec] = std::from_chars(svValue.data(), svValue.data() + svValue.size(), result);
            if (ec == std::errc())
            {
                return result;
            }
        }
        return std::nullopt;
    }
};

template <>
struct converter<uint8_t>
{
    static auto convert(std::string_view value) -> std::optional<uint8_t>
    {
        if (!value.empty())
        {
            unsigned long val = 0;
            auto [ptr, ec]    = std::from_chars(value.data(), value.data() + value.size(), val);
            if (ec == std::errc() && val <= 255)
            {
                return static_cast<uint8_t>(val);
            }
        }
        return std::nullopt;
    }
};

template <>
struct converter<ImVec2>
{
    static auto convert(std::string_view svValue) -> std::optional<ImVec2>
    {
        if (!svValue.starts_with("[") || !svValue.ends_with("]"))
        {
            return std::nullopt;
        }

        svValue.remove_prefix(1);
        svValue.remove_suffix(1);
        auto comma = svValue.find(", ");
        if (comma == std::string_view::npos)
        {
            return std::nullopt;
        }
        std::string_view svX = svValue.substr(0, comma);
        std::string_view svY = svValue.substr(comma + 1);

        if (auto optX = converter<float>::convert(svX))
        {
            ImVec2 result;
            result.x = optX.value();
            if (auto optY = converter<float>::convert(svY))
            {
                result.y = optY.value();
            }
            return result;
        }
        return std::nullopt;
    }
};

void ConfigImGuiStyle(const toml::node_view<toml::node> &styleNode, ImGuiStyle &style)
{
    if (!styleNode.is_table())
    {
        return;
    }

    auto SetStyle = [&styleNode]<typename T>(std::string_view key, T &val) {
        auto opt = styleNode[key].value<T>();
        if (opt.has_value())
        {
            val = opt.value();
        }
    };

    auto SetStyle2 = [&styleNode]<typename T>(std::string_view key, T &val) {
        auto opt = converter<T>::convert(styleNode[key].value_or(""));
        if (opt.has_value())
        {
            val = opt.value();
        }
    };
    SetStyle("Alpha", style.Alpha);
    SetStyle("DisabledAlpha", style.DisabledAlpha);
    SetStyle2("WindowPadding", style.WindowPadding);
    SetStyle("WindowRounding", style.WindowRounding);
    SetStyle("WindowBorderSize", style.WindowBorderSize);
    SetStyle("WindowBorderHoverPadding", style.WindowBorderHoverPadding);
    SetStyle2("WindowMinSize", style.WindowMinSize);
    SetStyle2("WindowTitleAlign", style.WindowTitleAlign);
    SetStyle("ChildRounding", style.ChildRounding);
    SetStyle("ChildBorderSize", style.ChildBorderSize);
    SetStyle("PopupRounding", style.PopupRounding);
    SetStyle("PopupBorderSize", style.PopupBorderSize);
    SetStyle2("FramePadding", style.FramePadding);
    SetStyle("FrameRounding", style.FrameRounding);
    SetStyle("FrameBorderSize", style.FrameBorderSize);
    SetStyle2("ItemSpacing", style.ItemSpacing);
    SetStyle2("ItemInnerSpacing", style.ItemInnerSpacing);
    SetStyle2("CellPadding", style.CellPadding);
    SetStyle2("TouchExtraPadding", style.TouchExtraPadding);
    SetStyle("IndentSpacing", style.IndentSpacing);
    SetStyle("ColumnsMinSpacing", style.ColumnsMinSpacing);
    SetStyle("ScrollbarSize", style.ScrollbarSize);
    SetStyle("ScrollbarRounding", style.ScrollbarRounding);
    SetStyle("GrabMinSize", style.GrabMinSize);
    SetStyle("GrabRounding", style.GrabRounding);
    SetStyle("LogSliderDeadzone", style.LogSliderDeadzone);
    SetStyle("ImageBorderSize", style.ImageBorderSize);
    SetStyle("TabRounding", style.TabRounding);
    SetStyle("TabBorderSize", style.TabBorderSize);
    SetStyle("TabCloseButtonMinWidthSelected", style.TabCloseButtonMinWidthSelected);
    SetStyle("TabCloseButtonMinWidthUnselected", style.TabCloseButtonMinWidthUnselected);
    SetStyle("TabBarBorderSize", style.TabBarBorderSize);
    SetStyle("TabBarOverlineSize", style.TabBarOverlineSize);
    SetStyle("TableAngledHeadersAngle", style.TableAngledHeadersAngle);
    SetStyle2("WindowMenuButtonPosition", style.WindowMenuButtonPosition);
    SetStyle2("TableAngledHeadersTextAlign", style.TableAngledHeadersTextAlign);
    SetStyle2("ColorButtonPosition", style.ColorButtonPosition);
    SetStyle2("ButtonTextAlign", style.ButtonTextAlign);
    SetStyle2("SelectableTextAlign", style.SelectableTextAlign);
    SetStyle("SeparatorTextBorderSize", style.SeparatorTextBorderSize);
    SetStyle2("SeparatorTextAlign", style.SeparatorTextAlign);
    SetStyle2("SeparatorTextPadding", style.SeparatorTextPadding);
    SetStyle2("DisplayWindowPadding", style.DisplayWindowPadding);
    SetStyle2("DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
    // CONFIG_STYLE(float, DockingSeparatorSize);
    SetStyle("MouseCursorScale", style.MouseCursorScale);
    SetStyle("CurveTessellationTol", style.CurveTessellationTol);
    SetStyle("CircleTessellationMaxError", style.CircleTessellationMaxError);
}

static void ConvertTo(const std::string &strColor, ImVec4 &color)
{
    if (strColor.empty())
    {
        return;
    }
    static const std::regex REGEX("rgba\\((\\d+),\\s?(\\d+),\\s?(\\d+),\\s?([0,1]\\.\\d+)\\)");
    if (std::smatch base_match; std::regex_match(strColor, base_match, REGEX))
    {
        if (base_match.size() == 5)
        {
            auto optR = converter<uint8_t>::convert(base_match[1].str().data());
            auto optG = converter<uint8_t>::convert(base_match[2].str().data());
            auto optB = converter<uint8_t>::convert(base_match[3].str().data());
            auto optW = converter<float>::convert(base_match[4].str().data());

            if (optR && optG && optB && optW)
            {
                color.x = optR.value() / 255.0F;
                color.y = optG.value() / 255.0F;
                color.z = optB.value() / 255.0F;
                color.w = optW.value();
            }
        }
    }
}

void ConfigImGuiColor(const toml::node_view<toml::node> &colorsNode, ImGuiStyle &style)
{
    if (!colorsNode.is_table())
    {
        return;
    }
    auto SetColor = [&colorsNode](std::string_view key, ImVec4 &color) {
        if (auto opt = colorsNode[key].value<std::string>())
        {
            ConvertTo(opt.value(), color);
        }
    };
    SetColor("Text", style.Colors[ImGuiCol_Text]);
    SetColor("TextDisabled", style.Colors[ImGuiCol_TextDisabled]);
    SetColor("WindowBg", style.Colors[ImGuiCol_WindowBg]);
    SetColor("ChildBg", style.Colors[ImGuiCol_ChildBg]);
    SetColor("PopupBg", style.Colors[ImGuiCol_PopupBg]);
    SetColor("Border", style.Colors[ImGuiCol_Border]);
    SetColor("BorderShadow", style.Colors[ImGuiCol_BorderShadow]);
    SetColor("FrameBg", style.Colors[ImGuiCol_FrameBg]);
    SetColor("FrameBgHovered", style.Colors[ImGuiCol_FrameBgHovered]);
    SetColor("FrameBgActive", style.Colors[ImGuiCol_FrameBgActive]);
    SetColor("TitleBg", style.Colors[ImGuiCol_TitleBg]);
    SetColor("TitleBgActive", style.Colors[ImGuiCol_TitleBgActive]);
    SetColor("TitleBgCollapsed", style.Colors[ImGuiCol_TitleBgCollapsed]);
    SetColor("MenuBarBg", style.Colors[ImGuiCol_MenuBarBg]);
    SetColor("ScrollbarBg", style.Colors[ImGuiCol_ScrollbarBg]);
    SetColor("ScrollbarGrab", style.Colors[ImGuiCol_ScrollbarGrab]);
    SetColor("ScrollbarGrabHovered", style.Colors[ImGuiCol_ScrollbarGrabHovered]);
    SetColor("ScrollbarGrabActive", style.Colors[ImGuiCol_ScrollbarGrabActive]);
    SetColor("CheckMark", style.Colors[ImGuiCol_CheckMark]);
    SetColor("SliderGrab", style.Colors[ImGuiCol_SliderGrab]);
    SetColor("SliderGrabActive", style.Colors[ImGuiCol_SliderGrabActive]);
    SetColor("Button", style.Colors[ImGuiCol_Button]);
    SetColor("ButtonHovered", style.Colors[ImGuiCol_ButtonHovered]);
    SetColor("ButtonActive", style.Colors[ImGuiCol_ButtonActive]);
    SetColor("Header", style.Colors[ImGuiCol_Header]);
    SetColor("HeaderHovered", style.Colors[ImGuiCol_HeaderHovered]);
    SetColor("HeaderActive", style.Colors[ImGuiCol_HeaderActive]);
    SetColor("Separator", style.Colors[ImGuiCol_Separator]);
    SetColor("SeparatorHovered", style.Colors[ImGuiCol_SeparatorHovered]);
    SetColor("SeparatorActive", style.Colors[ImGuiCol_SeparatorActive]);
    SetColor("ResizeGrip", style.Colors[ImGuiCol_ResizeGrip]);
    SetColor("ResizeGripHovered", style.Colors[ImGuiCol_ResizeGripHovered]);
    SetColor("ResizeGripActive", style.Colors[ImGuiCol_ResizeGripActive]);
    SetColor("InputTextCursor", style.Colors[ImGuiCol_InputTextCursor]);
    SetColor("TabHovered", style.Colors[ImGuiCol_TabHovered]);
    SetColor("Tab", style.Colors[ImGuiCol_Tab]);
    SetColor("TabSelected", style.Colors[ImGuiCol_TabSelected]);
    SetColor("TabSelectedOverline", style.Colors[ImGuiCol_TabSelectedOverline]);
    SetColor("TabDimmed", style.Colors[ImGuiCol_TabDimmed]);
    SetColor("TabDimmedSelected", style.Colors[ImGuiCol_TabDimmedSelected]);
    SetColor("TabDimmedSelectedOverline", style.Colors[ImGuiCol_TabDimmedSelectedOverline]);
    // CONFIG_COLOR(DockingPreview);
    // CONFIG_COLOR(DockingEmptyBg);
    SetColor("PlotLines", style.Colors[ImGuiCol_PlotLines]);
    SetColor("PlotLinesHovered", style.Colors[ImGuiCol_PlotLinesHovered]);
    SetColor("PlotHistogram", style.Colors[ImGuiCol_PlotHistogram]);
    SetColor("PlotHistogramHovered", style.Colors[ImGuiCol_PlotHistogramHovered]);
    SetColor("TableHeaderBg", style.Colors[ImGuiCol_TableHeaderBg]);
    SetColor("TableBorderStrong", style.Colors[ImGuiCol_TableBorderStrong]);
    SetColor("TableBorderLight", style.Colors[ImGuiCol_TableBorderLight]);
    SetColor("TableRowBg", style.Colors[ImGuiCol_TableRowBg]);
    SetColor("TableRowBgAlt", style.Colors[ImGuiCol_TableRowBgAlt]);
    SetColor("TextLink", style.Colors[ImGuiCol_TextLink]);
    SetColor("TextSelectedBg", style.Colors[ImGuiCol_TextSelectedBg]);
    SetColor("DragDropTarget", style.Colors[ImGuiCol_DragDropTarget]);
    SetColor("NavCursor", style.Colors[ImGuiCol_NavCursor]);
    SetColor("NavWindowingHighlight", style.Colors[ImGuiCol_NavWindowingHighlight]);
    SetColor("NavWindowingDimBg", style.Colors[ImGuiCol_NavWindowingDimBg]);
    SetColor("ModalWindowDimBg", style.Colors[ImGuiCol_ModalWindowDimBg]);
}

void UseTheme_(toml::table &table, ImGuiStyle &style)
{
    const auto styleTable  = table["style"];
    const auto styleColors = table["style"]["colors"];
    ConfigImGuiStyle(styleTable, style);
    ConfigImGuiColor(styleColors, style);
}

static toml::table g_themesTable;

auto ImGuiUtil::ThemesLoader::UseTheme(const size_t themeIndex, ImGuiStyle &style) const noexcept
    -> std::expected<void, std::string>
{
    if (themeIndex >= m_availableThemes.size())
    {
        return std::unexpected(std::format("Index {} out of range {}", themeIndex, m_availableThemes.size()));
    }
    try
    {
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
        if (g_themesTable.empty() || !g_themesTable.is_array_of_tables())
        {
            g_themesTable = toml::parse_file(m_filePath.get());
        }

        if (!g_themesTable.at("themes").is_array_of_tables())
        {
            return std::unexpected("Invalid themes format");
        }

        if (const auto themeTable = g_themesTable["themes"][theme.tomlArrayIndex]; themeTable.is_table())
        {
            ConfigImGuiStyle(themeTable["style"], style);
            ConfigImGuiColor(themeTable["style"]["colors"], style);
            return {};
        }
        return std::unexpected(std::format("Can't find theme with index: {}", themeIndex));
    }
    catch (std::exception &e)
    {
        return std::unexpected(std::format("Theme loading error: {}", e.what()));
    }
}

void ImGuiUtil::ThemesLoader::Cleanup()
{
    g_themesTable.clear();
}
}
