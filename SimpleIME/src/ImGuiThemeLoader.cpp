//
// Created by jamie on 2025/3/7.
//
#include "configs/converter.h"

#include "ImGuiThemeLoader.h"
#include "SimpleIni.h"
#include "common/log.h"
#include "imgui.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {
        auto ImGuiThemeLoader::GetAllThemeNames(const Path &path, std::vector<std::string> &themeNames, bool refresh)
            -> bool
        {
            if ((themeCache.empty() || refresh) && !IndexThemeNames(path))
            {
                log_error("Invalid directory: {}", path.string());
                return false;
            }
            for (const auto &entry : themeCache)
            {
                themeNames.push_back(entry.first);
            }
            return true;
        }

        auto ImGuiThemeLoader::LoadTheme(const std::string &themeName, ImGuiStyle &style) -> bool
        {
            if (!themeCache.contains(themeName))
            {
                return false;
            }
            LoadStyleFromFile(themeCache[themeName], style);
            return true;
        }

        template <>
        constexpr auto toString(const ImVec2 &value) -> std::string
        {
            return std::format("x: {}, y: {}", value.x, value.y);
        }

        template <>
        constexpr auto toString(const ImVec4 &value) -> std::string
        {
            return std::format("x: {}, y: {}, z: {}, w: {}", value.x, value.y, value.z, value.w);
        }

        template <>
        constexpr auto toString(const ImGuiDir &value) -> std::string
        {
            std::string str;
            switch (value)
            {
                case ImGuiDir_None:
                    str.append("ImGuiDir_None");
                    break;
                case ImGuiDir_Left:
                    str.append("ImGuiDir_Left");
                    break;
                case ImGuiDir_Right:
                    str.append("ImGuiDir_Right");
                    break;
                case ImGuiDir_Up:
                    str.append("ImGuiDir_Up");
                    break;
                case ImGuiDir_Down:
                    str.append("ImGuiDir_Down");
                    break;
                case ImGuiDir_COUNT:
                    str.append("ImGuiDir_COUNT");
                    break;
                default:;
            }
            return str;
        }

        template <typename Type>
        void ImGuiThemeLoader::GetSimpleIniValue(CSimpleIniA &ini, const char *section, const char *key, Type &target)
        {
            auto strV = ini.GetValue(section, key);
#ifdef SIMPLE_IME_DEBUG
            assert(strV != nullptr);
            log_debug("Loading {}::{}", section, key);
#endif
            if (strV != nullptr)
            {
                auto result = converter<Type>::convert(strV, target);
                target      = result;
                log_trace("     key {} value {}", key, toString(result));
            }
        }

        bool ImGuiThemeLoader::LoadStyleFromFile(const Path &path, ImGuiStyle &style)
        {
            CSimpleIniA ini;
            ini.LoadFile(path.c_str());

            GetSimpleIniValue(ini, "style", "alpha", style.Alpha);
            GetSimpleIniValue(ini, "style", "disabledAlpha", style.DisabledAlpha);
            GetSimpleIniValue(ini, "style", "windowPadding", style.WindowPadding);
            GetSimpleIniValue(ini, "style", "windowRounding", style.WindowRounding);
            GetSimpleIniValue(ini, "style", "windowBorderSize", style.WindowBorderSize);
            GetSimpleIniValue(ini, "style", "windowMinSize", style.WindowMinSize);
            GetSimpleIniValue(ini, "style", "windowMenuButtonPosition", style.WindowMenuButtonPosition);
            GetSimpleIniValue(ini, "style", "childRounding", style.ChildRounding);
            GetSimpleIniValue(ini, "style", "popupRounding", style.PopupRounding);
            GetSimpleIniValue(ini, "style", "popupBorderSize", style.PopupBorderSize);
            GetSimpleIniValue(ini, "style", "framePadding", style.FramePadding);
            GetSimpleIniValue(ini, "style", "frameRounding", style.FrameRounding);
            GetSimpleIniValue(ini, "style", "frameBorderSize", style.FrameBorderSize);
            GetSimpleIniValue(ini, "style", "itemSpacing", style.ItemSpacing);
            GetSimpleIniValue(ini, "style", "itemInnerSpacing", style.ItemInnerSpacing);
            GetSimpleIniValue(ini, "style", "cellPadding", style.CellPadding);
            GetSimpleIniValue(ini, "style", "indentSpacing", style.IndentSpacing);
            GetSimpleIniValue(ini, "style", "columnsMinSpacing", style.ColumnsMinSpacing);
            GetSimpleIniValue(ini, "style", "scrollbarSize", style.ScrollbarSize);
            GetSimpleIniValue(ini, "style", "scrollbarRounding", style.ScrollbarRounding);
            GetSimpleIniValue(ini, "style", "grabMinSize", style.GrabMinSize);
            GetSimpleIniValue(ini, "style", "grabRounding", style.GrabRounding);
            GetSimpleIniValue(ini, "style", "tabRounding", style.TabRounding);
            GetSimpleIniValue(ini, "style", "tabBorderSize", style.TabBorderSize);
            GetSimpleIniValue(ini, "style", "tabMinWidthForCloseButton", style.TabCloseButtonMinWidthUnselected);
            GetSimpleIniValue(ini, "style", "colorButtonPosition", style.ColorButtonPosition);
            GetSimpleIniValue(ini, "style", "buttonTextAlign", style.ButtonTextAlign);
            GetSimpleIniValue(ini, "style", "selectableTextAlign", style.SelectableTextAlign);

            LoadColors(ini, style);
            return true;
        }

        bool ImGuiThemeLoader::LoadColors(CSimpleIniA &ini, ImGuiStyle &style)
        {
            auto colors = std::span(style.Colors);
            LoadColor(ini, "Text", colors[ImGuiCol_Text]);
            LoadColor(ini, "TextDisabled", colors[ImGuiCol_TextDisabled]);
            LoadColor(ini, "WindowBg", colors[ImGuiCol_WindowBg]);
            LoadColor(ini, "ChildBg", colors[ImGuiCol_ChildBg]);
            LoadColor(ini, "PopupBg", colors[ImGuiCol_PopupBg]);
            LoadColor(ini, "Border", colors[ImGuiCol_Border]);
            LoadColor(ini, "BorderShadow", colors[ImGuiCol_BorderShadow]);
            LoadColor(ini, "FrameBg", colors[ImGuiCol_FrameBg]);
            LoadColor(ini, "FrameBgHovered", colors[ImGuiCol_FrameBgHovered]);
            LoadColor(ini, "FrameBgActive", colors[ImGuiCol_FrameBgActive]);
            LoadColor(ini, "TitleBg", colors[ImGuiCol_TitleBg]);
            LoadColor(ini, "TitleBgActive", colors[ImGuiCol_TitleBgActive]);
            LoadColor(ini, "TitleBgCollapsed", colors[ImGuiCol_TitleBgCollapsed]);
            LoadColor(ini, "MenuBarBg", colors[ImGuiCol_MenuBarBg]);
            LoadColor(ini, "ScrollbarBg", colors[ImGuiCol_ScrollbarBg]);
            LoadColor(ini, "ScrollbarGrab", colors[ImGuiCol_ScrollbarGrab]);
            LoadColor(ini, "ScrollbarGrabHovered", colors[ImGuiCol_ScrollbarGrabHovered]);
            LoadColor(ini, "ScrollbarGrabActive", colors[ImGuiCol_ScrollbarGrabActive]);
            LoadColor(ini, "CheckMark", colors[ImGuiCol_CheckMark]);
            LoadColor(ini, "SliderGrab", colors[ImGuiCol_SliderGrab]);
            LoadColor(ini, "SliderGrabActive", colors[ImGuiCol_SliderGrabActive]);
            LoadColor(ini, "Button", colors[ImGuiCol_Button]);
            LoadColor(ini, "ButtonHovered", colors[ImGuiCol_ButtonHovered]);
            LoadColor(ini, "ButtonActive", colors[ImGuiCol_ButtonActive]);
            LoadColor(ini, "Header", colors[ImGuiCol_Header]);
            LoadColor(ini, "HeaderHovered", colors[ImGuiCol_HeaderHovered]);
            LoadColor(ini, "HeaderActive", colors[ImGuiCol_HeaderActive]);
            LoadColor(ini, "Separator", colors[ImGuiCol_Separator]);
            LoadColor(ini, "SeparatorHovered", colors[ImGuiCol_SeparatorHovered]);
            LoadColor(ini, "SeparatorActive", colors[ImGuiCol_SeparatorActive]);
            LoadColor(ini, "ResizeGrip", colors[ImGuiCol_ResizeGrip]);
            LoadColor(ini, "ResizeGripHovered", colors[ImGuiCol_ResizeGripHovered]);
            LoadColor(ini, "ResizeGripActive", colors[ImGuiCol_ResizeGripActive]);
            LoadColor(ini, "Tab", colors[ImGuiCol_Tab]);
            LoadColor(ini, "TabHovered", colors[ImGuiCol_TabHovered]);
            LoadColor(ini, "TabActive", colors[ImGuiCol_TabActive]);
            LoadColor(ini, "TabUnfocused", colors[ImGuiCol_TabUnfocused]);
            LoadColor(ini, "TabUnfocusedActive", colors[ImGuiCol_TabUnfocusedActive]);
            LoadColor(ini, "PlotLines", colors[ImGuiCol_PlotLines]);
            LoadColor(ini, "PlotLinesHovered", colors[ImGuiCol_PlotLinesHovered]);
            LoadColor(ini, "PlotHistogram", colors[ImGuiCol_PlotHistogram]);
            LoadColor(ini, "PlotHistogramHovered", colors[ImGuiCol_PlotHistogramHovered]);
            LoadColor(ini, "TableHeaderBg", colors[ImGuiCol_TableHeaderBg]);
            LoadColor(ini, "TableBorderStrong", colors[ImGuiCol_TableBorderStrong]);
            LoadColor(ini, "TableBorderLight", colors[ImGuiCol_TableBorderLight]);
            LoadColor(ini, "TableRowBg", colors[ImGuiCol_TableRowBg]);
            LoadColor(ini, "TableRowBgAlt", colors[ImGuiCol_TableRowBgAlt]);
            LoadColor(ini, "TextSelectedBg", colors[ImGuiCol_TextSelectedBg]);
            LoadColor(ini, "DragDropTarget", colors[ImGuiCol_DragDropTarget]);
            LoadColor(ini, "NavHighlight", colors[ImGuiCol_NavHighlight]);
            LoadColor(ini, "NavWindowingHighlight", colors[ImGuiCol_NavWindowingHighlight]);
            LoadColor(ini, "NavWindowingDimBg", colors[ImGuiCol_NavWindowingDimBg]);
            LoadColor(ini, "ModalWindowDimBg", colors[ImGuiCol_ModalWindowDimBg]);
            return true;
        }

        void ImGuiThemeLoader::LoadColor(CSimpleIniA &ini, const char *key, ImVec4 &color)
        {
            if (const auto *value = ini.GetValue("colors", key); value != nullptr)
            {
                const std::regex base_regex("\"rgba\\((\\d+),\\s?(\\d+),\\s?(\\d+),\\s?([0,1]\\.\\d+)\\)\"");
                std::smatch      base_match;
                std::string      strValue(value);
                if (std::regex_match(strValue, base_match, base_regex))
                {
                    if (base_match.size() == 5)
                    {
                        uint32_t rUint = converter<uint32_t>::convert(base_match[1].str().data());
                        uint32_t gUint = converter<uint32_t>::convert(base_match[2].str().data());
                        uint32_t bUint = converter<uint32_t>::convert(base_match[3].str().data());

                        color.x = rUint / 255.0F;
                        color.y = gUint / 255.0F;
                        color.z = bUint / 255.0F;
                        color.w = converter<float>::convert(base_match[4].str().data(), color.w);
                        log_trace("Load color: {}: {}", key, toString(color));
                    }
                }
            }
        }

        auto ImGuiThemeLoader::IndexThemeNames(const Path &themeDir) -> bool
        {
            namespace fs = std::filesystem;
            const std::regex pattern(R"((\w+)_theme\.ini)", std::regex_constants::icase);
            std::smatch      themeNameMatch;

            if (!exists(themeDir) || !fs::is_directory(themeDir))
            {
                return false;
            }
            for (const auto &entry : fs::directory_iterator(themeDir))
            {
                if (entry.is_regular_file())
                {
                    const std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, themeNameMatch, pattern))
                    {
                        if (themeNameMatch.size() > 1)
                        {
                            themeCache[themeNameMatch[1].str()] = entry.path().string();
                        }
                    }
                }
            }
            return true;
        }
    }
}