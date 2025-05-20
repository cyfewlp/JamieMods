//
// Created by jamie on 2025/3/7.
//

#ifndef IMGUITHEMELOADER_H
#define IMGUITHEMELOADER_H

#pragma once

#include "SimpleIni.h"
#include "common/config.h"
#include "common/log.h"
#include "configs/converter.h"
#include "imgui.h"

#include <filesystem>
#include <regex>
#include <string>
#include <unordered_map>

namespace LIBC_NAMESPACE_DECL
{
namespace Ime
{
template <>
struct converter<ImVec2>
{
    static constexpr auto convert(const char *value, ImVec2 aDefault) -> ImVec2
    {
        if (value == nullptr)
        {
            return aDefault;
        }
        static const std::regex base_regex("\\[(\\d+\\.\\d+),\\s+(\\d+\\.\\d+)\\]");

        ImVec2      result = aDefault;
        std::smatch base_match;
        std::string strValue(value);

        if (std::regex_match(strValue, base_match, base_regex))
        {
            if (base_match.size() == 3)
            {
                float xVec = converter<float>::convert(base_match[1].str().data());
                float yVec = converter<float>::convert(base_match[2].str().data());
                result.x   = xVec;
                result.y   = yVec;
            }
        }
        return result;
    }
};

template <>
struct converter<ImGuiDir>
{
    static constexpr auto convert(const char *value, ImGuiDir aDefault) -> ImGuiDir
    {
        if (value == nullptr)
        {
            return aDefault;
        }
        std::string strValue(value);
        if (strValue.starts_with("\""))
        {
            strValue.erase(0, 1);
        }
        if (strValue.ends_with("\""))
        {
            strValue.erase(strValue.end() - 1);
        }
        ImGuiDir result = aDefault;
        if (strValue.compare("Left") == 0)
        {
            result = ImGuiDir_Left;
        }
        else if (strValue.compare("Right") == 0)
        {
            result = ImGuiDir_Right;
        }
        else if (strValue.compare("Up") == 0)
        {
            result = ImGuiDir_Up;
        }
        else if (strValue.compare("Down") == 0)
        {
            result = ImGuiDir_Down;
        }
        return result;
    }
};

class ImGuiThemeLoader
{
    std::unordered_map<std::string, std::string> themeCache;
    using Path = std::filesystem::path;

public:
    auto GetAllThemeNames(const Path &path, std::vector<std::string> &themeNames, bool refresh = false) -> bool;

    auto LoadTheme(const std::string &themeName, ImGuiStyle &style) -> bool;

private:
    auto IndexThemeNames(const Path &themeDir) -> bool;

    template <typename Type>
    static void GetSimpleIniValue(CSimpleIniA &ini, const char *section, const char *key, Type &target);
    static bool LoadStyleFromFile(const Path &path, ImGuiStyle &style);
    static bool LoadColors(CSimpleIniA &ini, ImGuiStyle &style);
    static void LoadColor(CSimpleIniA &ini, const char *key, ImVec4 &color);
    static void SetupMissingColors(ImGuiStyle &style);
};
}
}

#endif // IMGUITHEMELOADER_H
